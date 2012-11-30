#include "catalog.h"
#include "query.h"


// forward declaration
const Status ScanSelect(const string & result,
			const int projCnt,
			const AttrDesc projNames[],
			const AttrDesc *attrDesc,
			const Operator op,
			const char *filter,
			const int reclen);

/*
 * Selects records from the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Select(const string & result,
        const int projCnt,
        const attrInfo projNames[],
        const attrInfo *attr,
        const Operator op,
        const char *attrValue)
{
    Status status;
    AttrDesc attrDesc;
    AttrDesc* projNamesDesc;
    int reclen;
    Operator myOp;
    const char* filter;

    reclen = 0;
    //an array of attrDesc to hold all the descriptions for the projection
    projNamesDesc = new AttrDesc[projCnt];
    //tabulate the total length of the projection record
    for (int i = 0; i < projCnt; i++) {
        Status status = attrCat->getInfo(projNames[i].relName, projNames[i].attrName, projNamesDesc[i]);
        reclen += projNamesDesc[i].attrLen;
        if (status != OK) { return status; }
    }
    //implementation of select operation
    if (attr != NULL) { //SELECT ALL
        //get info for attr
        //e.g. attr op attrvalue
        status = attrCat->getInfo(string(attr->relName), string(attr->attrName), attrDesc);

        int tmpInt;
        float tmpFloat;
        //convert to proper data type
        switch (attr->attrType) {
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
        myOp = op;
    //if null than there is no where clause
    //select all tuple for projection attributes
    } else {
    
        strcpy(attrDesc.relName, projNames[0].relName);
        strcpy(attrDesc.attrName, projNames[0].attrName);
        attrDesc.attrOffset = 0;
        attrDesc.attrLen = 0;
        attrDesc.attrType = STRING;
        filter = NULL;
        myOp = EQ;
    }

    status = ScanSelect(result, projCnt, projNamesDesc, &attrDesc, myOp, filter, reclen);
    if (status != OK) { return status; }

    return OK;
}


#include "stdio.h"
#include "stdlib.h"
const Status ScanSelect(const string & result,
        const int projCnt,
        const AttrDesc projNames[],
        const AttrDesc *attrDesc,
        const Operator op,
        const char *filter,
        const int reclen)
{
    Status status;
    //used to keep track of how many total tuples are selected
    int resultTupCnt = 0;

    // open the result table
    InsertFileScan resultRel(result, status);
    if (status != OK) { return status; }

    char outputData[reclen];
    Record outputRec;
    outputRec.data = (void *) outputData;
    outputRec.length = reclen;
    // start scan on outer table
    HeapFileScan relScan(attrDesc->relName, status);
    if (status != OK) { return status; }

    status = relScan.startScan(attrDesc->attrOffset, attrDesc->attrLen, (Datatype) attrDesc->attrType, filter, op);
    if (status != OK) { return status; }

    // scan outer table
    RID relRID;
    Record relRec;
    while (relScan.scanNext(relRID) == OK) {
        status = relScan.getRecord(relRec);
        ASSERT(status == OK);

        // we have a match, copy data into the output record
        int outputOffset = 0;
        for (int i = 0; i < projCnt; i++) {
            memcpy(outputData + outputOffset, (char *)relRec.data + projNames[i].attrOffset, projNames[i].attrLen);
            outputOffset += projNames[i].attrLen;
       
        } // end copy attrs

        // add the new record to the output relation
        RID outRID;
        status = resultRel.insertRecord(outputRec, outRID);
        ASSERT(status == OK);
        resultTupCnt++;
    }
    printf("selected %d result tuples \n", resultTupCnt);
    return OK;
}
