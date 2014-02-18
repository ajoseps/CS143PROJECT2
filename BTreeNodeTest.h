#include "BTreeNode.h"
#include "PageFile.h"
#include <iostream>
#include <assert.h>

using namespace std;

class BTreeNodeTest{

public:
  void static readLeafNodeTest()
  {
    string fn = "movie.del";
    int pid = 1;

    BTLeafNode* leaf= new BTLeafNode();
    PageFile* pf = new PageFile(fn ,'r');
    leaf->read(pid, *pf);
    
    char* resultBuffer = leaf->getBuffer();
    char* testBuffer = "272,\"Baby Take a Bow\"
      2342,\"Last Ride, The\"
      2634,\"Matter of Life and Death, A\"
      3992,\"Strangers on a Train\"
      2965,\"Notre Dame de Paris\"
      3084,\"Outside the Law\"
      2244,\"King Creole\"
      1578,\"G.I. Blues\"
      3229,\"Plein soleil\"
      4589,\"Wild Ride, The\"
      489,\"Blue Hawaii\"
      4462,\"Voyage to the Bottom of the Sea\"
      1639,\"Girls! Girls! Girls!\"
      1568,\"Fun in Acapulco\"
      3518,\"Roustabout\"
      1109,\"Doctor Zhivago\"
      528,\"Born Free\"
      1390,\"Fantastic Voyage\"
      3297,\"Professionals, The\"
      3561,\"Sand Pebbles, The\"
      3619,\"Seconds\"
      4289,\"Trouble with Angels, The\"
      4583,\"Wild Angels, The\"
      1208,\"Easy Come, Easy Go\"
      598,\"Brotherhood, The\"
      3953,\"Stay Away, Joe\"
      3099,\"Paint Your Wagon\"
      4515,\"Wedding Party, The\"
      4657,\"Wrecking Crew, The\"
      40,\"A.K.A. Cassius Clay\"
      173,\"Angel Levine, The\"
      175,\"Angel Unchained\"
      841,\"Count Yorga, Vampire\"
      897,\"Cry of the Banshee\"
      1088,\"Dirty Dingus Magee\"
      1191,\"Dunwich Horror, The\"
      1236,\"Elvis: Thats the Way It Is\"
      1692,\"Great White Hope, The\"
      1942,\"Husbands\"
      2515,\"Love Story\"
      2619,\"MASH\"
      2648,\"McKenzie Break, The\"
      3546,\"Ryans Daughte\"";

    int diff = strcmp(resultBuffer, testBuffer);
    assert(diff==0);
    cout << "RAN TEST ON PAGE: " << pid << "\n" << endl;
  }
};
