#include <iostream>
#include "seephit.h"
using namespace std;

int main()
{
  constexpr auto parser =
    #include "test/valid.spt"
  
  SPTNode root = SPTNode::from(parser);
  root.dump(cerr);
  
}