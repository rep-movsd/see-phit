#ifndef SEEPHIT_SEEPHIT_H
#define SEEPHIT_SEEPHIT_H

#pragma once
#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <string>
#include <utility>
#include <vector>

using std::string;
using std::vector;
using std::pair;
using std::unordered_map;

using std::ostream;
using std::cerr;
using std::endl;

#include "debug.h"
#include "parse_error.h"
#include "tags.hpp"
#include "util.h"

// maximum nodes and attributes in the tree
#define SPT_MAX_NODES 2048
#define SPT_MAX_ATTRS 2048
#define SPT_MAX_WARNINGS 20
#define SPT_MAX_ATTR_PER_NODE 16

namespace spt
{

using attrs = vec<attr, SPT_MAX_ATTRS>;
using cnodes = vec<cnode, SPT_MAX_NODES>;
using node_attrs = vec<attr, SPT_MAX_ATTR_PER_NODE>;
using warnings = vec<Message, SPT_MAX_WARNINGS>;

// Hardcoded symbols to detect id and style
constexpr const char_view g_symID{"id"};
constexpr const char_view g_symStyle{"style"};
constexpr const char_view g_symPre{"pre"};

// These two tags are used internally to handle bare text and attributes
constexpr const char_view g_symText{"@text"};
constexpr const char_view g_symAttr{"@attr"};

// Compile time parser
struct parser
{
  cnodes nodes;
  sym_tab ids;
  warnings warns;
  Messages err {};
  
  int errRow = -1;
  int errCol = -1;
  
  // Macro to quit the current function if error
  #define ON_ERR_RETURN if(errRow > -1) return
    
  constexpr explicit parser(const char *pszText): pszText(pszText), pszStart(pszText) {}

  // Parse grammar
  // HTML     :: CONTENT | CONTENT HTML
  // CONTENT  :: TEXT | TAG
  // TAG      :: OPENTAG HTML CLOSETAG
  // OPENTAG  :: "<" TAGNAME ">"
  // CLOSETAG :: "<" TAGNAME "/>"
  // TEXT     :: [~>&]+
  // symEndTag represents the point at which the parsing should stop
  constexpr void parse_html(int iParentId)
  {
    while(errRow ==-1 && parse_content(iParentId));
  }
      
  void dump() const 
  {
    int i = 0;
    for(const auto &node: nodes)
    {
      cerr << "n=" << i++ << ",";
      node.dump();
      cerr << endl;
    }
  }
 
private:

  // Helper for error handling construct
  // Use as follows:
  // WITH_SAVE_POS { ... }
  struct saver
  {
    const char *saved = nullptr;
    bool done = true;
    constexpr explicit saver(const char *saved): saved(saved) {}
    constexpr const char *finish() {done = false; return saved;}
  };
  
  // Macro to execute a block of code, saving and restoring the a value across it
  #define WITH_SAVE_POS for(saver s(pszText); s.done; pszText = s.finish())
 

  const char *pszText = nullptr;  // Position in the stream
  const char *pszStart = nullptr;
  
  // Return line number of current poistion
  constexpr int cur_row() const
  {
    // Count the number of \n
    int n = 0;
    auto p = pszStart;
    while(p != pszText)
    {
      if(*p++ == '\n') ++n;
    }
    return n + 1;
  }
  
  // Return column number of current poistion
  constexpr int cur_col() const
  {
    // Count the number of chars to reach \n or beginning
    int n = 0;
    auto p = pszText;
    while(p != pszStart && *p-- != '\n')
    {
      ++n;
    }
    return n;
  }
  
  // Raises compiletime error if no more characters left to parse
  constexpr void check_eos()
  {
    bool eos = *pszText == 0;
    if(eos) 
    {
      PARSE_ERR(Error_Unexpected_end_of_stream);
    }
  }
  
  // Advances to first non-whitespace character
  // For simplicity assume anything below ascii 33 is white space
  constexpr void eat_space()
  {
    check_eos();
    while(*pszText && *pszText <= 32) { ++pszText; }
  }
  
