#include <iostream>

#include "seephit.h"
using namespace std;


int main()
{
  
  constexpr auto nodes = 
  #include "test/valid.spt" 
  
  SPTNode root = SPTNode::from(nodes);
  root.dump(cerr);
  
  //SPT::dumpNode(nodes, 0, 0);

  
}

