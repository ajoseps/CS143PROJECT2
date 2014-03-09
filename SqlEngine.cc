/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "SqlEngine.h"

// Custom Includes
#include "RecordFile.h"
#include "BTreeIndex.h"

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning

  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  // scan the table file from the beginning
  rid.pid = rid.sid = 0;
  count = 0;
  while (rid < rf.endRid()) {
    // read the tuple
    if ((rc = rf.read(rid, key, value)) < 0) {
      fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
      goto exit_select;
    }

    // check the conditions on the tuple
    for (unsigned i = 0; i < cond.size(); i++) {
      // compute the difference between the tuple value and the condition value
      switch (cond[i].attr) {
      case 1:
	diff = key - atoi(cond[i].value);
	break;
      case 2:
	diff = strcmp(value.c_str(), cond[i].value);
	break;
      }

      // skip the tuple if any condition is not met
      switch (cond[i].comp) {
      case SelCond::EQ:
	if (diff != 0) goto next_tuple;
	break;
      case SelCond::NE:
	if (diff == 0) goto next_tuple;
	break;
      case SelCond::GT:
	if (diff <= 0) goto next_tuple;
	break;
      case SelCond::LT:
	if (diff >= 0) goto next_tuple;
	break;
      case SelCond::GE:
	if (diff < 0) goto next_tuple;
	break;
      case SelCond::LE:
	if (diff > 0) goto next_tuple;
	break;
      }
    }

    // the condition is met for the tuple. 
    // increase matching tuple counter
    count++;

    // print the tuple 
    switch (attr) {
    case 1:  // SELECT key
      fprintf(stdout, "%d\n", key);
      break;
    case 2:  // SELECT value
      fprintf(stdout, "%s\n", value.c_str());
      break;
    case 3:  // SELECT *
      fprintf(stdout, "%d '%s'\n", key, value.c_str());
      break;
    }

    // move to the next tuple
    next_tuple:
    ++rid;
  }

  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
  rc = 0;

  // close the table file and return
  exit_select:
  rf.close();
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  ifstream myLoadFile;
  string tuple, myValue, tableName;
  int myKey;

  // Opens index file if index = true;
  BTreeIndex indexFile;
  if (index)
  {
    string indexName = table + ".idx";

    if (indexFile.open(indexName, 'w'))
    {
      return RC_FILE_OPEN_FAILED;
    }
  }
  
  myLoadFile.open(loadfile.c_str());

  // Checks if file was succesfully opened
  if(myLoadFile.is_open())
  {
    // Opens/Creates the table
    RecordFile myTable; 
    tableName = table + ".tbl";
    if (myTable.open(tableName, 'w')) {
      return RC_FILE_OPEN_FAILED;
    }

    // While the loadfile is not at EOF, it reads each line and parses it
    // Inserting each parsed tuple into the table
    while(!myLoadFile.eof()){
      getline(myLoadFile, tuple);
      
      // Appends only if it parses a tuple correctly
      if(!parseLoadLine(tuple, myKey, myValue))
      {
        RecordId endRid = myTable.endRid();

        myTable.append((int)myKey, myValue, endRid);

        if (index)
        { 
          if(myKey == NULL)
            cout << "ERROR: key is not init" << endl;
          if(endRid.sid == NULL)
            cout << "ERROR: sid is not init" << endl;
          if(endRid.pid == NULL)
            cout << "ERROR: pid is not init" << endl;

          if(endRid.sid < 0)
            cout << "ERROR: invalid sid value: " << endRid.sid << endl;
          if(endRid.pid < 0)
            cout << "ERROR: invalid pid value: " << endRid.pid << endl;
          
          if (indexFile.insert(myKey, endRid))
          {
            cout << "Error: NOT INSERTED INTO INDEX" <<endl;
          }
        }
      }
    }
    myTable.close();
  }
  else
  {
    cout << "Cannot Open File \n" << endl;
    return RC_FILE_OPEN_FAILED;
  }
  myLoadFile.close();
  if (index)
  {
    indexFile.close();
  }
  return 0;
}

RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;
    
    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');
    
    // if there is nothing left, set the value to empty string
    if (c == 0) { 
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}
