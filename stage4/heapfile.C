#include "heapfile.h"
#include "error.h"
#include <stdio.h>
#include <assert.h>

///////////////////////////////////////////////////////////////////////////////
//
// Title:            Project Stage 4-The Minirel HeapFile Manager 
// Files:            heapfile.C
// Semester:         CS-564 Fall 2012
//
// Team Name:        Krankit
// Member Name:      TerChrng Ng
// CS Login:         ter@cs.wisc.edu
// Student ID:       906 515 5054
// ----------------------------------------------------------------------------
// Member Name:      Nate DiPiazza
// CS Login:         ndipiazz@cs.wisc.edu
// Student ID:       906 653 1493
//
// Lecturer's Name:  AnHai Doan
// Lab Section:      001
///////////////////////////////////////////////////////////////////////////////

/**
routine to create a heapfile
@param fileName-file handle of file containing data
@return Status-status information from function
**/
const Status createHeapFile(const string fileName)
{
    File*           file;
    Status          status;
    FileHdrPage*    hdrPage;
    int             hdrPageNo;
    int             newPageNo;
    Page*           newPage;

    // try to open the file. This should return an error
    status = db.openFile(fileName, file);
    // if file exists we don't need to create one
    if (status != OK) {
        // file doesn't exist. First create it and allocate
        // an empty header page and data page.
        status = db.createFile(fileName);
        
        status = db.openFile(fileName, file);
        if (status != OK) return status;

        status = bufMgr->allocPage(file, newPageNo, newPage);
        if (status != OK) return status;

        //Initialize values
        hdrPage = (FileHdrPage*) newPage; // must cast page as FileHdrPage
        //designate first page as header page
        hdrPageNo = newPageNo;

        strcpy(hdrPage->fileName, fileName.c_str());
        //assert that fileName has been set
        assert(strlen(hdrPage->fileName) != 0);

        //First data page
        status = bufMgr->allocPage(file, newPageNo, newPage);
        if (status != OK) return status;

        newPage->init(newPageNo);

        //Update header with info
        hdrPage->firstPage = newPageNo;
        hdrPage->lastPage = newPageNo;
        hdrPage->pageCnt = 1;
        hdrPage->recCnt = 0;

        //Unpin and make dirty
        status = bufMgr->unPinPage(file, hdrPageNo, true);
        if (status != OK) return status;
        status = bufMgr->unPinPage(file, newPageNo, true);
        if (status != OK) return status;
        //not that heapfile setup file no longer needed
        status = db.closeFile(file);
        if (status != OK) return status;

        return OK;
    }
    return FILEEXISTS;
}//end createHeapFile

// routine to destroy a heapfile
const Status destroyHeapFile(const string fileName)
{
	return (db.destroyFile (fileName));
}

// constructor opens the underlying file
HeapFile::HeapFile(const string & fileName, Status& returnStatus)
{
    Status 	status;
    Page*	pagePtr;
    //inform user that file has been opened
    cout << "opening file " << fileName << endl;

    // open the file and read in the header page and the first data page
    if ((status = db.openFile(fileName, filePtr)) == OK) {

        status = filePtr->getFirstPage(headerPageNo);
        if (status != OK) {
            returnStatus = status;
            return;
        }
        status = bufMgr->readPage(filePtr, headerPageNo, pagePtr);
        if (status != OK) {
            returnStatus = status;
            return;
        }
        headerPage = (FileHdrPage*)pagePtr;
        hdrDirtyFlag = false;
        // current page is by default first page
        curPageNo = headerPage->firstPage;
        
        bufMgr->readPage(filePtr, curPageNo, curPage);
        if (status != OK) {
            returnStatus = status;
            return;
        }
        //default settings for new heapFile
        curDirtyFlag = false;
        curRec = NULLRID;

    } else {
        cerr << "open of heap file failed\n";
        returnStatus = status;
        return;
    }
    //if code reached than heapFile has been constucted properly
    returnStatus = OK;
}

// the destructor closes the file
HeapFile::~HeapFile()
{
    Status status;
    //inform user that file has been destroyed
    cout << "invoking heapfile destructor on file " << headerPage->fileName << endl;

    //see if there is a pinned data page. If so, unpin it
    if (curPage != NULL)
    {
    	status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
		curPage = NULL;
		curPageNo = 0;
		curDirtyFlag = false;
		if (status != OK) cerr << "error in unpin of data page\n";
    }

	//unpin the header page
    status = bufMgr->unPinPage(filePtr, headerPageNo, hdrDirtyFlag);
    if (status != OK) cerr << "error in unpin of header page\n";

	
	//done with the file
	status = db.closeFile(filePtr);
    if (status != OK)
    {
		cerr << "error in closefile call\n";
		Error e;
		e.print (status);
    }
}

