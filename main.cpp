#include <iostream>
#include "seephit.h"
using namespace std;

int main()
{
  constexpr auto parser =
    #include "test/valid.spt"
  
  //SPTDumper::dumpNode(parser.nodes);
  spt::tree spt_tree(parser);
  spt_tree.root.dump(cerr);
}