#ifndef SEEPHIT_SEEPHIT_H
#define SEEPHIT_SEEPHIT_H

#include "pch.h"
#include "debug.h"
#include "parse_error.h"
#include "tags.h"
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

// Control structures
constexpr const char_view g_symFor{"for"};
constexpr const char_view g_symIf{"if"};
constexpr const char_view g_symRoot{"root"};

// These two tags are used internally to handle bare text and attributes
constexpr const char_view g_symText{"@text"};
constexpr const char_view g_symAttr{"@attr"};

// Compile time parser
struct parser
{
  cnodes m_arrNodes;
  sym_tab m_ids;
  warnings m_arrWarns;
  Messages m_arrErrs {};
  
  int m_iErrRow = -1;
  int m_iErrCol = -1;
  
  // Index of the first parentless top level node
  // We need this to chain the top level parentless siblings together 
  int m_iElder = -1;
  
  // Macro to quit the current function if error
  #define ON_ERR_RETURN if(m_iErrRow > -1) return
    
  constexpr explicit parser(const char *pszText): m_pszText(pszText), m_pszStart(pszText) {}

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
    while(m_iErrRow == -1 && parse_content(iParentId));
  }
      
  // Dumps the tree nodes linearly
  void dump() const 
  {
    int i = 0;
    for(const auto &node: m_arrNodes )
    {
      cerr << "n=" << i++ << ",";
      node.dump();
      cerr << endl;
    }
  }
 
