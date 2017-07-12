#pragma once
#include <string>
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

// Dummy function to prettify compile-time errors
void ParseError(const char* err);

#endif


struct Symbol
{
  constexpr Symbol(): pBeg(), pEnd() {}
  constexpr int len() const { return pEnd - pBeg;}
  const char *pBeg;
  const char *pEnd;
  
  string getText() const { return string(pBeg, pEnd); }
};

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

struct Node
{
  constexpr Node():sibling(-1), child(-1), tag(), text() {}
  constexpr Node(const Node &n):
  sibling(n.sibling), child(n.child), tag(n.tag), text(n.text) {}
  
  const string getTag() const {return string(tag.pBeg, tag.pEnd - tag.pBeg);};
  const string getText() const {return string(text.pBeg, text.pEnd - text.pBeg);};
  
  int sibling;
  int child;
  Symbol tag;
  Symbol text;
};

template<int SIZE>
struct NodeArray
{
  Node nodes[SIZE];
  constexpr Node& operator[](size_t i) { return nodes[i]; }
  constexpr const Node& operator[](size_t i) const { return nodes[i]; }
};

typedef NodeArray<256> Nodes;

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
};


// Represents the state of the parser
struct ParseState
{
  const char *text;  // Position in the stream
  const char *start;
  size_t iFree;      // Index of next free node
  
  constexpr ParseState(const char *pos):text(pos), start(pos), iFree(){}
  constexpr ParseState():text(), start(), iFree(){}
  
  constexpr int pos() {return text - start;}
};

void dumpNode(const Nodes &nodes, int index, int indent);
void dumpNodeRaw(const Nodes &nodes, int n);

constexpr bool isAlpha(char ch)
{
  return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'); 
}

// Raises compiletime error if no more characters left to parse
constexpr const char *checkEOS(const char *pszText)
{
  bool eos = !*pszText;
  if(eos) ParseError("Unexpected end of stream");
  
  return pszText;
}

// Takes a string literal and returns pointer to first non-whitespace character
constexpr const char *eatSpace(const char *pszText)
{
  while(*pszText == ' ' || *pszText == '\n' || *pszText == '\r' || *pszText == '\t')
  {
    ++pszText;
  }
  return pszText;
}

// Takes a string literal text and tries to consume the characters in pszSym
constexpr const char *eatRaw(const char *pszText, const char *pszSym)
{
  // As long as we dont hit the end
  while(*pszSym)
  {
    // Any mismatch is a parse error
    if(*pszSym != *pszText) 
    {
      return nullptr;
    }
    
    ++pszSym;
    ++pszText;
    
    // Ensure we are not at end of stream before the symbol has been compared fully
    if(*pszSym)
    {
      checkEOS(pszText);
    }
  }
  
  return pszText;
}

// Takes a string literal text and tries to consume the characters in [sym.pBeg, sym.pEnd)
constexpr const char *eatSym(const char *pszText, const Symbol &sym)
{
  auto p = sym.pBeg;
  while(p != sym.pEnd)
  {
    if(*p != *pszText) return nullptr;    
    ++p;
    ++pszText;
    
    // Ensure we are not at end of stream before the symbol has been compared fully
    if(p != sym.pEnd)
    {
      checkEOS(pszText);
    }
  }
  
  return pszText;
}

// Takes a string literal text and tries to consume [a-z]+
constexpr const Symbol eatAlpha(const char *pszText)
{
  // Ensure not EOS
  checkEOS(pszText);
  
  Symbol sym;
  sym.pBeg = pszText;
  sym.pEnd = pszText;
  while(isAlpha(*sym.pEnd)) sym.pEnd++;
  
  // Ensure at least 1 character is consumed
  bool empty_tag = sym.pBeg == sym.pEnd;
  if(empty_tag) ParseError("Expecting an identifier");
  
  return sym;
}

// Takes a string literal text and tries to consume stuff until ch is encountered
constexpr const Symbol eatUntil(const char *pszText, char ch, const bool *unExpected)
{
  Symbol sym;
  sym.pBeg = pszText;
  sym.pEnd = pszText;
  while(*sym.pEnd && *sym.pEnd != ch) 
  {
    if(unExpected && unExpected[int(*sym.pEnd)])
    {
      ParseError("Unexpected character inside tag content");
    }
    sym.pEnd++;
  }
  
  return sym;
}

