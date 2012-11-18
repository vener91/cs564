#include <unistd.h>
#include <fcntl.h>
#include "catalog.h"
#include "utility.h"
#include "heapfile.h"

//
// Loads a file of (binary) tuples from a standard file into the relation.
// Any indices on the relation are updated appropriately.
//
// Returns:
// 	OK on success
// 	an error code otherwise
//

const Status UT_Load(const string & relation, const string & fileName)
{
    Status status;
    RelDesc rd;
    AttrDesc *attrs;
    int attrCnt, i;
    InsertFileScan * iFile;
    int width = 0;
    int records = 0;
    if (relation.empty() || fileName.empty() || relation == string(RELCATNAME)
            || relation == string(ATTRCATNAME))
        return BADCATPARM;

    // open Unix data file

    int fd;
    if ((fd = open(fileName.c_str(), O_RDONLY, 0)) < 0)
        return UNIXERR;

    // get relation data
    status = relCat->getInfo(relation, rd);
    if (status != OK) return status;

    status = attrCat->getRelInfo(relation, attrCnt, attrs);
    if (status != OK) return status;

    for (i = 0; i < attrCnt; i++) {
        cout << "Width: " << attrs[i].attrLen << endl;
        width += attrs[i].attrLen;
    }

    delete attrs;

    // start insertFileScan on relation
    iFile = new InsertFileScan(relation, status);
    if (status != OK) return status;

    // allocate buffer to hold record read from unix file
    char *record;
    if (!(record = new char [width])) return INSUFMEM;

    int nbytes;
    Record rec;

    // read next input record from Unix file and insert it into relation
    while((nbytes = read(fd, record, width)) == width) {
        RID rid;
        rec.data = record;
        rec.length = width;
        //cout << "DEBUG: Loading Record " << records << " - " << nbytes<<endl;
        if ((status = iFile->insertRecord(rec, rid)) != OK) return status;
        records++;
    }

    // close heap file and unix file
    if (close(fd) < 0) return UNIXERR;
    delete iFile;

    return OK;

}

