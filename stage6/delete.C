#include "catalog.h"
#include "query.h"


/*
 * Deletes records from a specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

#include "stdio.h"
#include "stdlib.h"
const Status QU_Delete(const string & relation,
		       const string & attrName,
		       const Operator op,
		       const Datatype type,
		       const char *attrValue)
{
    Status status;
    AttrDesc attrDesc;
    const char* filter;
    int resultTupCnt = 0;

    //if no attrName given then delete entire relation
    if(attrName.length() == 0){
       status = relCat->destroyRel(relation);
       return status;
    }
    HeapFileScan relScan(relation, status);
    if (status != OK) { return status; }

    status = attrCat->getInfo(relation, attrName, attrDesc);
    if (status != OK) { return status; }

    int tmpInt;
    float tmpFloat;
    switch (type) {
        case INTEGER:
            tmpInt = atoi(attrValue);
            filter = (char*)&tmpInt;
            break;
        case FLOAT:
            tmpFloat = atof(attrValue);
            filter = (char*)&tmpFloat;
            break;
        case STRING:
            filter = attrValue;
            break;
    }

    status = relScan.startScan(attrDesc.attrOffset, attrDesc.attrLen, type, filter, op);
    if (status != OK) { return status; }

    RID relRID;
    while (relScan.scanNext(relRID) == OK) {
        status = relScan.deleteRecord();
        if (status != OK) { return status; }
        resultTupCnt++;
    }

    printf("deleted %d result tuples \n", resultTupCnt);

    // part 6
    return OK;
}


