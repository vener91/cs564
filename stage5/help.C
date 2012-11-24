#include <sys/types.h>
#include <functional>
#include <string.h>
#include <stdio.h>
using namespace std;

#include "error.h"
#include "utility.h"
#include "catalog.h"

// define if debug output wanted


/**
 * If a relname is not specified, help lists the names of all the relations in the database.
 * Otherwise, it lists the name, type, length and offset of each attribute together with any other information you feel is useful.
 *
 * Returns:
 * OK on success
 * error code otherwise
 **/
const Status RelCatalog::help(const string & relation)
{
    Status status;
    RelDesc* rd;
    //length of attrs
    int attrCnt = 0;
    HeapFileScan* hfs;
    Record rec;
    RID rid;
    //an array of all the attribute description data for a relation
    AttrDesc *attrs;
    //type is: String, Integer, and Float
    string type;
    //if empty print relations catalog 
    if (relation.empty()){

        hfs = new HeapFileScan(RELCATNAME, status);
        if (status != OK) return status;
        status = hfs->startScan(0, 0, STRING, NULL, EQ);
        if (status != OK) return status;
        //formatted relations catalog output
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
    } else { // if a relation given print out the description information for its attributes
        cout << endl;
        status = attrCat->getRelInfo(relation, attrCnt, attrs);
        cout << "Relation: " << relation << endl;
	//add a border 
        cout << "------------------------------------" << endl;
        if (status != OK) return status;
        for(int i = 0; i < attrCnt; i++) {
            //convert attrType from int representation back to string for output
            if(attrs[i].attrType == 0){
                type = "String";
            }
            else if(attrs[i].attrType == 1){
                type = "Integer";
            }
            else{
                type = "Float";
            }
            cout << "Attribute: " << attrs[i].attrName << endl;
            cout << "Type: " << type << ", ";
            cout << "Length: " << attrs[i].attrLen << ", ";
            cout << "Offset: " << attrs[i].attrOffset << endl;  
        
        }//end for loop
        
    }//end else block
    //help executed OK
    return OK;
}//end help()
