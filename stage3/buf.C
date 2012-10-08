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

//----------------------------------------
// Constructor of the class BufMgr
//----------------------------------------

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


const Status BufMgr::allocBuf(int & frame)
{
/*
Allocates a free frame using the clock algorithm; if necessary, writing a dirty page back to disk. Returns BUFFEREXCEEDED if all buffer frames are pinned, UNIXERR if the call to the I/O layer returned an error when a dirty page was being written to disk and OK otherwise.  This private method will get called by the readPage() and allocPage() methods described below.

Make sure that if the buffer frame allocated has a valid page in it, that you remove the appropriate entry from the hash table.
*/

}


const Status BufMgr::readPage(File* file, const int PageNo, Page*& page)
{

/*
First check whether the page is already in the buffer pool by invoking the lookup() method on the hashtable to get a frame number.  There are two cases to be handled depending on the outcome of the lookup() call:

Case 1) Page is not in the buffer pool.  Call allocBuf() to allocate a buffer frame and then call the method file->readPage() to read the page from disk into the buffer pool frame. Next, insert the page into the hashtable. Finally, invoke Set() on the frame to set it up properly. Set() will leave the pinCnt for the page set to 1.  Return a pointer to the frame containing the page via the page parameter.

Case 2)  Page is in the buffer pool.  In this case set the appropriate refbit, increment the pinCnt for the page, and then return a pointer to the frame containing the page via the page parameter.

Returns OK if no errors occurred, UNIXERR if a Unix error occurred, BUFFEREXCEEDED if all buffer frames are pinned, HASHTBLERROR if a hash table error occurred.
*/



}


const Status BufMgr::unPinPage(File* file, const int PageNo, const bool dirty)
{
/*
Decrements the pinCnt of the frame containing (file, PageNo) and, if dirty == true, sets the dirty bit.  Returns OK if no errors occurred, HASHNOTFOUND if the page is not in the buffer pool hash table, PAGENOTPINNED if the pin count is already 0.
*/



}

const Status BufMgr::allocPage(File* file, int& pageNo, Page*& page)
{

/*
This call is kind of weird.  The first step is to to allocate an empty page in the specified file by invoking the file->allocatePage() method.
This method will return the page number of the newly allocated page.  Then allocBuf() is called to obtain a buffer pool frame.
Next, an entry is inserted into the hash table and Set() is invoked on the frame to set it up properly.
The method returns both the page number of the newly allocated page to the caller via the pageNo parameter and a pointer to the buffer frame
allocated for the page via the page parameter. Returns OK if no errors occurred, UNIXERR if a Unix error occurred, BUFFEREXCEEDED if all buffer
frames are pinned and HASHTBLERROR if a hash table error occurred.
*/
    Error error;
    Status rc = file->allocatePage(pageNo);
    if (rc != 0) {
        error.print(rc);
    }

    int initialClockHand = clockHand;
    while(1){
        clockHand++; //Increment clockhand
        clockHand = clockHand % numBufs; //Its a clock!

        //The two loops in the logic diagram
        if (bufTable[clockHand].valid == true && bufTable[clockHand].refbit == true) { //Check valid bit
            if (initialClockHand == clockHand) {
                return BUFFEREXCEEDED;
            }
            continue; //Loop if valid and refbit set
        } else if (bufTable[clockHand].valid == true && bufTable[clockHand].refbit == false && bufTable[clockHand].pinCnt > 0){
            if (initialClockHand == clockHand) {
                return BUFFEREXCEEDED;
            }
            continue;
        } else if (bufTable[clockHand].valid == true && bufTable[clockHand].refbit == false && bufTable[clockHand].pinCnt <= 0 && bufTable[clockHand].dirty == true) {
            assert(bufTable[clockHand].pinCnt == 0); //Sanity check
            /** We are suppose to flush page to disk **/
        }
        assert(bufTable[clockHand].dirty == false); //Should not be dirty
        bufTable[clockHand].Set(file, pageNo);


    }


}

const Status BufMgr::disposePage(File* file, const int pageNo)
{
    // see if it is in the buffer pool
    Status status = OK;
    int frameNo = 0;
    status = hashTable->lookup(file, pageNo, frameNo);
    if (status == OK)
    {
        // clear the page
        bufTable[frameNo].Clear();
    }
    status = hashTable->remove(file, pageNo);

    // deallocate it in the file
    return file->disposePage(pageNo);
}

const Status BufMgr::flushFile(const File* file)
{
  Status status;

  for (int i = 0; i < numBufs; i++) {
    BufDesc* tmpbuf = &(bufTable[i]);
    if (tmpbuf->valid == true && tmpbuf->file == file) {

      if (tmpbuf->pinCnt > 0)
	  return PAGEPINNED;

      if (tmpbuf->dirty == true) {
#ifdef DEBUGBUF
	cout << "flushing page " << tmpbuf->pageNo
             << " from frame " << i << endl;
#endif
	if ((status = tmpbuf->file->writePage(tmpbuf->pageNo,
					      &(bufPool[i]))) != OK)
	  return status;

	tmpbuf->dirty = false;
      }

      hashTable->remove(file,tmpbuf->pageNo);

      tmpbuf->file = NULL;
      tmpbuf->pageNo = -1;
      tmpbuf->valid = false;
    }

    else if (tmpbuf->valid == false && tmpbuf->file == file)
      return BADBUFFER;
  }

  return OK;
}


void BufMgr::printSelf(void)
{
    BufDesc* tmpbuf;

    cout << endl << "Print buffer...\n";
    for (int i=0; i<numBufs; i++) {
        tmpbuf = &(bufTable[i]);
        cout << i << "\t" << (char*)(&bufPool[i])
             << "\tpinCnt: " << tmpbuf->pinCnt;

        if (tmpbuf->valid == true)
            cout << "\tvalid\n";
        cout << endl;
    };
}


