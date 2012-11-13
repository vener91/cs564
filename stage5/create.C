#include "catalog.h"


const Status RelCatalog::createRel(const string & relation,
        const int attrCnt,
        const attrInfo attrList[])
{
    Status status;
    RelDesc rd;
    AttrDesc ad;

    if (relation.empty() || attrCnt < 1)
        return BADCATPARM;

    if (relation.length() >= sizeof rd.relName)
        return NAMETOOLONG;

    status = getInfo(relation, rd);
    if (status != OK && status != RELNOTFOUND) return status;
    if (status == OK) return RELEXISTS ;

    //Check if it exists

    strcpy(rd.relName, relation.c_str());
    rd.attrCnt = attrCnt; // number of attributes

    status = addInfo(rd);
    if (status != OK) return status;

    int offset = 0;
    for(int i = 0; i < attrCnt; i++){
        //Copy data
        strcpy(ad.relName, attrList[i].relName);
        strcpy(ad.attrName, attrList[i].attrName);
        ad.attrOffset = offset;
        ad.attrType = attrList[i].attrType;
        ad.attrLen = attrList[i].attrLen;
        offset += attrList[i].attrLen;

        //Add it
        status = attrCat->addInfo(ad);
        if (status != OK) return status;
    }

    return OK;

}
