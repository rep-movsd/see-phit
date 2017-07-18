#pragma once
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

struct Symbol
{
  constexpr Symbol(): pBeg(), pEnd() {}
  constexpr int len() const { return pEnd - pBeg;}
  const char *pBeg;
  const char *pEnd;
  
  string getText() const { return string(pBeg, pEnd); }
  
  constexpr size_t size() const { return pEnd - pBeg; } 
  
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
  
};

template<typename T, int SIZE>
struct Array
{
  T nodes[SIZE];
  constexpr T& operator[](size_t i) { return nodes[i]; }
  constexpr const T& operator[](size_t i) const { return nodes[i]; }
  size_t size = 0;
  const size_t capacity = SIZE;
  
  constexpr T& back() { return nodes[size]; }
  constexpr bool add() { return ++size < capacity;}
};

struct Attr 
{
  int iNode;
  Symbol name;      
  Symbol value;  
  
  constexpr Attr():iNode() {}
  
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
   */
  
  constexpr Node():sibling(-1), child(-1), tag(), text() {}
  constexpr Node(const Node &n):
  sibling(n.sibling), child(n.child), tag(n.tag), text(n.text) {}
  
  // returns the tag in uppercase
  const string getTag() const 
  {
    string ret(tag.pBeg, tag.pEnd - tag.pBeg);
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
constexpr int cmpUpperCase(const char *s1, const char *s2)
{
  while(toUpper(*s1) == toUpper(*s2)) 
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
    int cmp = cmpUpperCase(tag, tags[mid]);
    
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


// Represents the state of the parser
struct SPTParser
{
  Nodes nodes;
  Attrs attrs;
  
  constexpr SPTParser(const char *pszText): pszText(pszText), start(pszText) {}
  constexpr SPTParser(): pszText(), start() {}
  
  // HTML     :: TAG | HTML TAG
  // TAG      :: OPENTAG CONTENT CLOSETAG
  // CONTENT  :: HTML | TEXT 
  // OPENTAG  :: "<" TAGNAME ">"
  // CLOSETAG :: "<" TAGNAME "/>"
  constexpr const void parseHTML(int iParentIDX)
  {
    // If its an open tag
    if(isOpenTag())
    {
      DUMP << "Found open tag ..." << ENDL;
      
      // This next parsed nodes index will be the next free one, save it
      int nYoungestSibling = nodes.size;
      int nSiblings = 0;
      
      // Keep doing this in a loop
      do
      {
        // Parse the open tag
        int iCurIdx = nodes.size;
        parseOpenTag();
        const Symbol symTag = nodes[iCurIdx].tag;
        
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
  constexpr void newNode() 
  { 
    if(!nodes.add())
    {
      PARSE_ERR("Too many nodes");
    }
  } 
  
  // Advances the free attr pointer with bounds check, associates new attr with the current ndoe
  constexpr Attr &newAttr() 
  { 
    if(!attrs.add())
    {
      PARSE_ERR("Too many attributes");
    }
    
    // Link the attribute to the node
    attrs.back().iNode = nodes.size;
    return attrs.back();
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
    bool empty_tag = sym.pBeg == sym.pEnd;
    if(empty_tag) 
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
  constexpr void parseAttr()
  {
    Attr &attr = newAttr();
    
    // Swallow any space
    eatSpace();
    checkEOS();
    
    // Grab the name 
    attr.name = eatAlpha(isAttr);
    
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
    
    attr.value = eatUntil(close, nullptr);
    checkEOS();
    
    // Eat the close delim
    pszText++;
    
    // Swallow any space
    eatSpace();
  }
  
  // Parse "<TAG>", ignores leading whitespace
  // https://www.w3.org/TR/REC-xml/#sec-starttags
  // No space allowed between < and tag name
  constexpr void parseOpenTag()
  {
    DUMP << "Parsing open tag ..." << ENDL;
    
    // Left trim whitespace
    eatSpace();
    checkEOS();
    
    // Try to parse the "<"
    if(!eatRaw("<")) 
    {
      PARSE_ERR("Missing <");
    }
    
    // Try to parse an [a-z]+ as a tag then the closing ">"
    Symbol &sym = nodes.back().tag = eatAlpha(isAlpha);
    
    // Eat any trailing whitespace
    eatSpace();
    
    // Check if valid tag
    const int nTags = sizeof(arrTags)/sizeof(arrTags[0]);
    if(findTag(arrTags, nTags, sym.pBeg) == -1)
    {
      PARSE_ERR("Unknown tag name");
    }
    
    // Parse attributes
    //parseAttr()
    
    // Check if void tag
    const int nVoidTags = sizeof(arrVoidTags)/sizeof(arrVoidTags[0]);
    const int idx = findTag(arrVoidTags, nVoidTags, sym.pBeg);
    
    if(idx != -1)
    {
      DUMP << "Parsed void tag : index " << idx << " sym len " << sym.len() << ENDL;
      
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
    
    // Bump the free node index
    newNode();
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
  vector<SPTNode> children;
  string tag, text;
  
  SPTNode(const string &tag, const string &text) : tag(tag), text(text) {}
  
  static SPTNode from(const SPTParser &parser)
  {
    map<int, SPTNode*> dctNodes;
    SPTNode root("HTML", "");
    build(root, parser.nodes, 0, dctNodes);
    
    for(int n = 0; n < parser.attrs.size; ++n)
    {
      
    }
    
    return root;
  }
  
  void dump(ostream &ostr, int indent = 0) const
  {
    string sIndent = string(indent * 2, ' ');
    ostr << sIndent << "<" << tag << ">";
    
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
  static void build(SPTNode &parent, const Nodes &nodes, int index, map<int, SPTNode*> &dctNodes)
  {
    const Node &node = nodes[index];
    auto &tag = node.getTag();
    auto &text = node.getText(tag != "PRE");
    parent.children.emplace_back(SPTNode(tag, text));
    dctNodes[index] = &parent.children.back(); 
    
    if(node.child > -1)
    {
      build(parent.children.back(), nodes, node.child, dctNodes);
    }
    
    if(node.sibling > -1)
    {
      build(parent, nodes, node.sibling, dctNodes);
    }
  }
};
