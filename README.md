# see-phit

See-phit is a compile time HTML templating library written in modern C++. 

You write plain HTML as C++ string literals and it is parsed at compile time into a DOM like data structure.

It makes your "stringly typed" HTML text into an actual strongly typed DSL. 

C++14 is required to compile - it implements a fairly complete HTML parser as constexpr functions.
Before constexpr, the way to make C++ DSLs was by (ab)using operator overloading in ingenious ways, but now we can actually have a DSL as a user literal string and let the compiler compile your DSL into C++
 

Example:

    #include <iostream>
    #include "seephit.h"
    using namespace std;

    int main()
    {
      constexpr auto parser =
      R"*(
        <span >
        <p  color="red" height='10' >{{name}} is a {{profession}} in {{city}}</p  >
        </span>
        )*"_html;

      spt::tree spt_tree(parser);
      
      spt::template_dict dct;
      dct["name"] = "Mary";
      dct["profession"] = "doctor";
      dct["city"] = "London";
      
      spt_tree.root.dump(cerr, dct);
      cerr << endl;
      
      dct["city"] = "New York";
      dct["name"] = "John";
      dct["profession"] = "janitor";

      spt_tree.root.dump(cerr, dct);
      cerr << endl;
    } 
    
produces the following output

    <HTML>
      <span>
        <p COLOR='red' HEIGHT='10'>
        Mary is a doctor in London
        </p>
      </span>
    </HTML>

    <HTML>
      <span>
        <p COLOR='red' HEIGHT='10'>
        John is a janitor in New York
        </p>
      </span>
    </HTML>
    
The program will fail to compile if the HTML is malformed - We attempt to make the compiler generate the most sensible error message: 

For example, the following fragment:
    
    <DIV>
    This is a bad closing tag
    </DIVV>

Generates the compiler errors in gcc:

    $ g++ -Wall main.cpp
    main.cpp: In function 'int main()':
    main.cpp:8:3:   in constexpr expansion of 'operator""_html(const char*, size_t)(59)'
    seephit.h:716:20:   in constexpr expansion of '(&#'result_decl' not supported by dump_expr#<expression error>)->spt::parser::parse_html(-1)'
    seephit.h:126:26:   in constexpr expansion of '((spt::parser*)this)->spt::parser::parse_close_tag(symTag)'
    seephit.h:486:7:   in constexpr expansion of 'ParseError(((const char*)"Mismatched Close Tag")).ErrLine::operator[](((size_t)iLine))'
    main.cpp:8:3: error: array subscript value '3' is outside the bounds of array type 'int [0]'

And the following in clang:

    $ clang++ --std=c++14 main.cpp
    main.cpp:7:18: error: constexpr variable 'parser' must be initialized by a constant expression
      constexpr auto parser =
                    ^
    ./seephit_debug.h:29:48: note: cannot refer to element 3 of array of 0 elements in a constant expression
      constexpr int &operator[](size_t i) { return dummy[i];};
                                                  ^
    ./seephit.h:486:7: note: in call to '&ParseError("Mismatched Close Tag")->operator[](3)'
          PARSE_ERR("Mismatched Close Tag");
          ^
    ./seephit_debug.h:39:25: note: expanded from macro 'PARSE_ERR'
    int iLine = cur_line(); \
                            ^
    ./seephit.h:126:11: note: in call to '&parser->parse_close_tag(parser.nodes.m_Nodes[0].tag)'
              parse_close_tag(symTag);
              ^
    ./seephit.h:716:10: note: in call to '&parser->parse_html(-1)'
      parser.parse_html(-1);
            ^
    main.cpp:8:3: note: in call to 'operator""_html(&"\n    <DIV>\n    Mismatched closing tag\n    </DIVV>\n    \n    "[0], 59)'
      R"*(
      ^
    1 error generated.

The error "Mismatched Close Tag" is reported along with the '3' which represents the line number in which the error occured.
If your IDE does background parsing, it will indicate that your HTML template is malformed as you type it.

### Future plans
Add more complicated templating functionality with loops, conditionals and perhaps lambdas, and also allow this to be used on the frontend JS with emscripten.

Optimize the hell out of the templating engine