constexpr char toUpper(char ch)
{
  if(ch >= 'a' && ch <= 'z')
  {
    return ch + 'A' - 'a';
  }
  
  return ch;
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

// Parse "<TAG>", ignores leading whitespace
// https://www.w3.org/TR/REC-xml/#sec-starttags
// No space alloed between < and tag name
constexpr const ParseState parseOpenTag(Nodes &nodes, ParseState state)
{
  DUMP << "Parsing open tag ..." << ENDL;
  
  // Left trim whitespace
  state.text = eatSpace(state.text);
  checkEOS(state.text);
  
  // Try to parse the "<"
  bool openBracket = *state.text++ == '<';
  if(!openBracket) 
  {
    ParseError("Missing <");
  }
  
  // Try to parse an [a-z]+ as a tag then the closing ">"
  nodes[state.iFree].tag = eatAlpha(state.text);
  Symbol &sym = nodes[state.iFree].tag;

  // Eat any trailing whitespace
  state.text = eatSpace(sym.pEnd);
  
  // Check if valid tag
  const int nTags = sizeof(arrTags)/sizeof(arrTags[0]);
  if(findTag(arrTags, nTags, sym.pBeg) == -1)
  {
    ParseError("Unknown tag name");
  }
  
  //DUMP << "Parsed tag " << string(sym.pBeg, sym.pEnd) << ENDL;
  
  // Check if void tag
  const int nVoidTags = sizeof(arrVoidTags)/sizeof(arrVoidTags[0]);
  const int idx = findTag(arrVoidTags, nVoidTags, sym.pBeg);
  DUMP << "void tag index " << idx << " sym len " << sym.len() << ENDL;
  
  if(idx != -1)
  {
    // Void tag, add the "/" too
    state.text = eatSpace(state.text);
    state.text = eatRaw(state.text, "/");
    
    // Bump up the symbol end, so the parser can check for a trailing / and not look for a close tag
    sym.pEnd = state.text;

    bool voidTag = state.text;
    if(!voidTag) ParseError("Missing / on a void tag");
  }

  // Grab the final >
  state.text = eatRaw(state.text, ">");
  bool closeBracket = state.text;
  if(!closeBracket) 
  {
    if(idx != -1)
    {
      ParseError("Missing > on void tag");
    }
    else
    {
      ParseError("Missing > on open tag");
    }
  }
  
  // Bump the free node index
  state.iFree++;
  
  return state;
}

// Attempts to parse "</TAG>" given "TAG"
constexpr const ParseState parseCloseTag(ParseState state, const Symbol &sym)
{
  DUMP << "Parsing close tag ..." << ENDL;

  // Try to parse the "</"
  state.text = eatRaw(state.text, "</");
  bool openBracketSlash = state.text;
  if(!openBracketSlash) 
  {
    ParseError("Missing </");
  }
  
  // Try to parse the tag name then the closing ">"
  state.text = eatSym(state.text, sym);
  bool matchTag = !isAlpha(*state.text);
  if(!matchTag) 
  {
    ParseError("Mismatched tag name after </");
  }
  
  // Ignore space, parse >
  state.text = eatSpace(state.text);
  state.text = eatRaw(state.text, ">");
  bool closeBracket = state.text;
  if(!closeBracket) 
  {
    ParseError("Missing > in close tag");
  }
  
  state.text = eatSpace(state.text);
  return state;
}

// Parses all text content until a "<" is hit 
constexpr const ParseState parseTagContent(Nodes &nodes, ParseState state, int iParentIDX)
{
  // make sure we have something
  checkEOS(state.text);
  
  // Try to parse until a "<", forbid & and >
  bool contentUnexpectedChars[256] = {0};
  contentUnexpectedChars[int('>')] = true;
  contentUnexpectedChars[int('&')] = true;
  nodes[iParentIDX].text = eatUntil(state.text, '<', contentUnexpectedChars);
  
  // Make sure we have something left
  checkEOS(state.text);
  
  // Advance the pointer to point to the "<" 
  state.text = nodes[iParentIDX].text.pEnd; 
  
  return state;
}

// Checks if we have an open tag at p
constexpr bool isOpenTag(const char *p)
{
  p = eatSpace(p);
  if(*p && *p++ == '<')
  {
    if(*p)
    {
      if(isAlpha(*p)) return true;
      if(*p == '/') return false;
      
      ParseError("Expecting a tag name after <");
    }
  }
  return false;
}

// Checks if we have a close tag at state.text
constexpr bool isCloseTag(const char *p)
{
  if(*p && *p++ == '<')
  {
    if(*p && *p++ == '/')
    {
      return isAlpha(*p);
    }
  }
  return false;
}

// HTML     :: TAG | HTML TAG
// TAG      :: OPENTAG CONTENT CLOSETAG
// CONTENT  :: HTML | TEXT 
// OPENTAG  :: "<" TAGNAME ">"
// CLOSETAG :: "<" TAGNAME "/>"
constexpr const ParseState parseHTML(Nodes &nodes, ParseState state, int iParentIDX)
{
  // If its an open tag
  if(isOpenTag(state.text))
  {
    DUMP << "Found open tag ..." << ENDL;
    
    // This next parsed nodes index will be the next free one, save it
    int nYoungestSibling = state.iFree;
    int nSiblings = 0;
    
    // Keep doing this in a loop
    do
    {
      // Parse the open tag
      int iCurIdx = state.iFree;
      state = parseOpenTag(nodes, state);
      const Symbol symTag = nodes[iCurIdx].tag;
      
      //DUMP << symTag.getText() << ENDL;
      
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
        DUMP << "Found nested tag ..." << ENDL;
        
        // Recursively parse the content inside
        state = parseHTML(nodes, state, iCurIdx);
        
        // Parse the close tag
        state = parseCloseTag(state, symTag);
      }
      else // eat up any whitespace
      {
        state.text = eatSpace(state.text);
      }
    }
    while(isOpenTag(state.text));
  }
  else // Has to be text content
  {
    DUMP << iParentIDX << ENDL;
    
    if(iParentIDX == -1)
    {
      ParseError("Expecting an open tag at top level");
    }
    
    state = parseTagContent(nodes, state, iParentIDX);
  }
  
  return state;
}

constexpr Nodes operator"" _html(const char *pszText, size_t len)
{
  Nodes nodes {};
  ParseState state(pszText);
  parseHTML(nodes, state, -1);
  return nodes;
}

