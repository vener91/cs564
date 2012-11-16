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
        memcpy(&record, rec.data, sizeof(RelDesc));
        cout << "DEBUG REL Getting: " << record.relName << " - " << relation.c_str() << " - " << strcmp(record.relName, relation.c_str()) << endl;
        if (strcmp(record.relName, relation.c_str()) == 0) {
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
    Record rec;
    RelDesc record;

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
        cout << "DEBUG REL Removing: " << (char*)rec.data << endl;
        memcpy(&record, rec.data, sizeof(RelDesc));
        if (strcmp(record.relName, relation.c_str()) == 0) {
            hfs->deleteRecord();
            delete hfs;
            return OK;
        }
    }

    delete hfs;
    //Means can't find
    return RELNOTFOUND;


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

    hfs = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) return status;
    status = hfs->startScan(0, 0, STRING, NULL, EQ);
    if (status != OK) return status;

    //Search for it
    while ((status = hfs->scanNext(rid)) != FILEEOF){
        if (status != OK) return status;
        status = hfs->getRecord(rec);
        if (status != OK) return status;
        cout << "DEBUG ATTR Getting: " << (char*)rec.data << endl;
        memcpy(&record, rec.data, sizeof(RelDesc));
        if (strcmp(record.relName, relation.c_str()) == 0 && strcmp(record.attrName, attrName.c_str()) == 0) {
            delete hfs;
            return OK;
        }
    }

    delete hfs;
    //Means can't find
    return ATTRNOTFOUND;

}


const Status AttrCatalog::addInfo(AttrDesc & record)
{
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
    return OK;

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

    hfs = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) return status;
    status = hfs->startScan(0, 0, STRING, NULL, EQ);
    if (status != OK) return status;

    //Search for it
    while ((status = hfs->scanNext(rid)) != FILEEOF){
        if (status != OK) return status;
        status = hfs->getRecord(rec);
        if (status != OK) return status;
        cout << "DEBUG ATTR Removing: " << (char*)rec.data << endl;
        memcpy(&record, rec.data, sizeof(RelDesc));
        if (strcmp(record.relName, relation.c_str()) == 0 && strcmp(record.attrName, attrName.c_str()) == 0) {
            hfs->deleteRecord();
            delete hfs;
            return OK;
        }
    }

    delete hfs;
    //Means can't find
    return ATTRNOTFOUND;
}


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

    if (relation.empty()) return BADCATPARM;

    status = relCat->getInfo(relation, rd);
    if (status != OK) return status;

    attrCnt = rd.attrCnt;

    //Get Attributes
    attrs = new AttrDesc[attrCnt];

    hfs = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) return status;
    status = hfs->startScan(0, 0, STRING, NULL, EQ);
    if (status != OK) return status;

    attrsLeft = attrCnt - 1;
    //Search for it
    while ((status = hfs->scanNext(rid)) != FILEEOF){
        if (attrsLeft < 0) {
            break;
        }
        if (status != OK) return status;
        status = hfs->getRecord(rec);
        if (status != OK) return status;
        cout << "DEBUG ATTR Getting: " << (char*)rec.data << endl;
        memcpy(&attrs[attrsLeft], rec.data, sizeof(RelDesc));
        if (strcmp(attrs[attrsLeft].relName, relation.c_str()) == 0) {
            attrsLeft--;
        }
    }

    delete hfs;
    //Means can't find
    return OK;
}


AttrCatalog::~AttrCatalog()
{
    // nothing should be needed here
}

