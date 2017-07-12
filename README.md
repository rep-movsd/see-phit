# see-phit

See-phit is a compile time HTML templating library written in modern C++

You write plain HTML as C++ string literals and it is parsed at compile time into a DOM like data structure.

It makes your "stringly typed" HTML text into an actual strongly typed DSL

C++14 is required to compile

Example:

    #include <iostream>
    #include "seephit.h"

    int main()
    {
      constexpr auto nodes = 
      R"(
        <HTML>
          <DIV>
            <DIV> 
              Halloa
            </DIV>

            <DIV> 
              Guten tag
            </DIV>

            <P> Hi </P>
            </DIV>

          <DIV>
            <DIV> 
              Hell
            </DIV>
          </DIV>
        </HTML>    
      )"_html; 

      dumpNode(nodes, 0, 0);
    }
    
produces the following output

    <HTML>
      <DIV>
        Halloo
      </DIV>
      <DIV>
        Guten tag
      </DIV>
      <P>
        Hi
      </P>
    </HTML>
    <HTML>
      <DIV>
        Hell
      </DIV>
    </HTML>
    
The program will fail to compile if the HTML is malformed 


