#include <iostream>
#include "seephit.h"

using namespace std;

void dumpNode(const Nodes &nodes, int index, int indent)
{
  const Node &node = nodes[index];
  string sIndent = "\r" + string(indent * 2, ' ');
  cout << sIndent << "<" << node.getTag() << ">";
  if(node.child > -1)
  {
    cout << endl;
    dumpNode(nodes, node.child, indent + 1);
  }
  else
  {
    cout << node.getText();
  }
  cout << sIndent << "</" << node.getTag() << ">" << endl;
  
  if(node.sibling > -1)
  {
    dumpNode(nodes, node.sibling, indent);
  }
}



