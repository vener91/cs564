#include "catalog.h"


RelCatalog::RelCatalog(Status &status) :
    HeapFile(RELCATNAME, status)
{
    // nothing should be needed here
}
/*
 * This function scans the relcat relation
 * to look for the tuple whose first attribute matches the string relName. 
 * It calls scanNext() and getRecord() to get the desired tuple. 
 * Finally, it uses memcpy() to copy the tuple out of the buffer pool into the return parameter record.
 * @param relation
 * @param &record
 * @return status
 *
 */
const Status RelCatalog::getInfo(const string & relation, RelDesc &record)
{
    //make sure the a proper parameter is given
    if (relation.empty()) return BADCATPARM;

    Status status;
    //variables to store the sought record and its id
    Record rec;
    RID rid;
    //create a HeapFileScan object to scan the relation catalog
    HeapFileScan* hfs = new HeapFileScan(RELCATNAME, status);
    if (status != OK) return status;
    //unconditional scan to look for record within name relation
    status = hfs->startScan(0, 0, STRING, NULL, EQ);
    if (status != OK) return status;

    //Search for it
    while ((status = hfs->scanNext(rid)) != FILEEOF){
        if (status != OK) return status;
	//get the record if it exists
        status = hfs->getRecord(rec);
        if (status != OK) return status;
	//copy out the data
        memcpy(&record, rec.data, sizeof(RelDesc));
	//validate that the names match
        if (strcmp(record.relName, relation.c_str()) == 0) {
            delete hfs;
            return OK;
        }
    }

    delete hfs;
    //if this code reached then relation was not found in relation catalog
    return RELNOTFOUND;

}//end getInfo()

/**
 * This function adds the relation descriptor contained in record to the relcat relation.
 * RelDesc represents both the in-memory format and on-disk format of a tuple in relcat. 
 * First, it creates an InsertFileScan object on the relation catalog table. 
 * Next, it creates a record and inserts it into the relation catalog table using the method insertRecord of InsertFileScan.
 *
 * @param record
 * @return status
 **/
const Status RelCatalog::addInfo(RelDesc & record)
{
    
    //variables to store the sought record and its id
    RID rid;
    Record rec;
    //create an InsertFileScan object to insert tuple into relation catalog
    InsertFileScan*  ifs;
    Status status;

    ifs = new InsertFileScan(RELCATNAME, status);
    if (status != OK) return status;

    //Add it to record
    rec.data = &record;
    rec.length = sizeof(RelDesc);
    status = ifs->insertRecord(rec, rid);
    if (status != OK) return status;
    return OK;
}//end addInfo()

/**
 * Remove the tuple corresponding to relName from relcat.
 * It starts a filter scan on relcat to locate the rid of the desired tuple. 
 * It then will call deleteRecord() to remove it. 
 * @param relation
 * @return status
 *
 **/
const Status RelCatalog::removeInfo(const string & relation)
{
    
    //make sure the a proper parameter is given
    if (relation.empty()) return BADCATPARM;

    Status status;
    //variables to store the sought record and its id
    RID rid;
    Record rec;
    RelDesc record;
    HeapFileScan*  hfs;
    // create a HeapFileScan object to find relation to be removed
    hfs = new HeapFileScan(RELCATNAME, status);
    if (status != OK) return status;
    //unconditonal scan to find relation
    status = hfs->startScan(0, 0, STRING, NULL, EQ);
    if (status != OK) return status;

    //Search for it
    while ((status = hfs->scanNext(rid)) != FILEEOF){
        if (status != OK) return status;
	//get record if it exists
        status = hfs->getRecord(rec);
        if (status != OK) return status;
	//copy out data
        memcpy(&record, rec.data, sizeof(RelDesc));
	//validate theat we found the right record
        if (strcmp(record.relName, relation.c_str()) == 0) {
	    //now delete the record from the heap file
            hfs->deleteRecord();
            delete hfs;
	    //status is OK
            return OK;
        }
    }

    delete hfs;
    //if this code reached then relation was not found in relation catalog
    return RELNOTFOUND;


}//end removeInfo()

/**
 * destructor for RelCatalog()
 **/
RelCatalog::~RelCatalog()
{
    // nothing should be needed here
}

/**
 * constructor for AttrCatalog
 **/
AttrCatalog::AttrCatalog(Status &status) :
    HeapFile(ATTRCATNAME, status)
{
    // nothing should be needed here
}

/**
 * Returns the attribute descriptor record for attribute attrName in relation relName.
 * Scans over the underlying heapfile to get all tuples for relation and checks each tuple to find whether it corresponds to attrName.
 * This procedure is used  because a predicated HeapFileScan does not allow conjuncted predicates.
 * <i>Note: the tuples in attrcat are of type AttrDesc.</i>
 *
 * @param relation
 * @param &attrName
 * @param &record
 * @return status
 **/