private:

  // Helper for error handling construct
  struct saver
  {
    const char *saved = nullptr;
    bool done = true;
    constexpr explicit saver(const char *saved): saved(saved) {}
    constexpr const char *finish() {done = false; return saved;}
  };
  
  // Macro to execute a block of code, saving and restoring the a value across it
  // Use as follows: WITH_SAVE_POS { ... }
  #define WITH_SAVE_POS for(saver s(m_pszText); s.done; m_pszText = s.finish())
  #define INDEX_OF(ELEM, ARR) find_arr(ARR, (sizeof(ARR) / sizeof(ARR[0])), ELEM) 
  
  const char *m_pszText = nullptr;  // Position in the stream
  const char *m_pszStart = nullptr;
  
  // Return line number of current position
  constexpr int cur_row() const
  {
    // Count the number of newlines
    int n = 0;
    for(auto p = m_pszStart; p != m_pszText; ++p)
    {
      if(*p == '\n') ++n;
    }
    return n + 1;
  }
  
  // Return column number of current position
  constexpr int cur_col() const
  {
    // Count the number of chars to reach \n or beginning
    int n = 0;
    for(auto p = m_pszText; p != m_pszStart && *p-- != '\n'; ++n);
    return n;
  }
  
  // Raises compiletime error if no more characters left to parse
  constexpr void check_eos()
  {
    if(*m_pszText == 0) PARSE_ERR(Error_Unexpected_end_of_stream);
  }
  
  // Advances to first non-whitespace character
  // For simplicity assume anything below ascii 33 is white space
  constexpr void eat_space()
  {
    check_eos();
    while(*m_pszText && *m_pszText <= 32) ++m_pszText;
  }
  
  // Consumes characters matched by isX
  template<typename F> constexpr const char_view eat_only(F isX)
  {
    // Ensure not EOS
    check_eos();
    
    // Create a symbol starting here
    char_view sym{m_pszText, m_pszText};
    
    // Keep accumulating chars into the symbol until test fails
    while(isX(*sym.m_pEnd)) sym.m_pEnd++;
    
    // Ensure at least 1 character is consumed
    if(sym.empty()) PARSE_ERR(Error_Expecting_an_identifier);
    
    // Move current pos to end of symbol
    m_pszText = sym.m_pEnd;
    return sym;
  }
  
  // Tries to consume the string pszSym
  constexpr const bool eat_str(const char *pszSym)
  {
    // As long as we dont hit the end
    while(*pszSym)
    {
      // Any mismatch is a parse error
      if(*pszSym != *m_pszText ) return false;
      
      ++pszSym;
      ++m_pszText;
      
      // Ensure we are not at end of stream before the symbol has been compared fully
      if(*pszSym) check_eos();
    }
    
    return true;
  }
 
  // Consume stuff until ch is encountered
  constexpr const char_view eat_until(char chDelim, const bool *unExpected)
  {
    char_view sym( m_pszText, m_pszText );
    
    // Keep iterating until we hit the end of the symbol or the delimiting char
    while(*sym.m_pEnd && *sym.m_pEnd != chDelim ) 
    {
      // If we hit an unexpected char, throw an error
      if(unExpected && unExpected[static_cast<unsigned char>(*sym.m_pEnd)])
      {
        WITH_SAVE_POS
        {
          m_pszText = sym.m_pEnd;
          PARSE_ERR(Error_Unexpected_character_inside_tag_content);
        }
        break;
      }
      sym.m_pEnd++;
    }
    
    // Point current pos at end of symbol
    m_pszText = sym.m_pEnd;
    return sym;
  }
  
  // Checks if we have an open tag
  constexpr bool is_open_tag()
  {
    auto saved = m_pszText;
    eat_space();
    
    // Look for open tag and an alphanumeric character
    if( m_pszText[0] == '<')
    {
      if(is_alpha(m_pszText[1])) 
      {
        m_pszText = saved;
        return true;
      }
      
      // If its not an open tag, it has to be a close tag, else its invalid
      if(m_pszText[1] != '/') 
      {
        m_pszText++;
        PARSE_ERR(Error_Expecting_a_tag_name_after_open_bracket);
      }
    }
    
    m_pszText = saved;
    return false;
  }
  
  // Checks if we have a close tag, returns the tag symbol
  constexpr bool is_close_tag()
  {
    // Save the pointer, eat whitespace
    WITH_SAVE_POS
    {
      eat_space();
      
      // Check for "</" followed by alpha
      if(m_pszText[0] == '<' && m_pszText[1] == '/')
      {
        return is_alpha(m_pszText[2]);
      }
    }
    return false;
  }
  
  // Parses one attribute like NAME=VALUE
  // NAME is a sequence of [a-z\-] and VALUE is "text", 'text' or {text}
  constexpr bool parse_attrs(node_attrs &attrs)
  {
    ON_ERR_RETURN false;
    
    // Swallow any space
    eat_space();
    
    if(is_alpha(*m_pszText ))
    { 
      // Get the attr name
      const char_view &name = eat_only(is_attr); 

      bool bHasEqual = eat_str("=");
      if(bHasEqual)
      {
        char_view value;
        // Check what delimiter is used " or ' 
        char chDelim = m_pszText[0];
        if(chDelim == '"' || chDelim == '\'')
        {
          // Eat the open delim
          ++m_pszText;
          value = eat_until(chDelim, nullptr);
          
          check_eos();
          ON_ERR_RETURN false;
          
          // Eat the close delim
          m_pszText++;
        }
        else // no delimiter, stop at space
        {
          value = eat_only(is_attrval);
          eat_space();
        }                
        
        if(value.empty()) PARSE_ERR(Error_Empty_value_for_non_boolean_attribute);
        
        check_eos();
        ON_ERR_RETURN false;
        
        // Swallow any space
        eat_space();
        
        // Is it an ID tag
        if(name == g_symID)
        {
          // Verify that the ID has not been used before
          if(!m_ids.addSym(value))
          {
            // Save the pointer, point it to the start of symbol, for the warning
            WITH_SAVE_POS
            {
              m_pszText = value.begin();
              PARSE_WARN(Error_Duplicate_ID_on_tag);
            }
          }
          
          // Add the ID to the array of IDs
          m_arrNodes.back().id = value;
        }
        else // Regular attribute, accumulate it
        {
          attrs.push_back(attr(name, value));
          DUMP << "Parsed attr " << name << "=" << value << ENDL;
        }
      }
      else // No equal sign - test for boolean attributes
      {
        if(INDEX_OF(name.m_pBeg, g_arrBoolAttrs) == -1)
        {
          PARSE_ERR(Error_Expecting_a_value_for_attribute);
        }
        
        // Add the attribute to the list
        attrs.push_back(attr(name, name));
      }
      
      return true;
    } 
    
    return false;
  }
  
  // verifies an if tag 
  // <if cond={{var}}> <div> Stuff rendered if cond is non-zero </div> </if>  
  constexpr void check_if_tag(node_attrs &attrs)
  {
    int nAttr = attrs.size(); 
    if(nAttr < 1 || attrs[0].name != "cond")
    {
      PARSE_ERR(Error_Invalid_syntax_in_if_tag);
    }
  }
  
  // verifies a for tag 
  // <for var=name from=N to=N [inc=N]> ...
  constexpr void check_for_tag(node_attrs &attrs)
  {
    // Check that we have var, from, and to atrributes (inc is optional)
    int nAttr = attrs.size(); 
    bool bValid = attrs[0].name == "var" && attrs[1].name == "from" && attrs[2].name == "to";
    
    // Check also if the 4th attribute is "inc" if it exists
    if(!bValid || (nAttr > 3 && attrs[3].name != "inc"))
    {
      PARSE_ERR(Error_Invalid_syntax_in_for_tag);
    }
    else
    {
      // Verify that the for loop params are sane
      int iBeg = attrs[1].value.toInt();
      int iEnd = attrs[2].value.toInt();
      int iInc = nAttr == 4 ? attrs[3].value.toInt() : 1;
      if((iBeg > iEnd && iInc >= 0) || (iBeg < iEnd && iInc <= 0) || iBeg == iEnd)
      {
        PARSE_ERR(Error_Infinite_loop_in_for_tag);
      }
    }
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
    if(!eat_str("<")) PARSE_ERR(Error_Missing_open_bracket);
    
    // Try to parse an [a-z0-9]+ as a tag - 
    // is_open_tag would have already ensure first char is [a-z]
    char_view sym = eat_only(is_alnum);

    DUMP << "Parsed open tag: " << sym << ENDL;
    
    // add a node 
    m_arrNodes.push_back(cnode(sym));
    cnode &node = m_arrNodes.back();
    
    // Eat any trailing whitespace
    eat_space();
    
    // Check if valid tag
    if(INDEX_OF(sym.m_pBeg, g_arrCtrlTags) == -1 && INDEX_OF(sym.m_pBeg, g_arrTags) == -1)
    {
      WITH_SAVE_POS
      {
        m_pszText = sym.m_pBeg;
        PARSE_WARN(Error_Unknown_tag_name);
      }
    }
    
    // Parse all attributes
    while(parse_attrs(attrs));
    ON_ERR_RETURN false;
    
    // Check for control nodes, if and for, verify if they have the required attrs
    if(node.tag == g_symFor)
    {
      check_for_tag(attrs);
    }
    else if(node.tag == g_symIf)
    {
      check_if_tag(attrs);
    }
    
    // Check if void tag
    bool bIsVoidTag = INDEX_OF(node.tag.m_pBeg, arrVoidTags) != -1;
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
    ON_ERR_RETURN;
    eat_space();
    
    // Try to parse the "</"
    if(!eat_str("</")) 
    {
      PARSE_ERR(Error_Expecting_a_close_tag);
    }
    
    // Try to parse the tag name
    char_view sym = eat_only(is_alnum);
    if(sym != symExpected) 
    {
      DUMP << "Expected '" << symExpected << "' got '" << sym << "'" << ENDL;
      WITH_SAVE_POS
      {
        m_pszText = sym.begin();
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
    ON_ERR_RETURN;
    
    // If there are any attributes, they become the first children of this node
    if(attrs.size())
    {
      // Create a "attr" node, make it the child of this
      m_arrNodes.push_back(cnode(g_symAttr));
      cnode &nodeAttrs = m_arrNodes.back();
      node.child = m_arrNodes.size() - 1;
      
      // Add the first attribute as the child of the "attr" node
      m_arrNodes.push_back(cnode(attrs[0].name, attrs[0].value));
      int iYoungest = nodeAttrs.child = m_arrNodes.size() - 1;
      
      // Add the rest by chaining as siblings
      for(size_t i = 1; i < attrs.size(); ++i)
      {
        m_arrNodes.push_back(cnode(attrs[i].name, attrs[i].value));
        m_arrNodes[iYoungest].sibling = m_arrNodes.size() - 1;
        iYoungest = m_arrNodes.size() - 1;
      }
    }
  }
  
  // TAG :: OPENTAG HTML CLOSETAG
  constexpr int parse_tag()
  {
    ON_ERR_RETURN 0;
    
    // Parse the open tag, get its index
    int iCurrId = m_arrNodes.size();
    
    node_attrs attrs;
    bool bIsVoidTag = parse_open_tag(attrs);
    cnode &node = m_arrNodes[iCurrId];
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
  
  constexpr void check_template_braces(const char_view &text)
  {
    int nBrace = 0;
    auto p = text.begin();
    
    while(p != text.end())
    {
      // Check for {{
      if(p[0] == '{' && p[1] == '{')
      {
        ++nBrace;
      }
      else
      {
        // If we had one before check for a }}
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
    
    // If the counts mismatch, raise error
    if(nBrace)
    {
      PARSE_ERR(Error_Missing_close_brace_in_template);
    }
  }
  
  // Parse text until a <, forbidding & and >, optionally trims whitespace on bothe ends
  constexpr int parse_text(bool bTrim)
  {
    ON_ERR_RETURN 0;
    
    // make sure we have something
    check_eos();
    bool contentUnexpectedChars[256] = {false};
    contentUnexpectedChars[int('>')] = true;
    auto text = eat_until('<', contentUnexpectedChars);
    
    // Make sure we have something left
    check_eos();
        
    // Check if the braces are {{ matching }}
    check_template_braces(text);
    
    // Trim whitespace if needed
    if(bTrim) text.trim();
   
    // Add a text meta node and return its index
    m_arrNodes.push_back(cnode(g_symText, text));
    return m_arrNodes.size() - 1;
  }
  
  // CONTENT  :: TEXT | TAG
  constexpr const bool parse_content(int iParentId)
  {
    // If we are out of text, were done
    // Else if we found a close tag, were done
    if(*m_pszText && !is_close_tag() && m_iErrRow == -1)
    {
      // Parse either an open tag or text, get the new child nodes ID
      int iChild = -1;
      bool bIsOpenTag = is_open_tag();
      ON_ERR_RETURN false;
      
      if(bIsOpenTag)
      {
        iChild = parse_tag();
      }
      else
      {
        // Trim the text unless the parent node is a <pre>
        iChild = parse_text(m_arrNodes[iParentId].tag != g_symPre);
      }
      ON_ERR_RETURN false;
      
      // If it's not the topmost level
      if(iParentId >= 0)
      {
        // Does parent have a child?
        if(m_arrNodes[iParentId].child != -1)
        {
          // Walk down the sibling chain to get the last child
          int iYoungest = m_arrNodes[iParentId].child;
          while(m_arrNodes[iYoungest].sibling != -1)
          {
            iYoungest = m_arrNodes[iYoungest].sibling;
          }
          
          // Assign us as the last sibling 
          m_arrNodes[iYoungest].sibling = iChild;
        }
        else 
        {
          // Assign the parents "firstborn" to us
          m_arrNodes[iParentId].child = iChild;
        }
      }
      else // This is a top level node with no parent
      {
        // Do we know of an elder?
        if(m_iElder > -1)
        {
          // Set us to be the sibling of that elder
          m_arrNodes[m_iElder].sibling = iChild;
        }
        
        // We are the youngest elder
        m_iElder = iChild;
      }
      
      return true;
    }
    
    return false;
  }
};

// Runtime tree node
class rnode
{
  friend class tree;
  
  using attr_dict = unordered_map<string, string>; 
  
  // children if any
  vector<rnode> m_arrChildren;
  
  // attributes of this node
  attr_dict m_dctAttrs;
  
  // node tag, content text and id
  char_view m_symTag, m_symText, m_symId;
  
  // If content has template tags of the form {{key}}, store them in this
  template_text m_templates;
  
  // Whether it's a void node
  bool m_bVoidNode {};
  
  // Render the children of this node recursively
  void render_children(ostream &ostr, template_vals &dctVals, template_funs &dctFuns, int indent)
  {
    for(auto& child: m_arrChildren)
    {
      child.render(ostr, dctVals, dctFuns, indent);
    }
  }
  
  // Render the children in a for tag
  void render_for(ostream &ostr, template_vals &dctVals, template_funs &dctFuns, int indent)
  {
    // Get the for loop params
    auto iStart = std::stoi(m_dctAttrs.at("from"));
    auto iStop = std::stoi(m_dctAttrs.at("to"));
    auto iInc = m_dctAttrs.count("inc") ? std::stoi(m_dctAttrs.at("inc")) : 1;
    string sVar = m_dctAttrs.at("var");
    
    // Save the existing variable if any (allows nested loops with same var)
    bool bUsed = dctVals.count(sVar);
    template_val varSaved;
    if(bUsed) varSaved = dctVals.at(sVar);
    
    // Loop and render
    for(int i = iStart; iInc > 0 ? i < iStop : i > iStop; i += iInc)
    {
      dctVals[sVar] = i;
      render_children(ostr, dctVals, dctFuns, indent);
    }
    
    // Restore the loop var in the template dictionary or delete it if it didnt exits before
    if(bUsed)
    {
      dctVals[sVar] = varSaved;
    }
    else
    {
      dctVals.erase(sVar);
    }
  }
  
  // Render an if tag
  void render_if(ostream &ostr, template_vals &dctVals, template_funs &dctFuns, int indent)
  {
    if(std::stoi(m_dctAttrs.at("cond")))
    {
      render_children(ostr, dctVals, dctFuns, indent);
    }
  }
  
public:
  rnode() = default;

  rnode(const char_view &tag, const char_view &text, bool bVoidNode, int index) 
  : m_symTag(tag), m_symText(text), m_bVoidNode(bVoidNode) 
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
        m_templates.add(char_view(itCurr, itStart), false);
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
            m_templates.add(sKey, true);
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
  
  void render(ostream &ostr, template_vals &dctVals, template_funs &dctFuns, int indent = 0)
  {
    string sIndent(indent * 2, ' ');
    string sTag{m_symTag.m_pBeg, m_symTag.m_pEnd};

    bool bCtrlNode = false;
    bool bTextNode = m_symTag == g_symText;
    
    // Check if the tag is a control tag
    for(const char *p: g_arrCtrlTags)
    {
      if(p == sTag)
      {
        bCtrlNode = true;
        break;
      }
    }
    
    // For non text nodes we render tags
    if(!bTextNode)
    {
      // Ignore tag for control nodes
      if(!bCtrlNode)
      {
        // Render the open tag, and the ID if any
        ostr << sIndent << "<" << m_symTag;
        
        if(!m_symId.empty())
        {
          ostr << " ID" << "='" << m_symId << '\'';
        }
        
        // Render the attributes and close the >
        for(const auto &attr: m_dctAttrs )
        {
          ostr << ' ' << attr.first << '=' << '\'' << attr.second << '\'';
        }
        ostr << ">";
        
        // If tag has children add a newline
        if(!m_arrChildren.empty()) 
        {
          ostr << '\n';
          
          // Render children if any
          render_children(ostr, dctVals, dctFuns, indent + 1);
        }
      }
      else // control tags, do not indent
      {
        // Render children conditionally for if
        if(m_symTag == g_symIf)
        {
          render_if(ostr, dctVals, dctFuns, indent);
        }
        else if(m_symTag == g_symFor)
        {
          render_for(ostr, dctVals, dctFuns, indent);
        }
        else
        {
          render_children(ostr, dctVals, dctFuns, indent);
        }
      }
    }
    
    // Skip text and close tag for void tags and control tags
    if(!m_bVoidNode)
    {
      if(!m_templates.parts().empty())
      {
        ostr << sIndent;
        m_templates.render(ostr, dctVals, dctFuns);
        ostr << "\n";
      }
      
      if(!bTextNode && !bCtrlNode)
      {  
        ostr << sIndent << "</" << m_symTag << ">" << "\n";
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
  
};

// Encapsulates the runtime DOM tree including templates
class tree
{
private:
  rnode m_Root;

public:  
  template_vals m_dctTemplateVals;
  template_funs m_dctTemplateFuns;
  
  // Takes the compile time parser data and constructs thr runtime node tree 
  // Also generates a map for templates 
  tree(const parser &parser): m_Root ("root", "", false, -1)
  {
    build(parser, m_Root, 0);
  }
  
  // Test function that returns a map of all the keys with value == key
  template_vals get_default_dict()
  {
    template_vals ret;
    for(const auto &i: m_dctTemplateVals)
    {
      ret[i.first] = i.first;
    }
    return ret;
  }
    
  rnode root() const 
  {
    return m_Root;
  }
  
  // Recursively builds the runtime tree structure from the compile time parser
  // Detects strings of the form {{key}} inside node content and adds it to a template_dict
  static void build(const parser &parser, rnode &parent, int index)
  {
    // Get the node tag and content
    const cnode &cNode = parser.m_arrNodes[index];
    
    // Create a SPTNode and set ID if any
    rnode rNode(cNode.tag, cNode.text, cNode.child == VOID_TAG, index);
    if(!cNode.id.empty())
    {
      rNode.m_symId = cNode.id;
    }
    
    // Place this node as a child of the parent
    parent.m_arrChildren.emplace_back(rNode);
    
    // If there are children for this node
    if(cNode.child > NULL_NODE)
    {
      // Check if first child is "@ATTR"
      const auto &child = parser.m_arrNodes[cNode.child];
      if(child.tag == g_symAttr)
      {
        // Put the chain of attribute nodes into ther attrs array
        auto attr = parser.m_arrNodes[child.child];
        while(true)
        {
          parent.m_arrChildren.back().m_dctAttrs[attr.getTag()] = attr.getText();
          if(attr.sibling == NULL_NODE) break;
          attr = parser.m_arrNodes[attr.sibling];
        }
        
        // If there were more nodes after @ATTR, recursively process them
        if(child.sibling > NULL_NODE)
        {
          build(parser, parent.m_arrChildren.back(), child.sibling);
        }
      }
      else // No @ATTR
      {
        // Process children 
        build(parser, parent.m_arrChildren.back(), cNode.child);
      }
    }
    
    // Process siblings
    if(cNode.sibling > NULL_NODE)
    {
      build(parser, parent, cNode.sibling);
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
