#include "catalog.h"

/**
 * Create a Relation Catalog, including setting up the relations that describe itself. 
 * The program  ensures that a relation with the same name doesn't already exist
 * and adds a tuple to the relcat relation. 
 * Third, for each attrCnt attributes, it invokes the AttrCatalog::addInfo() method of the attribute catalog table 
 * passing the appropriate attribute information from the attrList[]. 
 * Finally, it creates a HeapFile instance to hold tuples of the relation.
 *
 * @param relation
 * @param attrCnt
 * @param attrList[]
 * @return status
 **/
const Status RelCatalog::createRel(const string & relation,
        const int attrCnt,
        const attrInfo attrList[])
{
    Status status;
    //initialize relation description and attribute description
    RelDesc rd;
    AttrDesc ad;
    //temp used in duplicate check
    const char* temp;
    int totalSize = 0;
    //keep total width for max check
    for(int m = 0; m < attrCnt; m++){
	totalSize += attrList[m].attrLen;
	if(totalSize > 1004) return INVALIDRECLEN;
    }
    //verify that no duplicate attribute names are used
    for(int j = 0; j < attrCnt; j++){
        temp = attrList[j].attrName;
        for(int k = 0; k < attrCnt; k++){
            if((k != j) && ( (strcmp(temp, attrList[k].attrName)) == 0) ) {
                //this attrName is already used
                return DUPLATTR;
            }//end if
        }//end for k
    }//end for j
    //check for valid parameters 
    if (relation.empty() || attrCnt < 1)
        return BADCATPARM;
    //make sure that input doesn not exceed size requirement
    if (relation.length() >= sizeof rd.relName)
        return NAMETOOLONG;
    //This getInfo should return RELEXISTS, since we are creating one.
    status = relCat->getInfo(relation, rd);
    if (status != OK && status != RELNOTFOUND) return status;
    //if exists than return status
    if (status == OK) return RELEXISTS ;
    //Check if it exists
    strcpy(rd.relName, relation.c_str());
    rd.attrCnt = attrCnt; // number of attributes
    //Create the relation in relcat
    status = relCat->addInfo(rd); 
    if (status != OK) return status;
    //the intitial offset will start at 0
    int offset = 0;
    //loop through to copy data for each attribute in the array
    for(int i = 0; i < attrCnt; i++){
        //Copy data
        strcpy(ad.relName, attrList[i].relName);
        strcpy(ad.attrName, attrList[i].attrName);
        ad.attrOffset = offset;
        ad.attrType = attrList[i].attrType;
        ad.attrLen = attrList[i].attrLen;
        ad.attrOffset = offset;
        offset += attrList[i].attrLen;
        //Add it to the attribute catalog
        status = attrCat->addInfo(ad);
        if (status != OK) return status;
    }

    //Create heapfile for the new relation
    status = createHeapFile(relation);
    if (status != OK) return status;

    //if this line reached all functions were executed fine
    return OK;

}//end createRel()