const Status AttrCatalog::getInfo(const string & relation,
        const string & attrName,
        AttrDesc &record)
{

    Status status;
    //variables to store the sought record and its id
    RID rid;
    Record rec;
    HeapFileScan*  hfs;
    //verify that given parameters are valid
    if (relation.empty() || attrName.empty()) return BADCATPARM;
    //create a HeapFileScan object to find tuple
    hfs = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) return status;
    //unconditional scan of string
    status = hfs->startScan(0, 0, STRING, NULL, EQ);
    if (status != OK) return status;

    //Search for it
    while ((status = hfs->scanNext(rid)) != FILEEOF){
        if (status != OK) return status;
	//get the record if it exists
        status = hfs->getRecord(rec);
        if (status != OK) return status;
        memcpy(&record, rec.data, sizeof(AttrDesc));
        if (strcmp(record.relName, relation.c_str()) == 0 && strcmp(record.attrName, attrName.c_str()) == 0) {
            delete hfs;
            return OK;
        }
    }

    delete hfs;
    //If code reached, then attribute wasn't in attribute catalog
    return ATTRNOTFOUND;

}//end getInfo()

/**
 * Adds a tuple (corresponding to an attribute of a relation) to the attrcat relation. 
 * @param record
 * @return status
 **/
const Status AttrCatalog::addInfo(AttrDesc & record)
{
    
    //variables to store the sought record and its id
    RID rid;
    Record rec;
    InsertFileScan*  ifs;
    Status status;

    ifs = new InsertFileScan(ATTRCATNAME, status);
    if (status != OK) return status;

    //Add it to record
    rec.data = &record;
    rec.length = sizeof(AttrDesc);
    status = ifs->insertRecord(rec, rid);
    if (status != OK) return status;
    //if reached then status is OK
    return OK;

}//end addInfo()

/**
 * Removes the tuple from attrcat that corresponds to attribute attrName of relation.
 *
 * @param relation
 * @param &attrName
 * @return status
 **/
const Status AttrCatalog::removeInfo(const string & relation,
        const string & attrName)
{
    Status status;
    //variables to store the sought record and its id
    Record rec;
    RID rid;
    AttrDesc record;
    HeapFileScan*  hfs;
    //verify that given parameters are valid
    if (relation.empty() || attrName.empty()) return BADCATPARM;
    //create HeapFileScan object to find approriate tuples
    hfs = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) return status;
    //scan through every record
    status = hfs->startScan(0, 0, STRING, NULL, EQ);
    if (status != OK) return status;

    //Search for it
    while ((status = hfs->scanNext(rid)) != FILEEOF){
        if (status != OK) return status;
	//get record if it exists
        status = hfs->getRecord(rec);
        if (status != OK) return status;
        memcpy(&record, rec.data, sizeof(RelDesc));
	//validate that the correct record was found
        if (strcmp(record.relName, relation.c_str()) == 0 && strcmp(record.attrName, attrName.c_str()) == 0) {
            //delete record from the file
	    hfs->deleteRecord();
            delete hfs;
            return OK;
        }
    }//end while loop

    delete hfs;
    //Means can't find
    return ATTRNOTFOUND;
}//end removeInfo()

/**
 * While getInfo() above returns the description of a single attribute,
 * this method returns (by reference) descriptors for all attributes of the relation via attr, 
 * an array of AttrDesc structures,  and the count of the number of attributes in attrCnt. 
 * The attrs array is allocated by this function, but it should be deallocated by the caller. 
 *
 * @param &relation
 * @param &attrCnt
 * @param *&attrs
 **/
const Status AttrCatalog::getRelInfo(const string & relation,
        int &attrCnt,
        AttrDesc *&attrs)
{
    Status status;
    RID rid;
    Record rec;
    HeapFileScan*  hfs;
    RelDesc rd;
    int attrsLeft;
    //check validity of parameter
    if (relation.empty()) return BADCATPARM;
    //get Relation description data
    status = relCat->getInfo(relation, rd);
    if (status != OK) return status;
    //how many attributes this relation has
    attrCnt = rd.attrCnt;
    //Get Attributes
    attrs = new AttrDesc[attrCnt];
    //HeapFileScan object for scanning att catalog
    hfs = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) return status;
    status = hfs->startScan(0, 0, STRING, NULL, EQ);
    if (status != OK) return status;
    attrsLeft = attrCnt - 1;
    //Search for it
    while ((status = hfs->scanNext(rid)) != FILEEOF){
        //if less than zero we have found them all
        if (attrsLeft < 0) {
            break;
        }
	//status check for scanNext()
        if (status != OK) return status;
        status = hfs->getRecord(rec);
        if (status != OK) return status;
        memcpy(&attrs[attrsLeft], rec.data, sizeof(AttrDesc));
        if (strcmp(attrs[attrsLeft].relName, relation.c_str()) == 0) {
            //decrement the number of attributes
            //that we still need to find
            attrsLeft--;
        }
    }//end while loop

    delete hfs;
    //Means can't find
    return OK;
}//end getRelInfo()

/**
 * AttrCatalog destructor 
 **/
AttrCatalog::~AttrCatalog()
{
    // nothing should be needed here
}

