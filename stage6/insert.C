#include "catalog.h"
#include "query.h"


/*
 * Inserts a record into the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Insert(const string & relation,
        const int attrCnt,
        const attrInfo attrList[])
{

    Status status;
    int relAttrCnt;
    AttrDesc* relAttrs;

    InsertFileScan resultRel(relation, status);
    if (status != OK) { return status; }

    status = attrCat->getRelInfo(relation, relAttrCnt, relAttrs);
    if (status != OK) { return status; }

    int reclen = 0;
    for (int i = 0; i < relAttrCnt; i++) {
        reclen += relAttrs[i].attrLen;
    }

    char outputData[reclen];
    Record outputRec;
    outputRec.data = (void *) outputData;
    outputRec.length = reclen;

    for (int i = 0; i < attrCnt; i++) {
        //Iterate through for a match
        for (int j = 0; j < relAttrCnt; j++) {
            if (strcmp(relAttrs[j].attrName, attrList[i].attrName) == 0) {
                if (attrList[i].attrValue == NULL) {
                    return ATTRTYPEMISMATCH; //Not sure if this is the right error
                }

                char* actualAttrValue;
                int tmpInt;
                float tmpFloat;
                switch (attrList[i].attrType) {
                    case INTEGER:
                        tmpInt = atoi((char*)attrList[i].attrValue);
                        actualAttrValue = (char*)&tmpInt;
                        break;
                    case FLOAT:
                        tmpFloat = atof((char*)attrList[i].attrValue);
                        actualAttrValue = (char*)&tmpFloat;
                        break;
                    case STRING:
                        actualAttrValue = (char*)attrList[i].attrValue;
                        break;
                }
                memcpy(outputData + relAttrs[j].attrOffset, actualAttrValue, relAttrs[j].attrLen);
            }
        }

    }

    RID outRID;
    //Done creating record, inserting..
    status = resultRel.insertRecord(outputRec, outRID);
    if (status != OK) { return status; }

    return OK;

}

