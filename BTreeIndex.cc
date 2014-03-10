/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include "BTreeNode.h"
#include <iostream>

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
    //PageFile pf(indexname, mode);
    
    if (pf.open (indexname, mode) !=0) {
    	return 1; 
    }
    
    char buffer[PageFile::PAGE_SIZE];
    memset(buffer, 0, PageFile::PAGE_SIZE);

    //if file is empty, initialize rootPid & treeHeight
    if (pf.endPid()==0)
    {
        rootPid = -1;
        treeHeight = 0;
    }
    else {
        if (pf.read(0, buffer) != 0)
        {
            return 1;
        }
        rootPid = (PageId)buffer[0];
        treeHeight = (int)(buffer[sizeof(PageId)]);
    }
    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    char buffer[PageFile::PAGE_SIZE];
    memset(buffer, 0, PageFile::PAGE_SIZE);
    buffer[0] = (PageId)rootPid;
    buffer[sizeof(PageId)] = (int)treeHeight;

    if (pf.write(0, buffer) != 0 ) {
        return RC_FILE_WRITE_FAILED;
    }
    return pf.close();
}


/*
* IN: key passed in from insert
* IN: rid passed in from insert
* OUT: overflow -- whether the last insert caused an overflow
* IN: height of the node that we're currently looking at
* IN: pid that we're currently looking at
* OUT: siblingPid -- new pid of the sibling after splitting
* OUT: siblingKey -- equivalent to midKey in insertAndSplit, needed to insert into parent node
*/

RC BTreeIndex::insertHelp (int key, const RecordId& rid, bool& overflow, int height, PageId pid, PageId& siblingPid, int& siblingKey) {

    overflow = false;
    if (height == treeHeight) //leaf node, base case.
    {
        BTLeafNode leaf;
        leaf.read(pid, pf);
        if (leaf.insert(key, rid) == RC_NODE_FULL)
        {
            overflow = true;
            BTLeafNode siblingLeaf;
            if (leaf.insertAndSplit(key, rid, siblingLeaf, siblingKey))
            {
                return 1; //ERROR IN SPLIT
            }
            siblingPid = pf.endPid();
            siblingLeaf.setNextNodePtr(leaf.getNextNodePtr());
            leaf.setNextNodePtr(siblingPid);

            if (siblingLeaf.write(siblingPid, pf))
            {
                return RC_FILE_WRITE_FAILED;
            }
            if (leaf.write(pid, pf))
            {
                return RC_FILE_WRITE_FAILED;
            }
        }
    }

    else { //nonleaf node
        BTNonLeafNode nonleaf;
        PageId childPid;

        nonleaf.read(pid, pf);
        nonleaf.locateChildPtr(key, childPid);
        insertHelp(key, rid, overflow, height+1, childPid, siblingPid, siblingKey);

        if (overflow)
        {
            if (nonleaf.insert(siblingKey, siblingPid)) //nonleaf full
            {

                BTNonLeafNode siblingNonLeaf;
                int midKey;                

                if (siblingNonLeaf.insertAndSplit(siblingKey, siblingPid, siblingNonLeaf, midKey))
                {
                    return 1; //ERROR IN SPLIT. TOO BAD.
                }
                siblingKey = midKey;
                siblingPid = pf.endPid();
                if (siblingNonLeaf.write(siblingPid, pf))
                {
                    return RC_FILE_WRITE_FAILED;
                }
            }
            else {
                overflow = false;
            }
            nonleaf.write(pid, pf);
        }
    }
    return 0;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
    if (treeHeight == 0) //initialize root node
    {
        BTLeafNode leaf;
        leaf.insert(key, rid);
        rootPid = pf.endPid();
        treeHeight = 1;
        leaf.write(rootPid, pf);
        return 0;
    }

    bool overflow = false;
    PageId siblingPid = -1;
    int siblingKey = -1;

    if (insertHelp(key, rid, overflow, 1, rootPid, siblingPid, siblingKey))
    {
        return 1; // ERROR SOMEHWERE IN INSERT TOO BAD SO SAD.
    }

    //new root node cus of overflow
    if (overflow)
    {
        BTNonLeafNode root;
        rootPid = pf.endPid();
        root.initializeRoot(rootPid, siblingKey, siblingPid);
        treeHeight++;
        if (root.write (rootPid, pf)) {
            return RC_FILE_WRITE_FAILED;
        }
    }
    return 0;
}

/*
 * Find the leaf-node index entry whose key value is larger than or 
 * equal to searchKey, and output the location of the entry in IndexCursor.
 * IndexCursor is a "pointer" to a B+tree leaf-node entry consisting of
 * the PageId of the node and the SlotID of the index entry.
 * Note that, for range queries, we need to scan the B+tree leaf nodes.
 * For example, if the query is "key > 1000", we should scan the leaf
 * nodes starting with the key value 1000. For this reason,
 * it is better to return the location of the leaf node entry 
 * for a given searchKey, instead of returning the RecordId
 * associated with the searchKey directly.
 * Once the location of the index entry is identified and returned 
 * from this function, you should call readForward() to retrieve the
 * actual (key, rid) pair from the index.
 * @param key[IN] the key to find.
 * @param cursor[OUT] the cursor pointing to the first index entry
 *                    with the key value.
 * @return error code. 0 if no error.
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor) //might need to re-do
{
    PageId childPid = -1;
    PageId pid = rootPid;
    int currHeight = 0;
    while (currHeight < treeHeight-1) {
        BTNonLeafNode nonleaf;
        nonleaf.read(pid, pf);
        nonleaf.locateChildPtr(searchKey, childPid);
        currHeight++;
    }

    BTLeafNode leaf;
    if (childPid < -1)
    {
        leaf.read(childPid, pf);
        cursor.pid = childPid;
        leaf.locate(searchKey, cursor.eid);
        return 0;
    }
    else
        return 1;


    // int leafPid;
    // int currHeight = 1;
    // while(currHeight < treeHeight) // stops when at leafnode
    // {
    //     cout << "BTreeIndex:: locate -- in while loop" << endl;
    //     BTNonLeafNode nonleaf;
    //     nonleaf.read(rootPid, pf);
    //     nonleaf.locateChildPtr(searchKey, leafPid); // gets the pid of the next node
    //     currHeight++;
    // }

    // cout<<"BTreeIndex::locate leafPid: "<<leafPid<<endl;

    // BTLeafNode leaf;
    // leaf.read(leafPid, pf);

    // // sets the cursor's values
    // leaf.locate(searchKey, cursor.eid);
    // cursor.pid = leafPid;
    // return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
    // Read in the page's buffer, into a local buffer
    // cout<< "BTreeIndex:: readForward -- cursor.eid: " << cursor.eid << endl << "pid: " <<cursor.pid <<endl;
    PageId searchPid = -1;
    if (cursor.pid < -1)
    {
        searchPid = cursor.pid;
    }
    // might need to check if leafnode
    BTLeafNode node;
    if (searchPid < -1 && searchPid >= pf.endPid())
    {
        node.read(searchPid, pf);
    }

    int entryId = cursor.eid;

    // iterating the cursor
    // if it is the last eid, it will go to the next page in the pagefile
    if(node.getKeyCount() <= entryId){ 
        cursor.pid = node.getNextNodePtr();
        cursor.eid = 0;
    }
    else{
        cursor.eid++;
    }

    // key and rid are updated with the corresponding inputted entryId
    // returns a RC
    return node.readEntry(entryId, key, rid);
}
