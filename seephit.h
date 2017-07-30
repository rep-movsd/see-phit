#pragma once
#include <cassert>
#include <cstdint>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <utility>
#include <iostream>

using std::string;
using std::vector;
using std::pair;
using std::map;

using std::find;

using std::ostream;
using std::cerr;
using std::endl;

#include "tags.hpp"
#include "seephit_debug.h"
#include "seephit_util.h"

// maximum nodes and attributes in the tree
#define SPT_MAX_NODES 1024
#define SPT_MAX_ATTRS 1024

namespace spt
{

typedef vec<attr, SPT_MAX_ATTRS> attrs;
typedef vec<cnode, SPT_MAX_NODES> cnodes;

// Hardcoded symbols to detect id and style
constexpr const char_view g_symID{"id"};
constexpr const char_view g_symStyle{"style"};
constexpr const char_view g_symAttr{"attr"};
constexpr const char_view g_symPre{"pre"};

// Compile time parser
struct parser
{
  cnodes nodes;
  sym_tab ids;
  
  constexpr parser(const char *pszText): pszText(pszText), pszStart(pszText) {}
  constexpr parser(): pszText(), pszStart() {}
  
  // HTML     :: TAG | HTML TAG
  // TAG      :: OPENTAG CONTENT CLOSETAG
  // CONTENT  :: HTML | TEXT 
  // OPENTAG  :: "<" TAGNAME ">"
  // CLOSETAG :: "<" TAGNAME "/>"
  // iParentId is the index of the outer node whose contents we are parsing
  constexpr const void parse_html(int iParentId)
  {
    // If its an open tag
    if(is_open_tag())
    {
      // Get this parents eldest child if any (could be the ATTR node)
      int iEldestSibling = iParentId > -1 ? nodes[iParentId].child : -1;
      
      // If there is no eldest iEldestSibling, then we are the first child
      int iYoungestSibling = iEldestSibling == -1 ? nodes.size() : iEldestSibling;
      
      // If there was a child for this parent set the siblings count
      int nSiblings = iEldestSibling == -1 ? 0 : 1;
      
      // Keep doing this in a loop
      do
      {
        // Parse the open tag, get its index
        auto attrs = parse_open_tag();
        int iCurrId = nodes.size() - 1;
        const char_view &symTag = nodes[iCurrId].tag;
        
        // The first attribute node will be at iCurIdx + 1
        if(attrs.size())
        {
          // Create a node called <@ATTR>, make it the first child
          int iAttrNode = nodes.push_back(cnode(g_symAttr));
          nodes[iCurrId].child = iAttrNode;
          
          // For each attribute, make a node
          int nAttr = 0;
          for(const auto &attr: attrs)
          {
            int iBack = nodes.push_back(cnode(attr.name, attr.value));
            if(!nAttr++)
            {
              nodes[iAttrNode].child = iBack;
            }
            else
            {
              nodes[iBack-1].sibling = iBack;
            }
          }
        }
        
        // Second sibling onwards
        if(++nSiblings > 1)
        {
          // Set elder siblings sibling index to this one, this becomes the youngest
          nodes[iYoungestSibling].sibling = iCurrId;
          iYoungestSibling = iCurrId;
        }
        else // First child, set the parents child index to this if parent exists
        {
          if(iParentId >= 0)
          {
            DUMP << "Setting child of " << iParentId << " to " << iCurrId << ENDL;
            nodes[iParentId].child = iCurrId;
          }
        }
        
        // If this is a void tag, no need to parse children or close tag
        if(symTag.m_pEnd[-1] != '/')
        {
          // Recursively parse the content inside
          parse_html(iCurrId);
          
          // Parse the close tag
          parse_close_tag(symTag);
        }
        else // eat up any whitespace
        {
          eat_space();
        }
      }
      while(is_open_tag());
    }
    else // Has to be text content
    {
      if(iParentId == -1)
      {
        PARSE_ERR("Expecting an open tag at top level");
      }
      
      bool bTrim = nodes[iParentId].tag != g_symPre;
      parse_tag_content(iParentId, bTrim);
    }
  }
  
 
  void dump() const 
  {
    int i = 0;
    for(const auto &node: nodes)
    {
      cerr << "n=" << i++ << endl;
      node.dump();
    }
  }
 
private:
  const char *pszText;  // Position in the stream
  const char *pszStart;
  
