#ifndef SEEPHIT_UTIL_H
#define SEEPHIT_UTIL_H

#pragma once

namespace spt
{
  
const int NULL_NODE = -1;
const int VOID_TAG = -2;

 
// Map of template names to values
using template_dict = map<string, string>;

// Basic constexpr functions for text processing
constexpr char to_upper(char ch)
{
  if(ch >= 'a' && ch <= 'z')
  {
    return ch + 'A' - 'a';
  }
  return ch;
}

constexpr char to_lower(char ch)
{
  if(ch >= 'A' && ch <= 'Z')
  {
    return ch + 'a' - 'A';
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

constexpr bool is_alnum(char ch)
{
  return is_alpha(ch) || (ch >= '0' && ch <= '9'); 
}

// Compare s1 and s2
// assumes s2 has a NUL terminator, but s1 may not
constexpr int comparei(const char *s1, const char *s2)
{
  while(to_lower(*s1) == to_lower(*s2)) 
  {
    s1++;
    s2++;
  }  
  
  if(!*s2) return 0;
  
  return to_lower(*s1) < to_lower(*s2) ? -1 : 1;
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
  
  return to_lower(*s1) < to_lower(*s2) ? -1 : 1;
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

// Simple abstraction for consexpr friendly dynamic arrays
// Supports a basic STL like interface
template<typename T, int SIZE> 
class vec
{
  size_t m_uSize = 0;
  const size_t m_iCapacity = SIZE;
  T m_Nodes[SIZE];

public:
  constexpr T& operator[](size_t i)             { return m_Nodes[i]; }
  constexpr const T& operator[](size_t i) const { return m_Nodes[i]; }
  
  constexpr T* begin()                  { return m_Nodes; }
  constexpr T* end()                    { return m_Nodes + m_uSize; }
                                        
  constexpr const T* begin() const      { return m_Nodes; }
  constexpr const T* end() const        { return m_Nodes + m_uSize; }
                                        
  constexpr T& back()                   { return m_Nodes[m_uSize - 1]; }
  constexpr size_t size() const         { return m_uSize; }
  constexpr int push_back(const T &val) { m_uSize++; back() = val; return m_uSize - 1;}
};

// Represents a string symbol - a pair of pointers into a const char* 
// Implements the basic functions that an STL sequence needs
// Implements comparision and ostream serialization 
struct char_view
{
  const char *m_pBeg{};
  const char *m_pEnd{};
  
  constexpr char_view() = default;
  constexpr char_view(const char *pBeg, const char *pEnd) noexcept : m_pBeg(pBeg), m_pEnd(pEnd) {}
  constexpr char_view(const char *p) noexcept : m_pBeg(p), m_pEnd(p) 
  {
    if(m_pEnd)
    {
      while(*m_pEnd) m_pEnd++;
    }
  }
  
  constexpr size_t size() const noexcept        { return m_pEnd - m_pBeg; }
  constexpr const char *begin() const noexcept  { return m_pBeg; }
  constexpr const char *end() const noexcept    { return m_pEnd; }
  constexpr bool empty() const noexcept         { return size() == 0; }
  constexpr char back() const                   { return m_pEnd[-1]; }
  constexpr char front() const                  { return m_pBeg[0]; }
  
  constexpr bool operator==(const char_view &that) const
  {
    if(size() == that.size())
    {
      auto p1 = m_pBeg, p2 = that.m_pBeg;
      while(p1 != m_pEnd)
      {
        if(to_lower(*p1) != to_lower(*p2)) return false;
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
      char a = to_lower(m_pBeg[i]);
      char b = to_lower(that.m_pBeg[i]);
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
  
  // REmoves space from either side
  constexpr void trim()
  {
    while(begin() < end() && is_space(front())) m_pBeg++;
    while(end() > begin() && is_space( back())) m_pEnd--;
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
class template_text
{
  // A sequence of char ranges, bool indicates if its a template
  // The ranges exclude the {{ and }} parts for template strings
  std::vector<pair<char_view, bool>> m_arrParts;
  
public:
  
  void add(const char_view &sym, bool bIsTemplate)
  {
    m_arrParts.push_back(std::make_pair(sym, bIsTemplate));
  }
  
  void render(ostream &ostr, const template_dict& dctTemplates) const
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
          cerr << endl << "Template key undefined: '" << sKey << "'" << endl;
          throw false;
        }
        
        ostr << i->second;
      }
      else // Non template text, render it
      {
        ostr << part.first;
      }
    }
  }
  
  std::vector<pair<char_view, bool>> parts() const { return m_arrParts; }
  
};

// Simple abstraction for a symbol table
struct sym_tab
{
  vec<char_view, 1024> m_arrSyms;
  
  // Adds a symbol to the table, returns false if already exists
  constexpr bool addSym(const char_view &symNew)
  {
    for(const auto &sym: m_arrSyms)
    {
      if(symNew == sym) return false;
    }
    m_arrSyms.push_back(symNew);
    return true;
  }
};

struct attr 
{
  char_view name;      
  char_view value;  
  
  constexpr attr() noexcept = default;
  constexpr attr(const char_view &name, const char_view &value): name(name), value(value) {}
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
   * HTML-> NULL_NODE
   * |
   * DIV -> DIV --------->  DIV -> -1
   * |      |                  |
   * NULL   P->NULL_NODE    NULL_NODE
   * 
   * Since this is meant to be a compile time DS, pointers sibling and child are just ints
   * They index into an array of fixed size
   * 
   * Attributes are nested as children under <_ATTR>
   * Styles are nested under <_ATTR><_STYLE></_STYLE></_ATTR>
   * 
   * child == VOID_TAG is a special case
   * 
   */
  
  constexpr cnode() = default;
    
  constexpr cnode(const char_view &tag):
    sibling(NULL_NODE), child(NULL_NODE), tag(tag){}

  constexpr cnode(const char_view &tag, const char_view &text):
    sibling(NULL_NODE), child(NULL_NODE), tag(tag), text(text){}
    
    
  void dump() const
  {
    cerr << "id=" << id << ",";
    cerr << "sibling=" << sibling << ",";
    cerr << "child=" << child << ",";
    cerr << "tag=" << tag << ",";
    cerr << "text=" << text << endl;
  }
  
  // returns the tag in lowercase
  const string getTag() const 
  {
    string ret(tag.m_pBeg, tag.size());
    transform(begin(ret), end(ret), begin(ret), to_lower);
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
  
  int sibling = NULL_NODE;
  int child = NULL_NODE;
  char_view tag;
  char_view text;
  char_view id;
};


} // namespace spt

#endif
