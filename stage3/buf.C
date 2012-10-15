#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <assert.h>
#include "page.h"
#include "buf.h"

#define ASSERT(c)  { if (!(c)) { \
		       cerr << "At line " << __LINE__ << ":" << endl << "  "; \
                       cerr << "This condition should hold: " #c << endl; \
                       exit(1); \
		     } \
                   }

///////////////////////////////////////////////////////////////////////////////
//
// Title:            Project Stage 3-The Buffer Manager
// Files:            buf.C
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

/** BufMgr constructor**/
BufMgr::BufMgr(const int bufs)
{
    numBufs = bufs;

    bufTable = new BufDesc[bufs];
    memset(bufTable, 0, bufs * sizeof(BufDesc));
    for (int i = 0; i < bufs; i++)
    {
        bufTable[i].frameNo = i;
        bufTable[i].valid = false;
    }

    bufPool = new Page[bufs];
    memset(bufPool, 0, bufs * sizeof(Page));

    int htsize = ((((int) (bufs * 1.2))*2)/2)+1;
    hashTable = new BufHashTbl (htsize);  // allocate the buffer hash table

    clockHand = bufs - 1;
}

/** BufMgr deconstructor**/
BufMgr::~BufMgr() {

    // flush out all unwritten pages
    for (int i = 0; i < numBufs; i++)
    {
        BufDesc* tmpbuf = &bufTable[i];
        if (tmpbuf->valid == true && tmpbuf->dirty == true) {

#ifdef DEBUGBUF
            cout << "flushing page " << tmpbuf->pageNo
                 << " from frame " << i << endl;
#endif

            tmpbuf->file->writePage(tmpbuf->pageNo, &(bufPool[i]));
        }
    }

    delete [] bufTable;
    delete [] bufPool;
}


/** This function allocates a free frame using the clock algorithm;
if necessary, writing a dirty page back to disk.
Returns BUFFEREXCEEDED if all buffer frames are pinned,
UNIXERR if the call to the I/O layer returned an error when a dirty page was being written to disc; else OK.
This private method will get called by the readPage() and allocPage() methods.
If the buffer frame allocated has a valid page in it, then remove the appropriate entry from the hash table.
@param  frame-reference to the frame being allocated
@return Status-status information from function
**/
const Status BufMgr::allocBuf(int & frame) {

    int pinCount = 0;
    //unsigned int initialClockHand = clockHand;
    while(1){
        advanceClock();
        //if valid is false, clear and return frame number
        if (bufTable[clockHand].valid != false) {
            if (bufTable[clockHand].pinCnt > 0) {
                pinCount++;
            }
            //if pinCount reaches numBufs than every buffer frame is being referenced
            if (pinCount == numBufs) {
                return BUFFEREXCEEDED;
            }
            //The two loops in the logic diagram
            if (bufTable[clockHand].refbit == true) { //Check valid bit
                bufTable[clockHand].refbit = false;
                continue; //Loop if valid and refbit set
            } else if (bufTable[clockHand].refbit == false && bufTable[clockHand].pinCnt > 0){
                continue;
            // if valid, unreferenced, not pinned, and dirty; than flush page in frame to disc 
            } else if (bufTable[clockHand].refbit == false && bufTable[clockHand].pinCnt <= 0 && bufTable[clockHand].dirty == true) {
                assert(bufTable[clockHand].pinCnt == 0); //Sanity check
                /**Flush page to disk **/
                flushFile(bufTable[clockHand].file);
                bufTable[clockHand].dirty = false;
            }
            assert(bufTable[clockHand].dirty == false); //Should not be dirty
            assert(bufTable[clockHand].pinCnt == 0);    //Should be zero
            if (bufTable[clockHand].valid == true) {
                assert(bufTable[clockHand].pinCnt == 0);
                hashTable->remove(bufTable[clockHand].file, bufTable[clockHand].pageNo);
            }
        }//end valid == false
        //A frame has been selected for removal 
        bufTable[clockHand].Clear();
        frame = clockHand; //clear frame and return frame pointer
        return OK;
    }//end valid not equal to false
}// end function allocBuf

/**
* This function reads in a page. It will either be in the buffer pool or on disc.
* It first checks whether the page is already in the buffer pool
* by invoking the lookup() method on the hashtable to get a frame number.
* If it is not in buffer pool, Call allocBuf() to allocate a buffer frame 
* and then call the method file->readPage() to read the page from disk
* into the buffer pool frame.
* @param file-a pointer to the file handle
* @param pageNo-the number of the page to be read
* @param page-a pointer to the page inside the frame
* @return Status-status information from function
**/
const Status BufMgr::readPage(File* file, const int pageNo, Page*& page) {

    Status rc; 
    int frameNo = 0;
    //Check to see if the page is in the buffer
    rc = hashTable->lookup(file, pageNo, frameNo);
    //if not OK or HASHNOTFOUND, than an error has been detected
    if (rc != OK && rc != HASHNOTFOUND) {
        return rc; //return error status message
    }
    if (rc == HASHNOTFOUND) {
        //Page is not in the buffer pool.  
        int newFrameNo; //declaration for new frame pointer
        // allocate a buffer frame for the page to be read
        rc = allocBuf(newFrameNo);
        if (rc != OK) {
            return rc;
        }
        //map new buffer addition in the hashTable
        rc = hashTable->insert(file, pageNo, newFrameNo);
        //if not OK than we do not want to call Set()
        if (rc != OK) {
            return rc; 
        }
        //set page from disc in buffer frame
        bufTable[newFrameNo].Set(file, pageNo);
        bufTable[newFrameNo].frameNo = newFrameNo; //update new frame number
        rc = file->readPage(pageNo, &bufPool[newFrameNo]);
        if (rc != OK) {
            //Repair damage
            disposePage(file, pageNo);
            return rc;
        }
        page = &bufPool[newFrameNo]; //pointer to the page to be read
    } else {
        /*Page is in the buffer pool:
          Set the appropriate refbit, 
          increment the pinCnt for the page,
          and then return a pointer to the frame.*/
        bufTable[frameNo].refbit = true;
        bufTable[frameNo].pinCnt++;
        page = &bufPool[frameNo]; //pointer to the page to be read
    }
    //Returns OK if no errors occurred.
    return OK;
}// end function readPage

