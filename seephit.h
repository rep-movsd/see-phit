#pragma once
#include <string>
#include "tags.hpp"

using namespace std;

// Allow runtime debugging for development
#ifdef SPT_DEBUG

#define constexpr 
#define DUMP cerr
#define ENDL "\n"

#else

#define DUMP DummyOut
#define ENDL 0

struct DummyOutStream
{
  template<typename T>  constexpr const DummyOutStream& operator <<(const T &) const { return *this;}
};

constexpr DummyOutStream DummyOut;

#endif


struct Symbol
{
  constexpr Symbol(): pBeg(), pEnd() {}
  const char *pBeg;
  const char *pEnd;
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

// Dummy function to prettify compile-time errors
void ParseError(const char* err);

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
constexpr const Symbol eatUntil(const char *pszText, char ch)
{
  Symbol sym;
  sym.pBeg = pszText;
  sym.pEnd = pszText;
  while(*sym.pEnd && *sym.pEnd != ch) sym.pEnd++;
  return sym;
}

constexpr int compare(const char *s1, const char *s2)
{
  while(*s1 == *s2) 
  {
    // NUL found, were done
    if(!*s1)
    {
      return 0;
    }
    
    s1++;
    s2++;
  }  
  
  return *s1 < *s2 ? -1 : 1;
}

constexpr int findTag(const char *tag)
{
  int left = 0;
  int right = sizeof(tags) / sizeof(tags[0]);
  
  while(left < right)
  {
    int mid = (left + right) / 2;
    int cmp = compare(tag, tags[mid]);
    
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
constexpr const ParseState parseOpenTag(Nodes &nodes, ParseState state)
{
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
  state.text = nodes[state.iFree].tag.pEnd;
  state.text = eatRaw(state.text, ">");
  
  bool closeBracket = state.text;
  if(!closeBracket) ParseError("Missing >");
  
  // Bump the free node index
  state.iFree++;
  
  return state;
}

// Attempts to parse "</TAG>" given "TAG"
constexpr const ParseState parseCloseTag(ParseState state, const Symbol &sym)
{
  // Try to parse the "</"
  state.text = eatRaw(state.text, "</");
  bool openBracketSlash = state.text;
  if(!openBracketSlash) 
  {
    ParseError("Missing </");
  }
  
  // Try to parse the tag name then the closing ">"
  state.text = eatSym(state.text, sym);
  bool matchTag = state.text;
  if(!matchTag) ParseError("Mismatched tag name after </");
  
  state.text = eatRaw(state.text, ">");
  bool closeBracket = state.text;
  if(!closeBracket) ParseError("Missing >");
  
  state.text = eatSpace(state.text);
  return state;
}

// Parses all text content until a "<" is hit 
constexpr const ParseState parseTagContent(Nodes &nodes, ParseState state, int iParentIDX)
{
  // make sure we have something
  checkEOS(state.text);
  
  // Try to parse until a "<"
  nodes[iParentIDX].text = eatUntil(state.text, '<');
  
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
    return isAlpha(*p);
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
      
      // Recursively parse the content inside
      state = parseHTML(nodes, state, iCurIdx);
      
      // Parse the close tag
      state = parseCloseTag(state, symTag);
    }
    while(isOpenTag(state.text));
  }
  else // Has to be text content
  {
    if(iParentIDX == 0)
    {
      ParseError("Expecting top level tag");
    }
    
    state = parseTagContent(nodes, state, iParentIDX);
  }
  
  return state;
}

constexpr Nodes operator"" _html(const char *pszText, size_t len)
{
  Nodes nodes {};
  ParseState state(pszText);
  parseHTML(nodes, state, 0);
  return nodes;
}

