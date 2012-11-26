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
    RID outRID;

    int offset = 0;
    for (int i = 0; i < attrCnt; i++) {
        offset = 0; //reset offset
        //Iterate through for a match
        for (int j = 0; j < relAttrCnt; j++) {
            if (strcmp(relAttrs[j].relName, attrList[i].relName) == 0) {
                if (attrList[i].attrValue == NULL) {
                    return ATTRTYPEMISMATCH; //Not sure if this is the right error
                }
                memcpy(outputData + offset, (char *)attrList[i].attrValue, attrList[i].attrLen);
            }
            offset += relAttrs[j].attrLen;
        }

    }

    //Done creating record, inserting..
    status = resultRel.insertRecord(outputRec, outRID);
    if (status != OK) { return status; }

    /*
    Insert a tuple with the given attribute values (in attrList) in relation.
    The value of the attribute is supplied in the attrValue member of the attrInfo structure.
    Since the order of the attributes in attrList[] may not be the same as in the relation, you might have to rearrange them before insertion.
    If no value is specified for an attribute, you should reject the insertion as Minirel does not implement NULLs.
    */

    return OK;

}