//accessor function
//@return number of records in heap file
const int HeapFile::getRecCnt() const
{
  return headerPage->recCnt;
}
/**
retrieve an arbitrary record from a file.
if record is not on the currently pinned page, the current page
is unpinned and the required page is read into the buffer pool
and pinned. returns a pointer to the record via the rec parameter
@param rid-record id
@param rec-desired record
@return Status-status information from function
**/
const Status HeapFile::getRecord(const RID & rid, Record & rec)
{
    Status status;
    //if false, record is not on this page
    if (rid.pageNo != curPageNo) {
        bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        //set current page to page with desired record
        curPageNo = rid.pageNo;
        bufMgr->readPage(filePtr, curPageNo, curPage);
        curDirtyFlag = false;
    }
    //get the record
    status = curPage->getRecord(rid, rec);
    if (status != OK) return status;
    //if reached record was found successfully 
    return OK;

}
//constructor for HeapFileScan
//this class is used to:
//retrieve all records from a HeapFile,
//or only those records that match a specified predicate.
//delete records in a file 
HeapFileScan::HeapFileScan(const string & name,
			   Status & status) : HeapFile(name, status)
{
    filter = NULL;
}

const Status HeapFileScan::startScan(const int offset_,
				     const int length_,
				     const Datatype type_,
				     const char* filter_,
				     const Operator op_)
{
    if (!filter_) {
        // no filtering requested                        
        filter = NULL;
        return OK;
    }
    // validate proper parameters used
    if (((offset_ < 0) || (length_ < 1)) ||
        ((type_ != STRING) && (type_ != INTEGER) && (type_ != FLOAT)) ||
        (((type_ == INTEGER) && (length_ != sizeof(int)))
         || ((type_ == FLOAT) && (length_ != sizeof(float)))) ||
        ((op_ != LT) && (op_ != LTE) && (op_ != EQ) && (op_ != GTE) && (op_ != GT) && (op_ != NE)))
    {
        return BADSCANPARM;
    }
    //all fields valid, setup
    offset = offset_;
    length = length_;
    type = type_;
    filter = filter_;
    op = op_;

    return OK;
}
//handles book keeping related to destroying HeapFileScan object
//@return Status-status information from function
const Status HeapFileScan::endScan()
{
    Status status;
    // generally must unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        curPage = NULL;
        curPageNo = 0;
        curDirtyFlag = false;
        return status;
    }
    return OK;
}
//destructor for HeapFileScan class
HeapFileScan::~HeapFileScan()
{
    endScan();
}
//note: not needed for phase 4
const Status HeapFileScan::markScan()
{
    // make a snapshot of the state of the scan
    markedPageNo = curPageNo;
    markedRec = curRec;
    return OK;
}
//note: not needed for phase 4
const Status HeapFileScan::resetScan()
{
    Status status;
    if (markedPageNo != curPageNo)
    {
		if (curPage != NULL)
		{
			status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
			if (status != OK) return status;
		}
		// restore curPageNo and curRec values
		curPageNo = markedPageNo;
		curRec = markedRec;
		// then read the page
		status = bufMgr->readPage(filePtr, curPageNo, curPage);
		if (status != OK) return status;
		curDirtyFlag = false; // it will be clean
    }
    else curRec = markedRec;
    return OK;
}
/**
advance the scan to the next record
@param outRid-RID of next record
@return Status-status information from function
**/
const Status HeapFileScan::scanNext(RID& outRid)
{
    Status 	status = OK;
    RID		nextRid;
    RID		tmpRid;
    int 	nextPageNo;
    Record      rec;

    //Get firstPage
    nextPageNo = curPageNo;
    while(true){
        //Get the next page
        if (nextPageNo == -1) {
            return FILEEOF; //No more page
        }

        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        if (status != OK) return status;

        //cout << "Page: " << nextPageNo << endl;
        status = bufMgr->readPage(filePtr, nextPageNo, curPage);
        if (status != OK) return status;
        curPageNo = nextPageNo;
        curDirtyFlag = false;

        //Iterate over all records
        if (curRec.pageNo == NULLRID.pageNo && curRec.slotNo == NULLRID.slotNo) {
            status = curPage->firstRecord(nextRid);
        } else {
            status = OK; //Force it to go into inner loop
        }
        if (status != OK && status != NORECORDS) {
            return status;
        } else if(status == OK){ //Got records
            if (curRec.pageNo != NULLRID.pageNo && curRec.slotNo != NULLRID.slotNo) {
                status = curPage->nextRecord(curRec, nextRid);
                tmpRid = nextRid;
            }

            if (status != OK && status != ENDOFPAGE) {
                return status;
            } else if (status == OK) { //If the last record, skip
                while (true) {
                    //Try to match record
                    status = curPage->getRecord(nextRid, rec);
                    if (status != OK) return status;
                    curRec = nextRid;
                    if (matchRec(rec)){ //If match
                        outRid = nextRid;
                        return OK;
                    }
                    tmpRid = nextRid;

                    //Did not match, get the next record
                    status = curPage->nextRecord(tmpRid, nextRid);
                    if (status == ENDOFPAGE) {
                        curRec = NULLRID;
                        break; //Finished scanning, but no records
                    }
                }
            } else {
                curRec = NULLRID;
            }
        }
        status = curPage->getNextPage(nextPageNo);
        if (status != OK) return status;
    }
    return OK;
}
/**
Returns the pointer to the current record.
The page is left pinned and the scan logic
is required to unpin the page.
@param rec-reference to desired record
@return Status-status information from function
**/
const Status HeapFileScan::getRecord(Record & rec)
{
    return curPage->getRecord(curRec, rec);
}

