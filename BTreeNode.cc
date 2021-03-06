#include "BTreeNode.h"
#include <iostream>

using namespace std;

/*
 * Returns a pointer to the BTLeafNode's buffer 
 */
char* BTLeafNode::getBuffer()
{
  return buffer;
}

int BTLeafNode::getBufferIndex()
{
  return buffer_index;
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ 
  // cout << "BTLeafNode :: read -- in read function" << endl;
  pf.read(pid, buffer);
  for (int i = 0; i < PageFile::PAGE_SIZE; ++i)
  {
    if (buffer[i] != -1)
    {
      buffer_index++;
      // cout << "BTLeafNode :: read -- buffer_index: " << buffer_index << endl;
    }
    else {
      keyCount = buffer_index / 12;
      printf("buffer: %s", buffer);
      return 0;
    }
  }
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{ 
  return pf.write(pid, buffer);
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
  cout << "BTLeafNode :: insert -- key: " << key << endl;
  int potentiallyUsedBuffer = buffer_index + sizeof(int) + sizeof(RecordId);
  if(potentiallyUsedBuffer < PageFile::PAGE_SIZE){
    if (buffer_index == 0) //when buffer is empty
    {
      cout << "BTLeafNode :: insert -- in if block cus buffer_index == 0" << endl << endl;
      insertKey(key, 0);
      insertRid(rid, sizeof(int));

    }
    else {
      // for (int i = sizeof(RecordId); i < buffer_index; i = i + sizeof(RecordId) + sizeof(int)) // when buffer isn't empty
      // {
      //   if (key <= buffer[i])
      //   {
      //     insertIndex = i;
      //     memcpy ((char*)(buffer + insertIndex + sizeof(RecordId) + sizeof(int)), (buffer + insertIndex), potentiallyUsedBuffer-insertIndex+1);
      //     insertKey(key, insertIndex);
      //     insertRid(rid, insertIndex + sizeof(int));
      //     break;
      //   }
      //   else {
      //     insertKey(key, buffer_index);
      //     insertRid(rid, buffer_index + sizeof(int));

      //   }
      // }
      int eid = -1;
      locate (key, eid);
      if (eid == -1)
      {
        insertKey(key, buffer_index);
        insertRid(rid, buffer_index + sizeof(int));
      }
      else {
        cout << "BTLeafNode :: insert -- in else statment / about to move buffer" << endl;
        insertIndex = eid * 12;
        memcpy ((char*)(buffer + insertIndex + sizeof(RecordId) + sizeof(int)), &(buffer[insertIndex]), potentiallyUsedBuffer-insertIndex);
        insertKey(key, insertIndex);
        insertRid(rid, insertIndex + sizeof(int));
        cout << "BTLeafNode :: insert -- buffer_index: " << buffer_index << endl;
      }
    }
    keyCount++;
    buffer_index = buffer_index + sizeof(int) + sizeof(RecordId);
    // cout << "BTLeafNode :: insert -- keyCount: " << getKeyCount() << endl;
    cout << "BTLeafNode :: insert -- buffer_index: " << buffer_index << endl;
      return 0;
  }
  else
    return RC_NODE_FULL; // ERROR
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
{
  cout << "BTLeafNode :: insertAndSplit called" <<endl;
  if (insert(key, rid) == 0 && split(sibling, siblingKey)) //if insert succeeds, then just split node
  {
      return 0;
  }
  else { //if insert fails due to lack of space, split then insert
    for (int i = sizeof(RecordId); i < buffer_index; i = i + sizeof(RecordId) + sizeof(int)) {
      if (key <= buffer[i]) //PROBLEM: comparing int to one char? do we need to include length?
      {
        insertIndex = i; //where to insert
      }
    }
    int halfKeyCount = (int)floor(keyCount/2);
    int splitIndex = sizeof(RecordId) * halfKeyCount + sizeof(int) * halfKeyCount;

    split(sibling, siblingKey);
    if (insertIndex <= splitIndex) //insert into original node after split
    {
      insert(key, rid);
      return 0;
    }
    else if (insertIndex > splitIndex) { //insert into new node
      sibling.insert(key, rid);
      return 0;
    }
    else {
      return 1; //ERROR
    }
  }
}

bool BTLeafNode::split (BTLeafNode& sibling, int& siblingKey) {
  if (sibling.buffer_index != 0)
  {
    return false; //ERROR: sibling node is not empty
  }
  else {
    int halfKeyCount = (int)floor(keyCount/2);
    int splitIndex = sizeof(RecordId) * halfKeyCount + sizeof(int) * halfKeyCount;
    
    memcpy ((char*)sibling.buffer, (buffer + splitIndex), buffer_index - splitIndex);
    //set sibling's buffer_index
    sibling.buffer_index = buffer_index - splitIndex;
    // PageId siblingPid = sibling.buffer[0]; //WRONG: sibling.buffer[0] is a key, not a PageId
    PageId siblingPid = *(PageId *)(sibling.buffer + sizeof(int));
    insertPid(siblingPid, splitIndex);
    // buffer_index += sizeof(PageId);
    buffer_index = splitIndex + sizeof(PageId); 
    siblingKey = *(int *)(sibling.buffer);
    return true;
  }
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{
  eid = 0;
  cout << "BTLeafNode:: locate -- buffer_index: " << buffer_index <<endl;
  for (int i = 0; i < buffer_index-sizeof(RecordId); i = i + sizeof(RecordId) + sizeof(int))
  {
    cout << "BTLeafNode :: locate -- value check: " << *((int *) (buffer + i)) << endl;
    cout << "BTLeafNode :: locate -- searchKey: " << searchKey << endl;
    if (searchKey > *((int *) (buffer + i)))
    {
      cout << "BTLeafNode :: locate -- IN IF BLOCK" <<endl;
      eid++;
    }
    else {
      break;
    }
  }
  cout << "BTLeafNode :: locate -- eid: " << eid <<endl;
  cout << "BTLeafNode :: locate -- keyCount: " << keyCount <<endl;
  if (eid == getKeyCount())
  {
    cout << "BTLeafNode :: locate -- didn't find eid" << endl;
    eid = -1;
    return RC_NO_SUCH_RECORD;
  }
  return 0;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{
  // cout << "buffer index: " << buffer_index <<endl;
  // cout<<"EID: " <<eid<<endl<<endl;
  if (buffer_index == 0)
  {
    return RC_NO_SUCH_RECORD;
  }

  else if (eid <= getKeyCount())
  {
    // cout<< "EID IN IF: " << eid << endl << "check cond: "<< 0-4-8 << endl;
    key = *((int *) (buffer + eid));
    rid.pid = *((PageId *) (buffer + eid+sizeof(int))); //struct rid = pid and sid WHICH ONE FIRST?????
    rid.sid = *((int *) (buffer + eid + sizeof(PageId) + sizeof(int)));
    return 0;
  }
  
    return RC_NO_SUCH_RECORD; //eid doesn't exist in buffer
  
}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{
  return *((PageId *)(buffer + buffer_index-sizeof(PageId)));
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
  memcpy((char*)(buffer + buffer_index-sizeof(PageId)), &pid, sizeof(PageId));
  return 0;
}

/* 
 * Inserts a key into the buffer
 * Returns True if inserts correctly
 * Returns False if node is full
 */
bool BTLeafNode::insertKey(int key, int insertIndex){
    if(buffer_index + sizeof(int) >= PageFile::PAGE_SIZE) {
      // cout << "BTLeafNode :: insertKey -- buffer_index: " << buffer_index << endl;
      // cout << "BTLeafNode :: insertKey -- about to return false" << endl;
      return false;
    }
    memcpy((char*) (buffer + insertIndex), &key, sizeof(int));
    cout << "BTLeafNode :: insertKey: " << key << endl;
    cout << "BTLeafNode :: insertKey -- insertIndex: " << insertIndex << endl;
    // buffer_index = buffer_index + sizeof(int);
    // keyCount++;
    // cout << "BTLeafNode :: insertKey -- buffer_index: " << buffer_index << endl;
    // cout << "BTLeafNode :: insertKey -- keyCount: " << getKeyCount() << endl;
    return true;
}

/* 
 * Inserts a PageId into the buffer
 * Returns True if inserts correctly
 * Returns False if node is full
 */
bool BTLeafNode::insertPid(PageId pid, int insertIndex){
  if(buffer_index + sizeof(PageId) >= PageFile::PAGE_SIZE)
      return false;
  memcpy((char*)(buffer + insertIndex), &pid, sizeof(PageId));
  // buffer_index = buffer_index + sizeof(PageId);
  return true;
}

/* 
 * Inserts a RId into the buffer
 * Returns True if inserts correctly
 * Returns False if node is full
 */
bool BTLeafNode::insertRid(const RecordId& rid, int insertIndex)
{
  if(buffer_index + sizeof(PageId) + sizeof(int) >= PageFile::PAGE_SIZE)
      return false;
  memcpy((char*)(buffer + insertIndex), &rid.pid, sizeof(PageId));
  // buffer_index = buffer_index + sizeof(PageId);
  memcpy((char*)(buffer + insertIndex + sizeof(PageId)), &rid.sid, sizeof(int));
  // buffer_index = buffer_index + sizeof(int);
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
  // cout << "BTNonLeafNode :: read -- in read function" << endl;
  pf.read(pid, buffer);
  for (int i = 0; i < PageFile::PAGE_SIZE; ++i)
  {
    if (buffer[i] != -1)
    {
      buffer_index++;
    }
    else {
      keyCount = buffer_index/8;
      printf("buffer: %s", buffer);
      return 0;
    }
  }
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ 
  // if(!pf.write(pid, buffer))
  //   return 0;
  // else
  //   return RC_FILE_WRITE_FAILED;
    return pf.write(pid, buffer);
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
  int potentiallyUsedBuffer = buffer_index + sizeof(int) + sizeof(PageId);
  if(potentiallyUsedBuffer < PageFile::PAGE_SIZE){
    for (int i = sizeof(PageId); i < buffer_index; i = i + sizeof(PageId) + sizeof(int))
    {
      if (key <= buffer[i]) //PROBLEM: comparing int to one char? do we need to include length?
      {
        insertIndex = i;
        memcpy ((char*)buffer[insertIndex + sizeof(PageId) + sizeof(int)], &buffer[insertIndex], potentiallyUsedBuffer-insertIndex+1);
        insertKey(key, insertIndex);
        insertPid(pid, insertIndex + sizeof(int));
//        return 0;
      }
      else { //insert at the end because it's bigger than everything else
        insertKey(key, buffer_index);
        insertPid(pid, buffer_index + sizeof(int));
//        return 0;
      }
    }
    buffer_index = buffer_index + sizeof(PageId) + sizeof(int);
    keyCount++;
    return 0;
  }
  else
    return RC_NODE_FULL; // ERROR
  
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
{ 
  if (insert(key, pid) == 0 && split(sibling, midKey)) //if insert succeeds, then just split node
  {
      return 0;
  }
  else { //if insert fails due to lack of space, split then insert
    for (int i = sizeof(PageId); i < buffer_index; i = i + sizeof(PageId) + sizeof(int)) {
      if (key <= buffer[i]) //PROBLEM: comparing int to one char? do we need to include length?
      {
        insertIndex = i; //where to insert
      }
    }
    int halfKeyCount = (int)floor(keyCount/2);
    int splitIndex = sizeof(PageId) * halfKeyCount + sizeof(int) * halfKeyCount;

    split(sibling, midKey);
    if (insertIndex <= splitIndex) //insert into original node after split
    {
      insert(key, pid);
      return 0;
    }
    else if (insertIndex > splitIndex) { //insert into new node
      sibling.insert(key, pid);
      return 0;
    }
    else {
      return 1; //ERROR
    }
  }
}

bool BTNonLeafNode::split(BTNonLeafNode& sibling, int& midKey)
{
    if (sibling.buffer_index != 0)
    {
      return false; //ERROR: sibling node is not empty
    }
    else {
      int halfKeyCount = (int)floor(keyCount/2);
      int splitIndex = sizeof(PageId) * halfKeyCount + sizeof(int) * halfKeyCount;
      
      memcpy ((char*)sibling.buffer, (buffer + splitIndex), buffer_index - splitIndex + 1);
      //set sibling's buffer_index
      sibling.buffer_index = buffer_index - splitIndex;
      buffer_index = splitIndex + sizeof(PageId); 
      memcpy((int*)&midKey, (sibling.buffer + sizeof(PageId)), sizeof(PageId));
      //midKey = sibling.buffer[ sizeof(PageId) ];
      return true;
    }
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{
  int key_index = sizeof(PageId); // index is at the first key in the node
  while((int) buffer[key_index] != searchKey) 
  {
    key_index += sizeof(int) + sizeof(PageId);
    if(key_index >= PageFile::PAGE_SIZE)
      return RC_NO_SUCH_RECORD; // searchKey not found
  }

  // key_index will point to last (key, pid) pair
  int pid_index = key_index + sizeof(int);
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
  if(insertPid(pid1, 0) && insert(key, pid2) == 0){
    buffer_index = buffer_index + sizeof(PageId);
    return 0; 
  }
  else
    return 1; // ERROR
}

/* 
 * Inserts a key into the buffer
 * Returns True if inserts correctly
 * Returns False if node is full
 */
  bool BTNonLeafNode::insertKey(int key, int insertIndex){
      if(buffer_index + sizeof(int) >= PageFile::PAGE_SIZE)
        return false;
      memcpy((char*) (buffer + insertIndex), &key, sizeof(int));
      // buffer_index+=sizeof(int);
      // keyCount++;
      return true;
  }

/* 
 * Inserts a PageId into the buffer
 * Returns True if inserts correctly
 * Returns False if node is full
 */
  bool BTNonLeafNode::insertPid(PageId pid, int insertIndex){
    if(buffer_index + sizeof(PageId) >= PageFile::PAGE_SIZE)
        return false;
    memcpy((char*) (buffer + insertIndex), &pid, sizeof(PageId));
    // buffer_index+=sizeof(PageId);
    return true;
  }

