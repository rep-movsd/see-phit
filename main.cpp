#include <iostream>
#include "seephit.h"

int main()
{
  constexpr auto nodes = 
  #include "test.spt" 
  
  dumpNode(nodes, 0, 0);
}

