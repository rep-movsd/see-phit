#pragma once
#include <cassert>
#include <cstdint>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <utility>
#include <iostream>
#include "tags.hpp"

using namespace std;

// Allow runtime debugging for development
#ifdef SPT_DEBUG

#define constexpr 
#define DUMP cerr
#define ENDL "\n"

// Dummy function to prettify compile-time errors
void ParseError(const char* err) {cerr << "Parse Error:" << err << endl;}
#define PARSE_ERR(x) ParseError(x)

#else

#define DUMP DummyOut
#define ENDL 0

struct DummyOutStream
{
  template<typename T>  constexpr const DummyOutStream& operator <<(const T &) const { return *this;}
};

constexpr DummyOutStream DummyOut;

struct ErrLine
{
  int dummy[0];
  constexpr int &operator[](size_t i) { return dummy[i];};
};

// Dummy function to prettify compile-time errors
constexpr ErrLine ParseError(const char* err)
{
  return ErrLine{};
}

#define PARSE_ERR(x) \
int iLine = curLine(); \
ParseError(x)[iLine] = 0

#endif

constexpr char toUpper(char ch)
{
  if(ch >= 'a' && ch <= 'z')
  {
    return ch + 'A' - 'a';
  }
  return ch;
}

constexpr bool isSpace(char ch)
{
  return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t';
}

// Represents a symbol - kind of like a string_view on a const char*
struct Symbol
{
  const char *pBeg;
  const char *pEnd;
  
  constexpr Symbol(): pBeg(), pEnd() {}
  constexpr Symbol(const char *pBeg, const char *pEnd): pBeg(pBeg), pEnd(pEnd) {}
  constexpr Symbol(const char *p): pBeg(p), pEnd(p)
  {
    while(*pEnd) pEnd++;
  }
  
  constexpr size_t size() const       { return pEnd - pBeg; }
  constexpr const char *begin() const { return pBeg; }
  constexpr const char *end() const   { return pEnd; }
  constexpr bool empty() const        { return size() == 0; }
  
  constexpr bool operator==(const Symbol &that) const
  {
    if(size() == that.size())
    {
      auto p1 = pBeg, p2 = that.pBeg;
      while(p1 != pEnd)
      {
        if(toUpper(*p1) != toUpper(*p2)) return false;
        ++p1;
        ++p2;
      }
      
      return true;
    }
    return false;
  }
  
  constexpr bool operator!=(const Symbol &that) const
  {
    return !(*this == that);
  }
  
  // Simplistic hash (not meant to be perfect)
  constexpr uint16_t hash()
  {
    uint16_t ret = 5381;
    
    for(auto ch: *this)
    {
      ret = ((ret << 5) + ret) + ch; 
    }
    return ret;
  }
  
  // Compare sym1 and sym2
  constexpr int cmpCaseLess(const Symbol &that) const
  {
    if(size() < that.size()) return -1;
    if(size() > that.size()) return 1;
    
    for(int i = 0; i < size(); ++i)
    {
      char a = toUpper(pBeg[i]);
      char b = toUpper(that.pBeg[i]);
      if(a < b) return -1;
      if(a > b) return 1;
    }
    
    return 0;
  }
  
  // Compare s1 and s2 with case sensitivity
  constexpr int cmpCase(const Symbol &that) const
  {
    if(size() < that.size()) return -1;
    if(size() > that.size()) return 1;
    
    for(int i = 0; i < size(); ++i)
    {
      char a = pBeg[i];
      char b = that.pBeg[i];
      if(a < b) return -1;
      if(a > b) return 1;
    }
    return 0;
  }

  string getText() const { return string(pBeg, pEnd); }
};

template<typename T, int SIZE>
struct Array
{
  T nodes[SIZE];
  constexpr T& operator[](size_t i) { return nodes[i]; }
  constexpr const T& operator[](size_t i) const { return nodes[i]; }
  size_t size = 0;
  const size_t capacity = SIZE;
  
