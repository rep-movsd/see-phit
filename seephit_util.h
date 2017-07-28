#pragma once

namespace spt
{
  
// Map of template names to values
typedef map<string, string> template_dict;

// Basic constexpr functions for text processing
constexpr char to_upper(char ch)
{
  if(ch >= 'a' && ch <= 'z')
  {
    return ch + 'A' - 'a';
  }
  return ch;
}

constexpr bool is_space(char ch)
{
  return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t';
}

constexpr bool is_alpha(char ch)
{
  return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'); 
}

constexpr bool is_digit(char ch)
{
  return (ch >= '0' && ch <= '9'); 
}

constexpr bool is_attr(char ch)
{
  return is_alpha(ch) || ch=='-'; 
}

constexpr bool is_attrval(char ch)
{
  return is_alpha(ch) || is_digit(ch); 
}

// Compare s1 and s2
// assumes s2 has a NUL terminator, but s1 may not
constexpr int comparei(const char *s1, const char *s2)
{
  while(to_upper(*s1) == to_upper(*s2)) 
  {
    s1++;
    s2++;
  }  
  
  if(!*s2) return 0;
  
  return to_upper(*s1) < to_upper(*s2) ? -1 : 1;
}

// Compare s1 and s2 with case sensitivity
// assumes s2 has a NUL terminator, but s1 may not
constexpr int compare(const char *s1, const char *s2)
{
  while(*s1 == *s2) 
  {
    s1++;
    s2++;
  }  
  
  if(!*s2) return 0;
  
  return to_upper(*s1) < to_upper(*s2) ? -1 : 1;
}

// Checks if val matches any entry in the array arr
constexpr int find_arr(const char *const arr[], int nTags, const char *val)
{
  int left = 0;
  int right = nTags;
  
  while(left < right)
  {
    int mid = (left + right) / 2;
    int cmp = comparei(val, arr[mid]);
    
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

// Simple abstraction for consexpr friendly arrays
// Supports a basic STL like interface
template<typename T, int SIZE> 
class array
{
  size_t m_uSize = 0;
  const size_t m_iCapacity = SIZE;
  T m_Nodes[SIZE];

public:
  constexpr T& operator[](size_t i)             { return m_Nodes[i]; }
  constexpr const T& operator[](size_t i) const { return m_Nodes[i]; }
  constexpr T* begin()                          { return m_Nodes; }
  constexpr T* end()                            { return m_Nodes + m_uSize; }
  constexpr T& back()                           { return m_Nodes[m_uSize - 1]; }
  constexpr size_t size() const                 { return m_uSize; }
  constexpr size_t back_index() const           { return m_uSize - 1;}
  constexpr bool extend()                       { return (m_uSize++) < m_iCapacity; }
};

// Represents a string symbol - a pair of pointers into a const char* 
// Implements the basic functions that an STL sequence needs
// Implements comparision and ostream serialization 
struct char_view
{
  const char *m_pBeg;
  const char *m_pEnd;
  
  constexpr char_view(): m_pBeg(), m_pEnd() {}
  constexpr char_view(const char *pBeg, const char *pEnd): m_pBeg(pBeg), m_pEnd(pEnd) {}
  constexpr char_view(const char *p): m_pBeg(p), m_pEnd(p)
  {
    while(*m_pEnd) m_pEnd++;
  }
  
  constexpr size_t size() const       { return m_pEnd - m_pBeg; }
  constexpr const char *begin() const { return m_pBeg; }
  constexpr const char *end() const   { return m_pEnd; }
  constexpr char back() const         { return m_pEnd[-1]; }
  constexpr bool empty() const        { return size() == 0; }
  
  constexpr bool operator==(const char_view &that) const
  {
    if(size() == that.size())
    {
      auto p1 = m_pBeg, p2 = that.m_pBeg;
      while(p1 != m_pEnd)
      {
        if(to_upper(*p1) != to_upper(*p2)) return false;
        ++p1;
        ++p2;
      }
      
      return true;
    }
    return false;
  }
  
  constexpr bool operator!=(const char_view &that) const
  {
    return !(*this == that);
  }
  
  // Compare sym1 and sym2
  constexpr int cmpCaseLess(const char_view &that) const
  {
    if(size() < that.size()) return -1;
    if(size() > that.size()) return 1;
    
    for(size_t i = 0; i < size(); ++i)
    {
      char a = to_upper(m_pBeg[i]);
      char b = to_upper(that.m_pBeg[i]);
      if(a < b) return -1;
      if(a > b) return 1;
    }
    
    return 0;
  }
  
  // Compare s1 and s2 with case sensitivity
  constexpr int cmpCase(const char_view &that) const
  {
    if(size() < that.size()) return -1;
    if(size() > that.size()) return 1;
    
    for(size_t i = 0; i < size(); ++i)
    {
      char a = m_pBeg[i];
      char b = that.m_pBeg[i];
      if(a < b) return -1;
      if(a > b) return 1;
    }
    return 0;
  }
  
};

// Ostream helper for above
inline ostream& operator<<(ostream &ostr, const char_view &sym)
{
  ostr.write(sym.m_pBeg, sym.size());
  return ostr;
}

// Abstracts templatable text
// A Sequence of char_view, pointing to eitehr plain text or template keys
struct template_text
{
  // A sequence of char ranges, bool indicates if its a template
  // The ranges exclude the {{ and }} parts for template strings
  array<pair<char_view, bool>, 16> m_arrParts;
  
  void render(ostream &ostr, const template_dict& dctTemplates)
  {
    // Render each part
    for(const auto &part: m_arrParts)
    {
      // If its a template, render the template value
      if(part.second)
      {
        // Get key and ensure it is in the map, then render the value
        string sKey(part.first.begin(), part.first.end());
        auto i = dctTemplates.find(sKey);
        if(i == dctTemplates.end())
        {
          throw string("Template key undefined:") + sKey;
        }
        
        ostr << i->second;
      }
      else // Non template text, render it
      {
        ostr << part.first;
      }
    }
  }
};

// Simple abstraction for a symbol table
struct sym_tab
{
  array<char_view, 1024> m_arrSyms;
  
  // Adds a symbol to the table, returns false if already exists
  constexpr bool addSym(const char_view &symNew)
  {
    for(const auto &sym: m_arrSyms)
    {
      if(symNew == sym) return false;
    }
    m_arrSyms.extend();
    m_arrSyms.back() = symNew;
    return true;
  }
};

struct attr 
{
  int node;
  char_view name;      
  char_view value;  
  
  constexpr attr():node(-1) {}
  
  constexpr attr(int iNode, const char_view &name, const char_view &value):
  node(iNode), name(name), value(value) {}
  
  constexpr attr(const attr &a):node(a.node), name(a.name), value(a.value){}
};

struct cnode
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
  
  constexpr cnode():sibling(-1), child(-1), tag(), text(), id() {}
  constexpr cnode(const cnode &n):
  sibling(n.sibling), child(n.child), tag(n.tag), text(n.text), id(n.id) {}
  
  // returns the tag in uppercase
  const string getTag() const 
  {
    string ret(tag.m_pBeg, tag.size());
    transform(begin(ret), end(ret), begin(ret), to_upper);
    return ret;
  };
  
  // Returns the text content, optionally stripping leading and trailing space
  const string getText(bool trim = false) const 
  {
    auto b = text.m_pBeg, e = text.m_pEnd;
    if(trim && b != e)
    {
      while(is_space(*b)) b++;
      while(e >= b && is_space(e[-1])) e--;
    }
    
    return string(b, e);
  };
  
  int sibling;
  int child;
  char_view tag;
  char_view text;
  char_view id;
};

}