/**
* This function decrements the pinCnt of the frame 
* containing (file, PageNo) parameters.
* If the page has been modified, 
* set the dirty variable to true. 
* @param file-a pointer to the file handle
* @param pageNo-the number of the page to be read
* @param dirty-is the page being unpinned dirty?
* @return Status-status information from function
**/
const Status BufMgr::unPinPage(File* file, const int pageNo, const bool dirty) {

    Status rc;
    int frameNo;
    //return HASHNOTFOUND if the page is not in the buffer pool hash table
    rc = hashTable->lookup(file, pageNo, frameNo);
    if (rc != OK) {
        return rc;
    }
    // return PAGENOTPINNED if the pin count is already 0.
    if (bufTable[frameNo].pinCnt == 0) {
        rc = PAGENOTPINNED;
        return rc;
    }
    bufTable[frameNo].pinCnt--; //decrement the pin count
    if (dirty == true) {
        bufTable[frameNo].dirty = true;
    }
    return OK;
}//end function unPinPage

/**
* This function allocate an empty page in the specified file by invoking the file->allocatePage() method.
* This method will return the page number of the newly allocated page.
* Then allocBuf() is called to obtain a buffer pool frame.
* Next, an entry is inserted into the hash table and Set() is invoked on the frame to set it up.
* @param file-a pointer to the file handle
* @param pageNo-the number of the page to be read
* @param page-a pointer to the page inside the frame
* @return Status-status information from function
**/
const Status BufMgr::allocPage(File* file, int& pageNo, Page*& page) {

    Status rc;
    int frameNo;
    //I/O call to allocate a page
    rc = file->allocatePage(pageNo);
    if (rc != OK) {
        return rc;
    }
    // allocate a buffer frame for the new page
    rc = allocBuf(frameNo);
    if (rc != OK) {
        return rc;
    }
    //map new buffer addition in the hashTable
    rc = hashTable->insert(file, pageNo, frameNo);
    if (rc != OK) {
        return rc;
    }
    //set page from disc in buffer frame
    bufTable[frameNo].Set(file, pageNo);
    bufTable[frameNo].frameNo = frameNo;
    page = &bufPool[frameNo]; //pointer to the page to be read

    return OK;
}// end function allocPage
/**
* This function handles page disposal from the buffer manager.
* It removes the page from the hashTable and
* clears the buffer frame.
* Finally, file->disposePage is called for disposal on the 
* file system level.
* @param file-a pointer to the file handle
* @param pageNo-the number of the page to be read
* @return Status-status information from function
**/
const Status BufMgr::disposePage(File* file, const int pageNo) {
    // see if page is in the buffer pool
    Status status = OK;
    int frameNo = 0;
    status = hashTable->lookup(file, pageNo, frameNo);
    if (status == OK)
    {
        // clear the page
        bufTable[frameNo].Clear();
    }
    // remove from hashTable
    status = hashTable->remove(file, pageNo);
    // deallocate page in the file
    return file->disposePage(pageNo);
}// end function disposePage
/**
* This function will be called by DB::closeFile()
* when all instances of a file have been closed.
* flushFile() should scan bufTable for pages belonging to the file.
* For each page found:
* remove the page from the hashTable and clear the buffer frame.
* Finally, file->disposePage is called for disposal on the 
* file system level.
* @param file-a pointer to the file handle
* @return Status-status information from function
**/
const Status BufMgr::flushFile(const File* file) {
    Status status;
    for (int i = 0; i < numBufs; i++) {
        BufDesc* tmpbuf = &bufTable[i];
        if (tmpbuf->valid == true && tmpbuf->file == file) {
            if (tmpbuf->pinCnt > 0)
                return PAGEPINNED;

            if (tmpbuf->dirty == true) {
                #ifdef DEBUGBUF
                    cout << "flushing page " << tmpbuf->pageNo<< " from frame " << i << endl;
                #endif
                if ((status = tmpbuf->file->writePage(tmpbuf->pageNo,&(bufPool[i]))) != OK) {
                 return status;
                }
                tmpbuf->dirty = false;
            } // end dirty true
            hashTable->remove(file,tmpbuf->pageNo);
            tmpbuf->file = NULL;
            tmpbuf->pageNo = -1;
            tmpbuf->valid = false;
        }
        else if (tmpbuf->valid == false && tmpbuf->file == file)
        return BADBUFFER;
    }//end for loop
  return OK;
}// end function flushFile

/**
* This function prints out the contents of the 
* buffer manager. Used for testing purposes.
* @param void
* @return void
**/
void BufMgr::printSelf(void)
{
    BufDesc* tmpbuf;
    cout << endl << "Print buffer...\n";
    for (int i=0; i<numBufs; i++) {
        tmpbuf = &(bufTable[i]);
        cout << i << "\t" << (char*)(&bufPool[i])
             << "\tpinCnt: " << tmpbuf->pinCnt
             << "\tpageNo:" << tmpbuf->pageNo;

        if (tmpbuf->valid == true)
            cout << "\tvalid";
        if (tmpbuf->dirty == true)
            cout << "\tdirty";
        if (tmpbuf->refbit == true)
            cout << "\trefbit";

        cout << endl;
    };
}// end function printSelf