  // Consumes [a-z]+
  template<typename F> constexpr const char_view eat_alpha(F isX)
  {
    // Ensure not EOS
    check_eos();
    
    char_view sym(pszText, pszText);
    while(isX(*sym.m_pEnd)) { sym.m_pEnd++;}
    
    // Ensure at least 1 character is consumed
    if(sym.empty()) 
    {
      PARSE_ERR(Error_Expecting_an_identifier);
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
  
  
  // Consume stuff until ch is encountered
  constexpr const char_view eat_until(char ch, const bool *unExpected)
  {
    char_view sym(pszText, pszText);
    while(*sym.m_pEnd && *sym.m_pEnd != ch) 
    {
      if(unExpected && unExpected[static_cast<unsigned char>(*sym.m_pEnd)])
      {
        for(saver s(pszText); s.done; pszText = s.finish())
        {
          pszText = sym.m_pEnd;
          PARSE_ERR(Error_Unexpected_character_inside_tag_content);
        }
        break;
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
    if(pszText[0] == '<')
    {
      if(is_alpha(pszText[1])) 
      {
        pszText = saved;
        return true;
      }
      
      if(pszText[1] != '/') 
      {
        pszText++;
        PARSE_ERR(Error_Expecting_a_tag_name_after_open_bracket);
      }
    }
    
    pszText = saved;
    return false;
  }
  
  // Checks if we have a close tag at state.text, returns the tag symbol
  constexpr bool is_close_tag()
  {
    // Save the pointer, eat whitespace
    WITH_SAVE_POS
    {
      eat_space();
      
      if(pszText[0] == '<' && pszText[1] == '/')
      {
        return is_alpha(pszText[2]);
      }
    }
    return false;
  }
  
  // Parses one attribute like NAME=VALUE
  // NAME is a sequence of [a-z\-] and VALUE is "text", 'text' or {text}
  constexpr bool parse_attrs(node_attrs &attrs)
  {
    if(errRow > -1) return false;
    
    // Swallow any space
    eat_space();
    
    if(is_alpha(*pszText))
    { 
      // Get the attr name
      const char_view &name = eat_alpha(is_attr); 

      bool bHasEqual = eat_str("=");
      
      if(bHasEqual)
      {
        // Check what delimiter is used " or ' 
        char chDelim = pszText[0];
        if(chDelim != '"' && chDelim != '\'')
        {
          PARSE_ERR(Error_Expecting_open_quote_for_attribute_value);
        }
        ++pszText;
        
        char_view value = eat_until(chDelim, nullptr);
        
        if(value.empty())
        {
          PARSE_ERR(Error_Empty_value_for_non_boolean_attribute);
        }
        
        check_eos();
        if(errRow > -1) return false;

        // Eat the close delim
        pszText++;
        
        
        // Swallow any space
        eat_space();
                
        
        // Is it an ID tag
        if(name == g_symID)
        {
          if(!ids.addSym(value))
          {
            // Save the pointer, point it to the start of symbol, for the warning
            WITH_SAVE_POS
            {
              pszText = value.begin();
              PARSE_WARN(Error_Duplicate_ID_on_tag);
            }
          }
          
          nodes.back().id = value;
        }
        else
        {
          attrs.push_back(attr(name, value));
          //DUMP << "Parsed attr " << name << "=" << value << ENDL;
        }
        
      }
      else
      {
        const int nAttrs = sizeof(arrBoolAttrs)/sizeof(arrBoolAttrs[0]);
        if(find_arr(arrBoolAttrs, nAttrs, name.m_pBeg) == -1)
        {
          PARSE_ERR(Error_Expecting_a_value_for_attribute);
        }
        
        attrs.push_back(attr(name, name));
      }
      
      return true;
    } 
    
    return false;
  }
  
  // Parse "<TAG>", ignores leading whitespace
  // https://www.w3.org/TR/REC-xml/#sec-starttags
  // No space allowed between < and tag name
  constexpr bool parse_open_tag(node_attrs &attrs)
  {
    ON_ERR_RETURN false;
    
    // Left trim whitespace
    eat_space();
    check_eos();
    
    // Try to parse the "<"
    if(!eat_str("<")) 
    {
      PARSE_ERR(Error_Missing_open_bracket);
    }
    
    // Try to parse an [a-z0-9]+ as a tag - 
    // is_open_tag would have already ensure first char is [a-z]
    char_view sym = eat_alpha(is_alnum);

    DUMP << "Parsed open tag: " << sym << ENDL;
    
    // add a node 
    nodes.push_back(cnode(sym));
    cnode &node = nodes.back();
    
    // Eat any trailing whitespace
    eat_space();
    
    // Check if valid tag
    const int nTags = sizeof(arrTags)/sizeof(arrTags[0]);
    if(find_arr(arrTags, nTags, sym.m_pBeg) == -1)
    {
      WITH_SAVE_POS
      {
        pszText = sym.m_pBeg;
        PARSE_WARN(Error_Unknown_tag_name);
      }
    }
    
    // Parse attributes
    while(parse_attrs(attrs));
    ON_ERR_RETURN false;
    
    // Check if void tag
    const int nVoidTags = sizeof(arrVoidTags)/sizeof(arrVoidTags[0]);
    bool bIsVoidTag = find_arr(arrVoidTags, nVoidTags, node.tag.m_pBeg) != -1;
    
    if(bIsVoidTag)
    {
      // Void tag, optionally eat the "/" too
      eat_space();
      eat_str("/");
    }

    // Grab the final >
    if(!eat_str(">")) 
    {
      if(bIsVoidTag)
      {
        PARSE_ERR(Error_Missing_close_bracket_on_void_tag);
      }
      else
      {
        PARSE_ERR(Error_Missing_close_bracket_on_open_tag);
      }
    }
    
    return bIsVoidTag;
  }
  
  // Attempts to parse "</TAG>" given "TAG"
  constexpr void parse_close_tag(const char_view &symExpected)
  {
    if(errRow > -1) return;
    
    eat_space();
    
    // Try to parse the "</"
    if(!eat_str("</")) 
    {
      PARSE_ERR(Error_Expecting_a_close_tag);
    }
    
    // Try to parse the tag name
    char_view sym = eat_alpha(is_alnum);
    if(sym != symExpected) 
    {
      DUMP << "Expected '" << symExpected << "' got '" << sym << "'" << ENDL;
      WITH_SAVE_POS
      {
        pszText = sym.begin();
        PARSE_ERR(Error_Mismatched_Close_Tag);
      }
    }
    
    // Ignore space, parse >
    eat_space();
    if(!eat_str(">")) 
    {
      PARSE_ERR(Error_Missing_close_bracket_in_close_tag);
    }
    
    // Eat trailing space
    eat_space();
  }

  // Creates a node "@attr" under the given node and chains attributes under it if any
  constexpr void append_attrs(cnode &node, node_attrs &attrs)
  {
    if(errRow > -1) return;
    
    // If there are any attributes, they become the first children of this node
    if(attrs.size())
    {
      // Create a "attr" node, make it the child of this
      nodes.push_back(cnode(g_symAttr));
      cnode &nodeAttrs = nodes.back();
      node.child = nodes.size() - 1;
      
      // Add the first attribute as the child of the "attr" node
      nodes.push_back(cnode(attrs[0].name, attrs[0].value));
      int iYoungest = nodeAttrs.child = nodes.size() - 1;
      
      // Add the rest by chaining as siblings
      for(size_t i = 1; i < attrs.size(); ++i)
      {
        nodes.push_back(cnode(attrs[i].name, attrs[i].value));
        nodes[iYoungest].sibling = nodes.size() - 1;
        iYoungest = nodes.size() - 1;
      }
    }
  }
  
  // TAG :: OPENTAG HTML CLOSETAG
  constexpr int parse_tag()
  {
    ON_ERR_RETURN 0;
    
    // Parse the open tag, get its index
    int iCurrId = nodes.size();
    
    node_attrs attrs;
    bool bIsVoidTag = parse_open_tag(attrs);
    cnode &node = nodes[iCurrId];
    append_attrs(node, attrs);
    
    if(!bIsVoidTag)
    {
      // Now we parse recursively
      parse_html(iCurrId);
      
      // Finally parse the close tag
      parse_close_tag(node.tag);
    }
    else
    {
      node.child = VOID_TAG;
    }
    
    return iCurrId;
  }
  
  // Parse text until a <, forbidding & and >, optionally trims whitespace on bothe ends
  constexpr int parse_text(bool bTrim)
  {
    if(errRow > -1) return 0;
    
    // make sure we have something
    check_eos();
    bool contentUnexpectedChars[256] = {false};
    contentUnexpectedChars[int('>')] = true;
    auto text = eat_until('<', contentUnexpectedChars);
    
    // Make sure we have something left
    check_eos();
        
    // check if we have a '{{'
    int nBrace = 0;
    auto p = text.begin();
    
    while(p != text.end())
    {
      if(p[0] == '{' && p[1] == '{')
      {
        ++nBrace;
      }
      else
      {
        if(nBrace)
        {
          if(p[0] == '}' && p[1] == '}')
          {
            --nBrace;
          }
        }
      }
      
      ++p;
    }
    
    if(nBrace)
    {
      PARSE_ERR(Error_Missing_close_brace_in_template);
    }
    
    // Trim whitespace if needed
    if(bTrim)     
    { 
      text.trim();
    }
    
    // Add a text meta node and return its index
    nodes.push_back(cnode(g_symText, text));
    
    return nodes.size() - 1;
  }
  
  // CONTENT  :: TEXT | TAG
  constexpr const bool parse_content(int iParentId)
  {
    // If we are out of text, were done
    // Else if we found a close tag, were done
    if(*pszText && !is_close_tag() && errRow == -1)
    {
      // Parse either an open tag or text, get the new child nodes ID
      int iChild = -1;
      if(is_open_tag())
      {
        if(errRow > -1) return false;
        iChild = parse_tag();
      }
      else
      {
        if(errRow > -1) return false;
        // Trim the text unless the parent node is a <pre>
        iChild = parse_text(nodes[iParentId].tag != g_symPre);
      }
      
      // If it's not the topmost level
      if(iParentId >= 0)
      {
        // Does parent have a child?
        if(nodes[iParentId].child != -1)
        {
          // Walk down the sibling chain to get the last child
          int iYoungest = nodes[iParentId].child;
          
          while(nodes[iYoungest].sibling != -1)
          {
            iYoungest = nodes[iYoungest].sibling;
          }
          
          // Assign us as the last sibling 
          nodes[iYoungest].sibling = iChild;
        }
        else 
        {
          // Assign the parents "firstborn" to us
          nodes[iParentId].child = iChild;
        }
      }
      
      return true;
    }
    
    return false;
  }
  
};

// Runtime tree node
class rnode
{
  friend struct tree;
  
  using attr_dict = unordered_map<string, string>; 
  
  // children if any
  vector<rnode> children;
  
  // attributes of this node
  attr_dict attrs;
  
  // node tag, content text and id
  char_view tag, text, id;
  
  // If content has template tags of the form {{key}}, store them in this
  template_text templates;
  
  // Whether its a void node
  bool bVoid{};
  
  int index;
  
public:
  rnode():  index(NULL_NODE) {}

  rnode(const char_view &tag, const char_view &text, bool bVoid, int index, template_dict &dctTemplates) 
    : tag(tag), text(text), bVoid(bVoid), index(index) 
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
  
  
  void render(ostream &ostr, const template_dict &dctTemplates, int indent = 0) const
  {
    string sIndent = string(indent * 2, ' ');
    bool bTextNode = tag == g_symText;
    
    if(!bTextNode)
    {
      ostr << sIndent << "<" << tag;
      
      if(!id.empty())
      {
        ostr << " ID" << "='" << id << '\'';
      }
      
      for(const auto &attr: attrs)
      {
        ostr << ' ' << attr.first << '=' << '\'' << attr.second << '\'';
      }
      
      ostr << ">";
      
      if(!children.empty()) 
      {
        ostr << "\n";
      }
      
      for(const auto& child: children)
      {
        child.render(ostr, dctTemplates, indent + 1);
      }
    }
    
    // Skip text and close tag for void tags
    if(!bVoid)
    {
      if(!templates.parts().empty())
      {
        ostr << sIndent;
        templates.render(ostr, dctTemplates);
        ostr << "\n";
      }
      
      if(!bTextNode)
      {  
        ostr << sIndent << "</" << tag << ">" << "\n";
      }
    }
    else
    {
      if(!bTextNode)
      {
        ostr << "\n";
      }
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
  tree(const parser &parser): root("HTML", "", false, -1, templates)
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
    rnode rNode(cNode.tag, cNode.text, cNode.child == VOID_TAG, index, dctTemplates);
    if(!cNode.id.empty())
    {
      rNode.id = cNode.id;
    }
    
    // Place this node as a child of the parent
    parent.children.emplace_back(rNode);
    
    // If there are children for this node
    if(cNode.child > NULL_NODE)
    {
      // Check if first child is "@ATTR"
      const auto &child = parser.nodes[cNode.child];
      if(child.tag == g_symAttr)
      {
        // Put the chain of attribute nodes into ther attrs array
        auto attr = parser.nodes[child.child];
        while(true)
        {
          parent.children.back().attrs[attr.getTag()] = attr.getText();
          if(attr.sibling == NULL_NODE) break;
          attr = parser.nodes[attr.sibling];
        }
        
        // If there were more nodes after @ATTR, recursively process them
        if(child.sibling > NULL_NODE)
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
    if(cNode.sibling > NULL_NODE)
    {
      build(parser, parent, dctTemplates, cNode.sibling);
    }
  }
};


} // namespace spt

constexpr spt::parser operator"" _html(const char *pszText, size_t /*unused*/)
{
  spt::parser parser(pszText);
  parser.parse_html(spt::NULL_NODE);
  return parser;
}





#endif
