#include <sys/types.h>
#include <functional>
#include <string.h>
#include <stdio.h>
using namespace std;

#include "error.h"
#include "utility.h"
#include "catalog.h"

// define if debug output wanted


//
// Retrieves and prints information from the catalogs about the for the
// user. If no relation is given (relation is NULL), then it lists all
// the relations in the database, along with the width in bytes of the
// relation, the number of attributes in the relation, and the number of
// attributes that are indexed.  If a relation is given, then it lists
// all of the attributes of the relation, as well as its type, length,
// and offset, whether it's indexed or not, and its index number.
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status RelCatalog::help(const string & relation)
{
    Status status;
    RelDesc* rd;
    int attrCnt = 0;
    HeapFileScan* hfs;
    Record rec;
    RID rid;
    if (relation.empty()){

        hfs = new HeapFileScan(RELCATNAME, status);
        if (status != OK) return status;
        status = hfs->startScan(0, 0, STRING, NULL, EQ);
        if (status != OK) return status;

        /*
        //Count number of relations
        while ((status = hfs->scanNext(rid)) != FILEEOF){
            attrCnt++;
        }
        hfs->resetScan();
        */

        cout << "List of relations"<< endl << endl;
        printf("%-*.*s ", 20, 20, "relName");
        printf("%-*.*s ", 5, 5, "attrCnt");
        printf("\n");

        for(int j = 0; j < 20; j++) {
            putchar('-');
        }
        printf("  ");
        for(int j = 0; j < 5; j++) {
            putchar('-');
        }
        printf("  ");
        printf("\n");
        //attrs = new AttrDesc[attrCnt];
        //Search for it
        while ((status = hfs->scanNext(rid)) != FILEEOF){
            status = hfs->getRecord(rec);
            if (status != OK) return status;
            attrCnt++;
            rd = (RelDesc*)(rec.data);
            printf("%-*.*s  ", 20, 20, rd->relName);
            printf("%-*d  ", 5, rd->attrCnt);
            printf("\n");
        }

        // close scan and data file
        if ((status = hfs->endScan()) != OK)
            return status;
        delete hfs;

        cout << endl << "Number of relations: " << attrCnt << endl;
    } else {
        UT_Print(relation);
    }




    return OK;
}
