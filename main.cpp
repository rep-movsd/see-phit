#include <iostream>
#include "seephit.h"

int main()
{
  constexpr auto nodes = 
  #include "test.spt" 
  
  dumpNode(nodes, 0, 0);

  #define TEST(X)   cout << X << "->" << findTag(X) << endl;
  
  TEST("aaaa");
  TEST("a");
  TEST("div");
  TEST("xmp");
  TEST("xxxx");
  
}

