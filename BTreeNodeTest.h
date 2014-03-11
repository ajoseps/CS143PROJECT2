#include "BTreeNode.h"
#include "PageFile.h"
#include <iostream>
#include <assert.h>
#include <fstream>

using namespace std;

class BTreeNodeTest{

public:
  void static readLeafNodeTest()
  {
  //   string fn = "movie.del";
  //   int pid = 0;
  //   int pagesize = 1024;
  //   int randIndex = rand() % pagesize-1 + 0;

  //   // Reading from pagefile
  //   BTLeafNode* leaf= new BTLeafNode();
  //   PageFile* pf = new PageFile(fn ,'r');
  //   leaf->read(pid, *pf);
  //   char* resultBuffer = leaf->getBuffer();

  //   // Reading from testfile
  //   ifstream tf (fn.c_str());
  //   char testBuffer[pagesize];
  //   tf.read(testBuffer, pagesize);
  //   tf.close();

  //   // Test Last Byte
  //   assert(testBuffer[pagesize-1] == resultBuffer[pagesize-1]);

  //   // Test First Byte
  //   assert(resultBuffer[0] == testBuffer[0]);

  //   // Test Random Byte
  //   assert(resultBuffer[randIndex] == testBuffer[randIndex]);

  //   for (int i = 0; i < pagesize-1; i++)
  //   {
  //       cout << "buffer: " << testBuffer[i] << endl;
  //   }

  //   cout << "RAN read() TEST ON PAGE: " << pid << "\n" << endl;
  // }

  // void static getKeyCountLeafNodeTest()
  // {
  //   string fn = "movie.del";
  //   int pid = 0;
  //   int pagesize = 1024;
  //   int randIndex = rand() % pagesize-1 + 0;

  //   // Reading from pagefile
  //   BTLeafNode* leaf= new BTLeafNode();
  //   PageFile* pf = new PageFile(fn ,'r');
  //   leaf->read(pid, *pf);
  //   int keyCount = leaf->BTLeafNode::getKeyCount();

  //   // There are 42 keys in page 0
  //   // assert(keyCount == 42);

  //   cout << "RAN keyCount() TEST ON PAGE: " << pid << "\n" << endl;
  // }
};