  constexpr T* begin() { return nodes; }
  constexpr T* end() { return nodes + size; }
  
  constexpr T& back() 
  { 
    if(!size)
    {
      assert("Array is 0 sized");
    }
    return nodes[size - 1]; 
  }

  constexpr int curr() { return size - 1;}
  
  constexpr bool add() 
  { 
    ++size;
    return (size-1) < capacity;
  }
};

// Simple abstraction for a symbol table
struct SymTable
{
  Array<Symbol, 1024> syms;
  
  // Adds a symbol to the table, returns false if already exists
  constexpr bool addSym(const Symbol &symNew)
  {
    for(const auto &sym: syms)
    {
      if(symNew == sym) return false;
    }
    syms.add();
    syms.back() = symNew;
    return true;
  }
};


struct Attr 
{
  int iNode;
  Symbol name;      
  Symbol value;  
  
  constexpr Attr():iNode(-1) {}
  
  constexpr Attr(int iNode, const Symbol &name, const Symbol &value):
  iNode(iNode), name(name), value(value) {}
  
  constexpr Attr(const Attr &a):iNode(a.iNode), name(a.name), value(a.value){}
};

struct Node
{
  /*
   * 
   * Represents a multiway tree 
   * 
   * Each node has two pointers 
   * sibling -> 
   * and 
   * child |
   * 
   * Two symbols, tag and text represent the tagname and the text content if any
   * 
   * For example
   * <HTML>
   *  <DIV>
   *  </DIV>
   *  <DIV>
   *    <P>Hello</P>
   *  </DIV>
   *  <DIV>
   *  </DIV>
   * </HTML>
   * 
   * 
   * HTML-> -1
   * |
   * DIV -> DIV ->  DIV -> -1
   * |      |       |
   * NULL   P->-1   -1
   * 
   * Since this is meant to be a compile time DS, pointers sibling and child are just ints
   * They index into an array of fixed size
   * 
   * Attributes are nested as children under <_ATTR>
   * Styles are nested under <_ATTR><_STYLE></_STYLE></_ATTR>
   * 
   */
  
  constexpr Node():sibling(-1), child(-1), tag(), text(), id() {}
  constexpr Node(const Node &n):
  sibling(n.sibling), child(n.child), tag(n.tag), text(n.text), id(n.id) {}
  
  // returns the tag in uppercase
  const string getTag() const 
  {
    string ret(tag.pBeg, tag.size());
    transform(begin(ret), end(ret), begin(ret), toUpper);
    return ret;
  };
  
    
  // Returns the text content, optionally stripping leading and trailing space
  const string getText(bool trim = false) const 
  {
    auto b = text.pBeg, e = text.pEnd;
    if(trim && b != e)
    {
      while(isSpace(*b)) b++;
      while(e >= b && isSpace(e[-1])) e--;
    }
    
    return string(b, e);
  };
  
  int sibling;
  int child;
  Symbol tag;
  Symbol text;
  Symbol id;
};

typedef Array<Attr, 256> Attrs;
typedef Array<Node, 1024> Nodes;

struct SPT 
{
  static void dumpNode(const Nodes &nodes, int index, int indent)
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
    else
    {
      cerr << endl;
    }
    
    if(node.sibling > -1)
    {
      dumpNode(nodes, node.sibling, indent);
    }
  }
  
  static void dumpNodeRaw(const Nodes &nodes, int n)
  {
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
  
  static void dumpAttrs(const Attrs &attrs)
  {
    for(int i = 0; i < attrs.size; ++i)
    {
      const Attr &attr = attrs[i];
      cerr << attr.name.getText() << "=" << attr.value.getText() << endl;
    }
  }
};

constexpr bool isAlpha(char ch)
{
  return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'); 
}

constexpr bool isNum(char ch)
{
  return (ch >= '0' && ch <= '9'); 
}

constexpr bool isAttr(char ch)
{
  return isAlpha(ch) || ch=='-'; 
}

constexpr bool isAttrVal(char ch)
{
  return isAlpha(ch) || isNum(ch); 
}

