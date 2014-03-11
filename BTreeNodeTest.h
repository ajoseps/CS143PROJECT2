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
    string filename= "movie.del";
    int pid = 0;
    int pagesize = 1024;
    int randIndex = rand() % pagesize-1 + 0;

    // Reading from pagefile
    // BTLeafNode* leaf= new BTLeafNode();
    // PageFile* pf = new PageFile(filename,'r');
    // leaf->read(pid, *pf);
    // char* resultBuffer = leaf->getBuffer();
    BTLeafNode leaf;
    PageFile pf (filename, 'r');
    leaf.read(pid, pf);
    char* resultBuffer = leaf.getBuffer();


    // Reading from testfile
    ifstream testfile (filename.c_str());
    char testBuffer[pagesize];
    testfile.read(testBuffer, pagesize);
    testfile.close();

    // Test Last Byte
    assert(testBuffer[pagesize-1] == resultBuffer[pagesize-1]);

    // Test First Byte
    assert(resultBuffer[0] == testBuffer[0]);

    // Test Random Byte
    assert(resultBuffer[randIndex] == testBuffer[randIndex]);

    for (int i = 0; i <= pagesize-1; i++)
    {
        cout << "buffer[" << i << "]: " << resultBuffer[i] << endl;
    }
    cout << "BUFFER INDEX: " << leaf.getBufferIndex() << endl;
    cout << "RAN read() TEST ON PAGE: " << pid << "\n" << endl;
  }

  void static getKeyCountLeafNodeTest()
  {
    string filename= "movie.del";
    int pid = 0;
    int pagesize = 1024;
    int randIndex = rand() % pagesize-1 + 0;

    // Reading from pagefile
    BTLeafNode* leaf= new BTLeafNode();
    PageFile* pf = new PageFile(filename,'r');
    leaf->read(pid, *pf);
    int keyCount = leaf->BTLeafNode::getKeyCount();

    // There are 42 keys in page 0
    // assert(keyCount == 42);

    cout << "RAN keyCount() TEST ON PAGE: " << pid << "\n" << endl;
  }
};
