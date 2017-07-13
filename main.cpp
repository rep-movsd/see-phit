#include <iostream>
#include "seephit.h"
using namespace std;

int main()
{
  constexpr auto nodes =
  #include "test/valid.spt"
  
  dumpNode(nodes, 0, 0);
}