// Compare s1 and s2
// assumes s2 has a NUL terminator, but s1 may not
constexpr int cmpCaseLess(const char *s1, const char *s2)
{
  while(toUpper(*s1) == toUpper(*s2)) 
  {
    s1++;
    s2++;
  }  
  
  if(!*s2) return 0;
  
  return toUpper(*s1) < toUpper(*s2) ? -1 : 1;
}

// Compare s1 and s2 with case sensitivity
// assumes s2 has a NUL terminator, but s1 may not
constexpr int cmpCase(const char *s1, const char *s2)
{
  while(*s1 == *s2) 
  {
    s1++;
    s2++;
  }  
  
  if(!*s2) return 0;
  
  return toUpper(*s1) < toUpper(*s2) ? -1 : 1;
}


// Checks if tag[0..len) matches any entry in the tags array
constexpr int findTag(const char *const tags[], int nTags, const char *tag)
{
  int left = 0;
  int right = nTags;
  
  while(left < right)
  {
    int mid = (left + right) / 2;
    int cmp = cmpCaseLess(tag, tags[mid]);
    
    if(cmp > 0)
    {
      left = mid + 1;
    }
    else if(cmp < 0)
    {
      right = mid;
    }
    else
    {
      return mid;
    }
  }
  
  return -1;
}

constexpr const Symbol g_symID{"id"};

// Represents the state of the parser
struct SPTParser
{
  Nodes nodes;
  SymTable ids;
  
  constexpr SPTParser(const char *pszText): pszText(pszText), start(pszText) {}
  constexpr SPTParser(): pszText(), start() {}
  
  // HTML     :: TAG | HTML TAG
  // TAG      :: OPENTAG CONTENT CLOSETAG
  // CONTENT  :: HTML | TEXT 
  // OPENTAG  :: "<" TAGNAME ">"
  // CLOSETAG :: "<" TAGNAME "/>"
  // iParentIDX is the index of the oute node whose contents we are parsing
  constexpr const void parseHTML(int iParentIDX)
  {
    // If its an open tag
    if(isOpenTag())
    {
      // This next parsed nodes index will be the next free one, save it
      int iParentsEldest = iParentIDX > -1 ? nodes[iParentIDX].child : -1;
      int nYoungestSibling = iParentsEldest == -1 ? nodes.size : iParentsEldest;
      int nSiblings = iParentsEldest == -1 ? 0 : 1;
      
      // Keep doing this in a loop
      do
      {
        // Parse the open tag, get its index
        auto attrs = parseOpenTag();
        int iCurIdx = nodes.curr();
        const Symbol &symTag = nodes[iCurIdx].tag;
        
        // The first attribute node will be at iCurIdx + 1
        if(attrs.size)
        {
          // Create a node called <@ATTR>, make it the first child
          Node &node = newNode();
          node.tag = Symbol("@ATTR");
          int iAttrNode = nodes.curr();
          nodes[iCurIdx].child = iAttrNode;
          
          // For each attribute, make a node
          int nAttr = 0;
          for(const auto &attr: attrs)
          {
            Node &node = newNode();
            node.tag = attr.name;
            node.text = attr.value;
            if(!nAttr++)
            {
              nodes[iAttrNode].child = nodes.curr();
            }
            else
            {
              nodes[nodes.curr()-1].sibling = nodes.curr();
            }
          }
        }
        
        // Second sibling onwards
        if(++nSiblings > 1)
        {
          // Set elder siblings sibling index to this one, this becomes the youngest
          nodes[nYoungestSibling].sibling = iCurIdx;
          nYoungestSibling = iCurIdx;
        }
        else // First child, set the parents child index to this if parent exists
        {
          if(iParentIDX >= 0)
          {
            nodes[iParentIDX].child = iCurIdx;
          }
        }
        
        // If this is a void tag, no need to parse children or close tag
        if(symTag.pEnd[-1] != '/')
        {
          // Recursively parse the content inside
          parseHTML(iCurIdx);
          
          // Parse the close tag
          parseCloseTag(symTag);
        }
        else // eat up any whitespace
        {
          eatSpace();
        }
      }
      while(isOpenTag());
    }
    else // Has to be text content
    {
      if(iParentIDX == -1)
      {
        PARSE_ERR("Expecting an open tag at top level");
      }
      
      parseTagContent(iParentIDX);
    }
  }
  
  
private:
  const char *pszText;  // Position in the stream
  const char *start;
  
