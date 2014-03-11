/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "Bruinbase.h"
#include "SqlEngine.h"
// #include "BTreeNodeTest.h"
 #include "BTreeNode.h"
 #include <iostream>
 using namespace std;


int main()
{
  // run the SQL engine taking user commands from standard input (console).

  // Tester
  
  // BTreeNodeTest* tester = new BTreeNodeTest();
  // tester->readLeafNodeTest();
  // tester->getKeyCountLeafNodeTest();
  /*
  BTLeafNode leaf;
  PageFile pf;
  char buffer[PageFile::PAGE_SIZE];
  int buffer_index = 0;
  memset(buffer,-1,PageFile::PAGE_SIZE);
  pf.open('movie.del', 'r');
  for (int i = 0; i < PageFile::PAGE_SIZE; i++)
  {
  	if (buffer[i] != -1)
  	{
  		buffer_index++;
  	}
  	else {
  		break;
  	}
  }
  for (int j = 0; j < buffer_index; j++)
  {
  	cout << buffer[j] <<endl;
  }*/
  SqlEngine::run(stdin);

  return 0;
}
