#include "catalog.h"

//
// Destroys a relation. It performs the following steps:
//
// 	removes the catalog entry for the relation
// 	destroys the heap file containing the tuples in the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status RelCatalog::destroyRel(const string & relation)
{
    Status status;

    if (relation.empty() ||
            relation == string(RELCATNAME) ||
            relation == string(ATTRCATNAME))
        return BADCATPARM;

    //remove all tuples from relcat and attrcat for this relation
    status = attrCat->dropRelation(relation);
    if (status != OK) return status;
    status = destroyHeapFile(relation);
    if (status != OK) return status;

    return OK;
}


//
// Drops a relation. It performs the following steps:
//
// 	removes the catalog entries for the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status AttrCatalog::dropRelation(const string & relation)
{

    HeapFileScan* hfs;
    Status status;
    AttrDesc *attrs;
    Record rec;
    RID rid;
    int attrCnt, i;

    if (relation.empty()) return BADCATPARM;

    hfs = new HeapFileScan(RELCATNAME, status);
    if (status != OK) return status;
    status = hfs->startScan(0, 0, STRING, NULL, EQ);
    if (status != OK) return status;

    //Search for it
    while ((status = hfs->scanNext(rid)) != FILEEOF){
        if (status != OK) return status;
        status = hfs->getRecord(rec);
        if (status != OK) return status;
        //cout << "DEBUG Destory.c Rel: " << rec.data << endl;
        if (strcmp((char*)rec.data, relation.c_str()) == 0) {
            //Remove record
            hfs->deleteRecord();
        }
    }
    status = hfs->endScan();
    if (status != OK) return status;

    delete hfs;

    hfs = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) return status;
    status = hfs->startScan(0, 0, STRING, NULL, EQ);
    if (status != OK) return status;

    //Search for it
    while ((status = hfs->scanNext(rid)) != FILEEOF){
        if (status != OK) return status;
        status = hfs->getRecord(rec);
        if (status != OK) return status;
        //cout << "DEBUG Destory.c Rel: " << rec.data << endl;
        if (strcmp((char*)rec.data, relation.c_str()) == 0) {
            //Remove record
            hfs->deleteRecord();
        }
    }
    status = hfs->endScan();
    if (status != OK) return status;

    delete hfs;
    return OK;
}