  // Return line number of current poistion
  constexpr int curLine() const
  {
    int n = 0;
    auto p = start;
    while(p != pszText)
    {
      if(*p++ == '\n') ++n;
    }
    return n;
  }
  
  // Advances the free node pointer with bounds check
  constexpr Node& newNode() 
  { 
    if(!nodes.add())
    {
      PARSE_ERR("Too many nodes");
    }
    
    return nodes.back();
  } 
  
  // Raises compiletime error if no more characters left to parse
  constexpr void checkEOS()
  {
    bool eos = !*pszText;
    if(eos) 
    {
      PARSE_ERR("Unexpected end of stream");
    }
  }
  
  // Advances to first non-whitespace character
  constexpr void eatSpace()
  {
    while(isSpace(*pszText)) ++pszText;
  }
  
  // Consumes [a-z]+
  template<typename F> constexpr const Symbol eatAlpha(F isX)
  {
    // Ensure not EOS
    checkEOS();
    
    Symbol sym;
    sym.pBeg = pszText;
    sym.pEnd = pszText;
    while(isX(*sym.pEnd)) sym.pEnd++;
    
    // Ensure at least 1 character is consumed
    if(sym.empty()) 
    {
      PARSE_ERR("Expecting an identifier");
    }
    
    pszText = sym.pEnd;
    return sym;
  }
  
  // Tries to consume the string pszSym
  constexpr const bool eatRaw(const char *pszSym)
  {
    // As long as we dont hit the end
    while(*pszSym)
    {
      // Any mismatch is a parse error
      if(*pszSym != *pszText) 
      {
        return false;
      }
      
      ++pszSym;
      ++pszText;
      
      // Ensure we are not at end of stream before the symbol has been compared fully
      if(*pszSym)
      {
        checkEOS();
      }
    }
    
    return true;
  }
  
  // Tries to consume the characters in [sym.pBeg, sym.pEnd)
  constexpr bool eatSym(const Symbol &sym)
  {
    auto p = sym.pBeg;
    while(p != sym.pEnd)
    {
      if(toUpper(*p) != toUpper(*pszText)) return false;    
      ++p;
      ++pszText;
      
      // Ensure we are not at end of stream before the symbol has been compared fully
      if(p != sym.pEnd)
      {
        checkEOS();
      }
    }
    
    return true;
  }
  
  // Consume stuff until ch is encountered
  constexpr const Symbol eatUntil(char ch, const bool *unExpected)
  {
    Symbol sym;
    sym.pBeg = pszText;
    sym.pEnd = pszText;
    while(*sym.pEnd && *sym.pEnd != ch) 
    {
      if(unExpected && unExpected[int(*sym.pEnd)])
      {
        PARSE_ERR("Unexpected character inside tag content");
      }
      sym.pEnd++;
    }
    
    pszText = sym.pEnd;
    return sym;
  }
  
  // Checks if we have an open tag at p
  constexpr bool isOpenTag()
  {
    auto saved = pszText;
    eatSpace();
    if(*pszText && *pszText++ == '<')
    {
      if(*pszText)
      {
        if(isAlpha(*pszText)) 
        {
          pszText = saved;
          return true;
        }
        
        if(*pszText == '/') 
        {
          pszText = saved;
          return false;
        }
        
        PARSE_ERR("Expecting a tag name after <");
      }
    }
    
    pszText = saved;
    return false;
  }
  
