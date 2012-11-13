#include "catalog.h"


RelCatalog::RelCatalog(Status &status) :
    HeapFile(RELCATNAME, status)
{
    // nothing should be needed here
}


const Status RelCatalog::getInfo(const string & relation, RelDesc &record)
{
    if (relation.empty())
        return BADCATPARM;

    Status status;
    Record rec;
    RID rid;

    HeapFileScan* hfs = new HeapFileScan(RELCATNAME, status);
    if (status != OK) return status;
    status = hfs->startScan(0, 0, STRING, NULL, EQ);
    if (status != OK) return status;

    //Search for it
    while ((status = hfs->scanNext(rid)) != FILEEOF){
        if (status != OK) return status;
        status = hfs->getRecord(rec);
        if (status != OK) return status;
        cout << "DEBUG: " << rec.data << endl;
        if (strcmp((char*)rec.data, relation.c_str()) == 0) {
            memcpy(&record, rec.data, sizeof(RelDesc));
            delete hfs;
            return OK;
        }
    }

    delete hfs;
    //Means can't find
    return RELNOTFOUND;

}


const Status RelCatalog::addInfo(RelDesc & record)
{
    RID rid;
    Record rec;
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
}

const Status RelCatalog::removeInfo(const string & relation)
{
    Status status;
    RID rid;
    HeapFileScan*  hfs;

    if (relation.empty()) return BADCATPARM;



}


RelCatalog::~RelCatalog()
{
    // nothing should be needed here
}


AttrCatalog::AttrCatalog(Status &status) :
    HeapFile(ATTRCATNAME, status)
{
    // nothing should be needed here
}


const Status AttrCatalog::getInfo(const string & relation,
        const string & attrName,
        AttrDesc &record)
{

    Status status;
    RID rid;
    Record rec;
    HeapFileScan*  hfs;

    if (relation.empty() || attrName.empty()) return BADCATPARM;




}


const Status AttrCatalog::addInfo(AttrDesc & record)
{
    RID rid;
    InsertFileScan*  ifs;
    Status status;





}


const Status AttrCatalog::removeInfo(const string & relation,
        const string & attrName)
{
    Status status;
    Record rec;
    RID rid;
    AttrDesc record;
    HeapFileScan*  hfs;

    if (relation.empty() || attrName.empty()) return BADCATPARM;

}


const Status AttrCatalog::getRelInfo(const string & relation,
        int &attrCnt,
        AttrDesc *&attrs)
{
    Status status;
    RID rid;
    Record rec;
    HeapFileScan*  hfs;

    if (relation.empty()) return BADCATPARM;




}


AttrCatalog::~AttrCatalog()
{
    // nothing should be needed here
}