// delete record from file.
const Status HeapFileScan::deleteRecord()
{
    Status status;

    assert(curPageNo == curRec.pageNo);
    status = bufMgr->readPage(filePtr, curPageNo, curPage);
    if (status != OK) return status;
    status = curPage->deleteRecord(curRec);
    curDirtyFlag = true;
    status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
    if (status != OK) return status;

    // reduce count of number of records in the file
    headerPage->recCnt--;
    hdrDirtyFlag = true;
    return status;
}
// mark current page of scan dirty
const Status HeapFileScan::markDirty()
{
    curDirtyFlag = true;
    return OK;
}
/**
This function checks to see if
the record in question matches
the specified predicate.
@param rec-reference to record 
@return true if matched, else false
**/
const bool HeapFileScan::matchRec(const Record & rec) const
{
    // no filtering requested
    if (!filter) return true;

    // see if offset + length is beyond end of record
    // maybe this should be an error???
    if ((offset + length -1 ) >= rec.length)
	return false;

    float diff = 0;                       // < 0 if attr < fltr
    switch(type) {

    case INTEGER:
        int iattr, ifltr;                 // word-alignment problem possible
        memcpy(&iattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ifltr,
               filter,
               length);
        diff = iattr - ifltr;
        break;

    case FLOAT:
        float fattr, ffltr;               // word-alignment problem possible
        memcpy(&fattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ffltr,
               filter,
               length);
        diff = fattr - ffltr;
        break;

    case STRING:
        diff = strncmp((char *)rec.data + offset,
                       filter,
                       length);
        break;
    }

    switch(op) {
    case LT:  if (diff < 0.0) return true; break;
    case LTE: if (diff <= 0.0) return true; break;
    case EQ:  if (diff == 0.0) return true; break;
    case GTE: if (diff >= 0.0) return true; break;
    case GT:  if (diff > 0.0) return true; break;
    case NE:  if (diff != 0.0) return true; break;
    }

    return false;
}
/**
Constructor for the InsertFileScan class
Used to insert records into a file.
**/
InsertFileScan::InsertFileScan(const string & name,
                               Status & status) : HeapFile(name, status)
{
  //Do nothing. Heapfile constructor will bread the header page and the first
  // data page of the file into the buffer pool
}
//Destructor for InsertFileScan 
InsertFileScan::~InsertFileScan()
{
    Status status;
    // unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, true);
        curPage = NULL;
        curPageNo = 0;
        if (status != OK) cerr << "error in unpin of data page\n";
    }
}
/**
Insert a record into the file
assigns the RID of the record to outRid
@param rec-record to be inserted
@param outRid-the RID of the successfully inserted record
@return Status-status information from function
**/
const Status InsertFileScan::insertRecord(const Record & rec, RID& outRid)
{
    Page*	newPage;
    int		newPageNo;
    Status	status;
    bool unpinstatus;
    RID		rid;

    // check for very large records
    if ((unsigned int) rec.length > PAGESIZE-DPFIXED)
    {
        // will never fit on a page, so don't even bother looking
        return INVALIDRECLEN;
    }
    //get lastPage
    newPageNo = headerPage->lastPage;
    unpinstatus = false;
    while(true){
        //Get the next page
        if (newPageNo == -1) {
            //flag == -1 so there are no more pages
            //Need to allocate new page
            status = bufMgr->allocPage(filePtr, newPageNo, newPage);
            if (status != OK) return status;
            //set allocated page as next page
            curPage->setNextPage(newPageNo);
            //new page book keeping block////////////////////////////////
            curDirtyFlag = true;
            status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
            if (status != OK) return status;

            newPage->init(newPageNo);
            headerPage->lastPage = newPageNo;
            curPageNo = newPageNo;
            curPage = newPage;
            unpinstatus = true;
            curDirtyFlag = true;
            /////////////////////////////////////////////////////////////
        } else {

            if (curPage != NULL) {
                status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
                if (status != OK) return status;
            }

            status = bufMgr->readPage(filePtr, newPageNo, curPage);
            if (status != OK) return status;
            curPageNo = newPageNo;
            curDirtyFlag = false;
            unpinstatus = false;
        }
        //ready to actually insert the record
        status = curPage->insertRecord(rec, rid);
        //make sure that there was enough space
        //and that it was inserted successfully
        if (status == OK || status != NOSPACE) {
            // rid is now the current record
            unpinstatus = true;
            //file modified, set dirty bit
            curDirtyFlag = true;
            outRid = rid;
            curRec = rid;
            //Update header
            headerPage->recCnt++;
            //header modified as well
            hdrDirtyFlag = true;
        }
        //if true, return error status
        if (status != OK && status != NOSPACE) return status;
        //newPageNo is either newly setup page or equal to current page
        status = curPage->getNextPage(newPageNo);
        if (status != OK) return status;
        //done inserting record
        if (unpinstatus == true) {
            return OK;
        }

    }
    return OK; //Would never reach here
}//end insertRecord()
