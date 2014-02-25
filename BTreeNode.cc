#include "BTreeNode.h"

using namespace std;


/*
 * Returns a pointer to the BTLeafNode's buffer 
 */
char* BTLeafNode::getBuffer()
{
  return buffer;
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ 
  if(!pf.read(pid, buffer))
    return 0; 
  else
    return 1;
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{ 
  if(!pf.write(pid, buffer))
    return 0;
  else
    return 1;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
  return keyCount;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
  if(buffer_index + sizeof(key) + sizeof(RecordId) < PageFile::PAGE_SIZE){
    if(insertKey(key) && insertRid(rid))
      return 0; 
    else
      return 1; // ERROR
  }
  else
    return 1; // ERROR
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{ return 0; }

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ return 0; }

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{ return 0; }

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{
  return (PageId)buffer[buffer_index-sizeof(PageId)];
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
  memcpy((char*)buffer[buffer_index-sizeof(PageId)], &pid, sizeof(PageId));
  return 0;
}

/* 
 * Inserts a key into the buffer
 * Returns True if inserts correctly
 * Returns False if node is full
 */
bool BTLeafNode::insertKey(int key){
    if(buffer_index + sizeof(int) >= PageFile::PAGE_SIZE)
      return false;
    memcpy((char*)buffer[buffer_index], &key, sizeof(int));
    buffer_index+=sizeof(int);
    keyCount++;
    return true;
}

/* 
 * Inserts a PageId into the buffer
 * Returns True if inserts correctly
 * Returns False if node is full
 */
bool BTLeafNode::insertPid(PageId pid){
  if(buffer_index + sizeof(PageId) >= PageFile::PAGE_SIZE)
      return false;
  memcpy((char*)buffer[buffer_index], &pid, sizeof(PageId));
  buffer_index+=sizeof(PageId);
  return true;
}

/* 
 * Inserts a RId into the buffer
 * Returns True if inserts correctly
 * Returns False if node is full
 */
bool BTLeafNode::insertRid(const RecordId& rid)
{
  if(buffer_index + sizeof(PageId) + sizeof(int) >= PageFile::PAGE_SIZE)
      return false;
  memcpy((char*)buffer[buffer_index], &rid.pid, sizeof(PageId));
  buffer_index+=sizeof(PageId);
  memcpy((char*)buffer[buffer_index], &rid.sid, sizeof(int));
  buffer_index+=sizeof(int);
  return true;
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ 
  if(!pf.read(pid, buffer))
    return 0; 
  else
    return 1;
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ 
  if(!pf.write(pid, buffer))
    return 0;
  else
    return 1;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
  return keyCount; 
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 
  if(buffer_index + sizeof(int) + sizeof(PageId) < PageFile::PAGE_SIZE){
    if(insertKey(key) && insertPid(pid))
      return 0; 
    else
      return 1; // ERROR
  }
  else
    return 1; // ERROR
  
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{ return 0; }

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{
  int key_index = sizeof(PageId);
  while(buffer[key_index] != searchKey)
  {
    key_index += 2*sizeof(int);
    if(key_index >= PageFile::PAGE_SIZE)
      return 1;
  }

  int pid_index = key_index - sizeof(int);
  pid = buffer[pid_index];
  return 0;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ 
  // !insert because insert returns 0 if it returns succesfully
  if(insertPid(pid1) && !insert(key, pid2))
    return 0; 
  else
    return 1; // ERROR
}

/* 
 * Inserts a key into the buffer
 * Returns True if inserts correctly
 * Returns False if node is full
 */
  bool BTNonLeafNode::insertKey(int key){
      if(buffer_index + sizeof(int) >= PageFile::PAGE_SIZE)
        return false;
      memcpy((char*)buffer[buffer_index], &key, sizeof(int));
      buffer_index+=sizeof(int);
      keyCount++;
      return true;
  }

/* 
 * Inserts a PageId into the buffer
 * Returns True if inserts correctly
 * Returns False if node is full
 */
  bool BTNonLeafNode::insertPid(PageId pid){
    if(buffer_index + sizeof(PageId) >= PageFile::PAGE_SIZE)
        return false;
    memcpy((char*)buffer[buffer_index], &pid, sizeof(PageId));
    buffer_index+=sizeof(PageId);
    return true;
  }

