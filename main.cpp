#include <iostream>
#include "seephit.h"
using namespace std;



int main()
{
  constexpr spt::parser parser =
    #include "test/large.spt"
  
  //parser.dump();
  
  //SPTDumper::dumpNode(parser.nodes);
  spt::tree spt_tree(parser);
  
  spt::template_dict dct;
  dct["name"] = "Mary";
  dct["profession"] = "doctor";
  dct["city"] = "London";
  
  spt_tree.root.dump(cerr, dct);
  cerr << endl;
  
  dct["city"] = "New York";
  dct["name"] = "John";
  dct["profession"] = "janitor";

  spt_tree.root.dump(cerr, dct);
  cerr << endl;
  
  
}