  // Return line number of current poistion
  constexpr int cur_line() const
  {
    int n = 0;
    auto p = pszStart;
    while(p != pszText)
    {
      if(*p++ == '\n') ++n;
    }
    return n;
  }
  
  // Raises compiletime error if no more characters left to parse
  constexpr void check_eos()
  {
    bool eos = !*pszText;
    if(eos) 
    {
      PARSE_ERR("Unexpected end of stream");
    }
  }
  
  // Advances to first non-whitespace character
  constexpr void eat_space()
  {
    while(is_space(*pszText)) ++pszText;
  }
  
  // Consumes [a-z]+
  template<typename F> constexpr const char_view eat_alpha(F isX)
  {
    // Ensure not EOS
    check_eos();
    
    char_view sym;
    sym.m_pBeg = pszText;
    sym.m_pEnd = pszText;
    while(isX(*sym.m_pEnd)) sym.m_pEnd++;
    
    // Ensure at least 1 character is consumed
    if(sym.empty()) 
    {
      PARSE_ERR("Expecting an identifier");
    }
    
    pszText = sym.m_pEnd;
    return sym;
  }
  
  // Tries to consume the string pszSym
  constexpr const bool eat_str(const char *pszSym)
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
        check_eos();
      }
    }
    
    return true;
  }
  
  // Tries to consume the characters in [sym.pBeg, sym.pEnd)
  constexpr bool eat_symbol(const char_view &sym)
  {
    auto p = sym.m_pBeg;
    while(p != sym.m_pEnd)
    {
      if(to_upper(*p) != to_upper(*pszText)) return false;    
      ++p;
      ++pszText;
      
      // Ensure we are not at end of stream before the symbol has been compared fully
      if(p != sym.m_pEnd)
      {
        check_eos();
      }
    }
    
    return true;
  }
  
  // Consume stuff until ch is encountered
  constexpr const char_view eat_until(char ch, const bool *unExpected)
  {
    char_view sym;
    sym.m_pBeg = pszText;
    sym.m_pEnd = pszText;
    while(*sym.m_pEnd && *sym.m_pEnd != ch) 
    {
      if(unExpected && unExpected[int(*sym.m_pEnd)])
      {
        PARSE_ERR("Unexpected character inside tag content");
      }
      sym.m_pEnd++;
    }
    
    pszText = sym.m_pEnd;
    return sym;
  }
  
  // Checks if we have an open tag at p
  constexpr bool is_open_tag()
  {
    auto saved = pszText;
    eat_space();
    if(*pszText && *pszText++ == '<')
    {
      if(*pszText)
      {
        if(is_alpha(*pszText)) 
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
  constexpr bool is_close_tag()
  {
    auto p = pszText;
    if(*p && *p++ == '<')
    {
      if(*p && *p++ == '/')
      {
        return is_alpha(*p);
      }
    }
    return false;
  }
  
  // Parses one attribute like NAME=VALUE
  // NAME is a sequence of [a-z\-] and VALUE is "text", 'text' or {text}
  constexpr bool parse_attrs(vec<attr, 32> &attrs)
  {
    // Swallow any space
    eat_space();
    check_eos();
    
    if(is_alpha(*pszText))
    { 
      // Get the attr name
      char_view name = eat_alpha(is_attr); 
      
      if(!eat_str("="))
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
      
      check_eos();
      
      char_view value = eat_until(close, nullptr);
      if(value.empty())
      {
        PARSE_ERR("Empty value for attribute");
      }
      // Eat the close delim
      pszText++;
      
      // Swallow any space
      eat_space();
      
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
        attrs.push_back(attr(name, value));
        DUMP << "Parsed attr " << name << "=" << value << ENDL;
      }
      
      return true;
    } 
    
    return false;
  }
  
  // Parse "<TAG>", ignores leading whitespace
  // https://www.w3.org/TR/REC-xml/#sec-starttags
  // No space allowed between < and tag name
  constexpr vec<attr, 32> parse_open_tag()
  {
    DUMP << "Parsing open tag ..." << ENDL;
    DUMP << "Node size = " << nodes.size() << ENDL;
    
    // Left trim whitespace
    eat_space();
    check_eos();
    
    // Try to parse the "<"
    if(!eat_str("<")) 
    {
      PARSE_ERR("Missing <");
    }
    
    // Try to parse an [a-z]+ as a tag
    char_view sym = eat_alpha(is_alpha);

    // add a node 
    nodes.push_back(cnode(sym));
    cnode &node = nodes.back();
    
    // Eat any trailing whitespace
    eat_space();
    
    // Check if valid tag
    const int nTags = sizeof(arrTags)/sizeof(arrTags[0]);
    if(find_arr(arrTags, nTags, sym.m_pBeg) == -1)
    {
      PARSE_ERR("Unknown tag name");
    }
    
    // Parse attributes
    vec<attr, 32> attrs, styles;
    while(parse_attrs(attrs));
    
    // Check if void tag
    const int nVoidTags = sizeof(arrVoidTags)/sizeof(arrVoidTags[0]);
    const int idx = find_arr(arrVoidTags, nVoidTags, node.tag.m_pBeg);
    
    if(idx != -1)
    {
      // Void tag, add the "/" too
      eat_space();
      
      // Bump up the symbol end, so the parser can check for a trailing / and not look for a close tag
      node.tag.m_pEnd = pszText + 1;
      
      if(!eat_str("/")) 
      {
        PARSE_ERR("Missing / on a void tag");
      }
    }

    // Grab the final >
    if(!eat_str(">")) 
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
  constexpr void parse_close_tag(const char_view &symExpected)
  {
    // Try to parse the "</"
    if(!eat_str("</")) 
    {
      PARSE_ERR("Expecting a close tag");
    }
    
    // Try to parse the tag name
    char_view sym = eat_alpha(is_alpha);
    if(sym != symExpected) 
    {
      PARSE_ERR("Mismatched Close Tag");
    }
    
    DUMP << "Close " << sym << ENDL;
    
    // Ignore space, parse >
    eat_space();
    if(!eat_str(">")) 
    {
      PARSE_ERR("Missing > in close tag");
    }
    
    // Eat trailing space
    eat_space();
  }

  // Parses all text content until a "<" is hit 
  constexpr void parse_tag_content(int iParentIDX, bool bTrim)
  {
    // make sure we have something
    check_eos();
    
    // Try to parse until a "<", forbid & and >
    bool contentUnexpectedChars[256] = {0};
    contentUnexpectedChars[int('>')] = true;
    contentUnexpectedChars[int('&')] = true;
    nodes[iParentIDX].text = eat_until('<', contentUnexpectedChars);

    if(bTrim)
    {
      nodes[iParentIDX].text.trim();
    }
    
    // Make sure we have something left
    check_eos();
  }
  
};

// Runtime tree node
class rnode
{
  friend struct tree;
  
  typedef map<string, string> attr_dict; 
  
  // children if any
  vector<rnode> children;
  
  // attributes of this node
  attr_dict attrs;
  
  // node tag, content text and id
  char_view tag, text, id;
  
  // If content has template tags of the form {{key}}, store them in this
  template_text templates;
  
  int index;
  
public:
  rnode(): index(-1){}

  rnode(const char_view &tag, const char_view &text, int index, template_dict &dctTemplates) 
    : tag(tag), text(text), index(index) 
  {
    // Iterate through the text and detect if we have a template strings
    const char *szOpen = "{{";
    const char *szClose = "}}";
    
    auto itCurr = text.begin();
    do
    {
      // Find a "{{"
      auto itStart = std::search(itCurr, text.end(), szOpen, szOpen + 2);
      
      // The part from itCurr to itStart is a non template chunk iff itCurr != itStart
      if(itCurr != itStart)
      {
        templates.add(char_view(itCurr, itStart), false);
        itCurr = itStart;
      }
      
      // Now either itStart is at {{ or its text.end()
      if(itStart != text.end())
      {
        // find the closing }}, if found add this chunk as a template
        auto itEnd = std::search(itStart, text.end(), szClose, szClose + 2);
        if(itEnd != text.end())
        {
          // Check if non empty tag
          if((itEnd - itStart) > 2)
          {
            char_view sKey(itStart + 2, itEnd);
            templates.add(sKey, true);
            dctTemplates[string(sKey.begin(), sKey.end())] = "";
          }
          else
          {
            cerr << "Found an empty template tag {{}}";
          }
          
          // Set the current pointer beyond the template }}
          itCurr = itEnd + 2;
        }
        else  
        {
          // Set the current pointer to the end;
          itCurr = text.end();
        }
      }
    } 
    while(itCurr != text.end());
  }
  
  
  void dump(ostream &ostr, const template_dict &dctTemplates, int indent = 0) const
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
      child.dump(ostr, dctTemplates, indent + 1);
    }
    
    // Skip text and close tag for void tags
    if(tag.back() != '/')
    {
      if(templates.parts().size())
      {
        ostr << "\n" << sIndent;
        templates.render(ostr, dctTemplates);
        ostr << "\n";
      }
      ostr << sIndent << "</" << tag << ">" << "\n";
    }
    else
    {
      ostr << "\n";
    }
  }
  
private:
};

