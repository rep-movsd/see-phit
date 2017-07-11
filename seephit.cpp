#include <iostream>
#include "seephit.h"

using namespace std;


void dumpNode(const Nodes &nodes, int index, int indent)
{
  const Node &node = nodes[index];
  string sIndent = string(indent * 2, ' ');
  cerr << sIndent << "<" << node.getTag() << ">";
  if(node.child > -1)
  {
    cerr << endl;
    dumpNode(nodes, node.child, indent + 1);
  }
  else
  {
    cerr << node.getText();
  }
  
  // Skip close tag for void tags
  if(node.getTag().back() != '/')
  {
    cerr << "</" << node.getTag() << ">" << endl;
  }
  
  if(node.sibling > -1)
  {
    dumpNode(nodes, node.sibling, indent);
  }
}

void dumpNodeRaw(const Nodes &nodes, int n)
{
  int i = 0;
  for(int i = 0; i < n; ++i)
  {
    const Node &n = nodes[i];
    cerr << i << endl;
    cerr << "tag:" << n.getTag() << endl;
    cerr << "sibling:" << n.sibling << endl;
    cerr << "child:" << n.child << endl;
    cerr << "text:" << n.getText() << endl;
    cerr << endl;
  }
}


