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
    RelDesc rd;
    AttrDesc *attrs;
    int attrCnt;
    HeapFileScan* hfs;
    Record rec;
    RID rid;
    if (relation.empty()){
         
        hfs = new HeapFileScan(RELCATNAME, status);
	if(status != OK) return status;
	status = hfs->startScan(0,0,STRING,NULL,EQ);
	if(status != OK) return status;

        while ((status = hfs->scanNext(rid)) != FILEEOF){
	        if(status != OK) return status;
            
            status = hfs->getRecord(rec);
	        if(status != OK) return status;
	        
            memcpy(&rd, rec.data, sizeof(RelDesc));
            UT_Print(rd.relName);
            cout<<"------------------------------"<<endl;
	        
	    }

    }
    else {
        string type;
        UT_Print(relation);
        cout<<endl;
        status = attrCat->getRelInfo(relation, attrCnt, attrs);
        //type, length, and offset are always zero for some reason
        
        /*for(int i = 0; i < attrCnt; i++){
            //cout<<attrs[i].attrName<<":  "<<attrs[i].attrType<<"  "<<attrs[i].attrLen<<"  "<<attrs[i].attrOffset<<"  "<<"index: "<<i<<endl;
            if(attrs[i].attrType == 0){
                type = "String";
                //cout<<type<<endl;
            }
            else if(attrs[i].attrType == 1){
                type = "Integer";
                //cout<<type<<endl;
            }
            else{
                type = "Float";
                //cout<<type<<endl;
            }
            
                
        }*/
        
        
    }




    return OK;
}