// Encapsulates the runtime DOM tree including templates
struct tree
{
  rnode root;
  template_dict templates;
  
  // Takes the compile time parser data and constructs thr runtime node tree 
  // Also generates a map for templates 
  tree(const parser &parser): root("HTML", "", -1, templates)
  {
    build(parser, root, templates, 0);
  }
  
  // Recursively builds the runtime tree structure from the compile time parser
  // Detects strings of the form {{key}} inside node content and adds it to a template_dict
  static void build(const parser &parser, rnode &parent, template_dict &dctTemplates, int index)
  {
    // Get the node tag and content
    const cnode &cNode = parser.nodes[index];
    
    // Create a SPTNode and set ID if any
    rnode rNode(cNode.tag, cNode.text, index, dctTemplates);
    if(cNode.id.size())
    {
      rNode.id = cNode.id;
    }
    
    // Place this node as a child of the parent
    parent.children.emplace_back(rNode);
    
    // If there are children for this node
    if(cNode.child > -1)
    {
      // Check if first child is "@ATTR"
      const auto &child = parser.nodes[cNode.child];
      if(child.tag == g_symAttr)
      {
        // Put the chain of attribute nodes into ther attrs array
        auto attr = parser.nodes[child.child];
        while(1)
        {
          parent.children.back().attrs[attr.getTag()] = attr.getText();
          if(attr.sibling == -1) break;
          attr = parser.nodes[attr.sibling];
        }
        
        // If there were more nodes after @ATTR, recursively process them
        if(child.sibling > -1)
        {
          build(parser, parent.children.back(), dctTemplates, child.sibling);
        }
      }
      else // No @ATTR
      {
        // Process children 
        build(parser, parent.children.back(), dctTemplates, cNode.child);
      }
    }
    
    // Process siblings
    if(cNode.sibling > -1)
    {
      build(parser, parent, dctTemplates, cNode.sibling);
    }
  }
};

} // namespace spt

constexpr spt::parser operator"" _html(const char *pszText, size_t)
{
  spt::parser parser(pszText);
  parser.parse_html(-1);
  return parser;
}