  // Checks if we have a close tag at state.text
  constexpr bool isCloseTag()
  {
    auto p = pszText;
    if(*p && *p++ == '<')
    {
      if(*p && *p++ == '/')
      {
        return isAlpha(*p);
      }
    }
    return false;
  }
  
  // Parses one attribute like NAME=VALUE
  // NAME is a sequence of [a-z\-] and VALUE is "text", 'text' or {text}
  constexpr bool parseAttrs(Array<Attr, 32> &attrs)
  {
    // Swallow any space
    eatSpace();
    checkEOS();
    
    if(isAlpha(*pszText))
    { 
      // Get the attr name
      Symbol name = eatAlpha(isAttr); 
      
      if(!eatRaw("="))
      {
        PARSE_ERR("Expecting '=' after attribute name");
      }
      
      // Check what delimiter is used "  ' or {
      char close = 0;
      switch(pszText[0])
      {
        case '"': 
        case '\'': 
          close = pszText[0];  
          pszText++;
          break;
          
        case '{':  
          close = '}'; 
          pszText++;
          break;
          
        default:
          PARSE_ERR("Expecting open quote or '{' for attribute value");
      }
      
      checkEOS();
      
      Symbol value = eatUntil(close, nullptr);
      if(value.empty())
      {
        PARSE_ERR("Empty value for attribute");
      }
      // Eat the close delim
      pszText++;
      
      // Swallow any space
      eatSpace();
      
      // Is it an ID tag
      int cmp = name.cmpCaseLess(g_symID); 
      if(cmp == 0)
      {
        if(!ids.addSym(name))
        {
          PARSE_ERR("Duplicate ID on tag");
        }
        
        nodes.back().id = value;
      }
      else
      {
        if(!attrs.add())
        {
          PARSE_ERR("Too many attributes");
        }
        
        Attr &attr = attrs.back();
        attr.name = name;
        attr.value = value;
      }

      //DUMP << "Parsed attr " << attr.name.getText() << "=" << attr.value.getText()  << ENDL;
      
      return true;
    } 
    
    return false;
  }
  
  // Parse "<TAG>", ignores leading whitespace
  // https://www.w3.org/TR/REC-xml/#sec-starttags
  // No space allowed between < and tag name
  constexpr Array<Attr, 32> parseOpenTag()
  {
    DUMP << "Parsing open tag ..." << ENDL;
    DUMP << "Node size = " << nodes.size << ENDL;
    
    // Left trim whitespace
    eatSpace();
    checkEOS();
    
    // Try to parse the "<"
    if(!eatRaw("<")) 
    {
      PARSE_ERR("Missing <");
    }
    
    // Bump the free node index
    Node &node = newNode();
    
    // Try to parse an [a-z]+ as a tag
    Symbol &sym = node.tag = eatAlpha(isAlpha);
    
    //DUMP << sym.getText() << ENDL;
    
    // Eat any trailing whitespace
    eatSpace();
    
    // Check if valid tag
    const int nTags = sizeof(arrTags)/sizeof(arrTags[0]);
    if(findTag(arrTags, nTags, sym.pBeg) == -1)
    {
      PARSE_ERR("Unknown tag name");
    }
    
    // Parse attributes
    Array<Attr, 32> attrs;
    while(parseAttrs(attrs));
    
    // Check if void tag
    const int nVoidTags = sizeof(arrVoidTags)/sizeof(arrVoidTags[0]);
    const int idx = findTag(arrVoidTags, nVoidTags, sym.pBeg);
    
    if(idx != -1)
    {
      // Void tag, add the "/" too
      eatSpace();
      
      // Bump up the symbol end, so the parser can check for a trailing / and not look for a close tag
      sym.pEnd = pszText + 1;
      
      if(!eatRaw("/")) 
      {
        PARSE_ERR("Missing / on a void tag");
      }
    }

    // Grab the final >
    if(!eatRaw(">")) 
    {
      if(idx != -1)
      {
        PARSE_ERR("Missing > on void tag");
      }
      else
      {
        PARSE_ERR("Missing > on open tag");
      }
    }
   
    return attrs;
  }
  
  // Attempts to parse "</TAG>" given "TAG"
  constexpr void parseCloseTag(const Symbol &symExpected)
  {
    // Try to parse the "</"
    if(!eatRaw("</")) 
    {
      PARSE_ERR("Expecting a close tag");
    }
    
    // Try to parse the tag name
    Symbol sym = eatAlpha(isAlpha);
    if(sym != symExpected) 
    {
      PARSE_ERR("Mismatched Close Tag");
    }
    
    //DUMP << "Close " << sym.getText() << ENDL;
    
    // Ignore space, parse >
    eatSpace();
    if(!eatRaw(">")) 
    {
      PARSE_ERR("Missing > in close tag");
    }
    
    // Eat trailing space
    eatSpace();
  }

  // Parses all text content until a "<" is hit 
  constexpr void parseTagContent(int iParentIDX)
  {
    // make sure we have something
    checkEOS();
    
    // Try to parse until a "<", forbid & and >
    bool contentUnexpectedChars[256] = {0};
    contentUnexpectedChars[int('>')] = true;
    contentUnexpectedChars[int('&')] = true;
    nodes[iParentIDX].text = eatUntil('<', contentUnexpectedChars);
    
    // Make sure we have something left
    checkEOS();
  }
  
};


constexpr SPTParser operator"" _html(const char *pszText, size_t)
{
  SPTParser parser(pszText);
  parser.parseHTML(-1);
  return parser;
}

struct SPTNode
{
  typedef map<string, string> AttrDict; 
  vector<SPTNode> children;
  AttrDict attrs;
  string tag, text, id;
  int index;
  
  SPTNode(const string &tag, const string &text, int index) : tag(tag), text(text), index(index) {}
  
  static SPTNode from(const SPTParser &parser)
  {
    // Construct a map of node ID to attr list
    
    SPTNode root("HTML", "", -1);
    build(parser, root, 0);
    return root;
  }
  
  void dump(ostream &ostr, int indent = 0) const
  {
    string sIndent = string(indent * 2, ' ');
    ostr << sIndent << "<" << tag;
    
    if(id.size())
    {
      ostr << " ID" << "=[" << id << ']';
    }
    
    for(const auto &attr: attrs)
    {
      ostr << ' ' << attr.first << '=' << '\'' << attr.second << '\'';
    }
    
    ostr << ">";
    
    if(children.size()) 
    {
      ostr << "\n";
    }
    
    for(const auto& child: children)
    {
      child.dump(ostr, indent + 1);
    }
    
    // Skip text and close tag for void tags
    if(tag.back() != '/')
    {
      if(text.length())
      {
        ostr << "\n" << sIndent << text << "\n";
      }
      ostr << sIndent << "</" << tag << ">" << "\n";
    }
    else
    {
      ostr << "\n";
    }
  }
  
private:
  static void build(const SPTParser &parser, SPTNode &parent, int index)
  {
    const Node &node = parser.nodes[index];
    auto &tag = node.getTag();
    auto &text = node.getText(tag != "PRE");
    
    SPTNode sptNode(tag, text, index);
    if(node.id.size())
    {
      sptNode.id = node.id.getText();
    }
    
    parent.children.emplace_back(sptNode);
    
    if(node.child > -1)
    {
      // Check if first child is "@ATTR"
      const Node &child = parser.nodes[node.child];
      if(child.getTag() == "@ATTR")
      {
        Node attr = parser.nodes[child.child];
        while(1)
        {
          parent.children.back().attrs[attr.getTag()] = attr.getText();
          if(attr.sibling == -1) break;
          attr = parser.nodes[attr.sibling];
        }
        
        if(child.sibling > -1)
        {
          build(parser, parent.children.back(), child.sibling);
        }
      }
      else
      {
        build(parser, parent.children.back(), node.child);
      }
    }
    
    if(node.sibling > -1)
    {
      build(parser, parent, node.sibling);
    }
  }
};

