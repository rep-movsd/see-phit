# see-phit

See-phit is a compile time HTML templating library written in modern C++. 

You write plain HTML as C++ string literals and it is parsed at compile time into a DOM like data structure.

It makes your "stringly typed" HTML text into an actual strongly typed DSL. 

C++14 is required to compile - it implements a fairly complete HTML parser as constexpr functions.
Before constexpr, the way to make C++ DSLs was by (ab)using operator overloading in ingenious ways, but now we can actually have a DSL as a user literal string and let the compiler compile your DSL into C++
 

Example:
``` cpp
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
  
  spt_tree.root.render(cerr, dct);
  cerr << endl;
  
  dct["city"] = "New York";
  dct["name"] = "John";
  dct["profession"] = "janitor";

  spt_tree.root.render(cerr, dct);
  cerr << endl;
}
```

produces the following output
``` html
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
```
The program will fail to compile if the HTML is malformed - We attempt to make the compiler generate the most sensible error message: 

For example, the following fragment:
    
``` html
<DIV>
This is a bad closing tag
</DIVV>
```

Generates the following compile errors in gcc:

```
$ g++ --std=c++14  -Wall main.cpp
In file included from seephit.h:21:0,
                 from main.cpp:3:
parse_error.h: In instantiation of 'constexpr spt::Error<ROW, COL, WHAT>::Error() [with int ROW = 4; int COL = 3; WHAT = spt::Mismatched_Close_Tag]':
main.cpp:13:3:   required from here
parse_error.h:40:15: error: incompatible types in assignment of 'const int' to 'char [0]'
     SPTParser = fatal;
     ~~~~~~~~~~^~~~~~~
```
    
And the following in clang:

```
$ clang++ --std=c++14 -Wall main.cpp
In file included from main.cpp:3:
In file included from ./seephit.h:21:
./parse_error.h:40:15: error: array type 'char [0]' is not assignable
    SPTParser = fatal;
    ~~~~~~~~~ ^
main.cpp:13:3: note: in instantiation of member function 'spt::Error<4, 3, spt::Mismatched_Close_Tag>::Error' requested here
  REPORT_ERRORS(parser);
  ^
./parse_error_generated.h:100:94: note: expanded from macro 'REPORT_ERRORS'
spt::IF<hasErr, spt::Error<parser.errRow, parser.errCol, spt::MsgToType<parser.err>::type>> {};
                                                                                             ^
1 error generated.
```

Some complicated template magic has been implemented to show the ROW and COLUMN in the text where the error occured.
gcc actually prints ROW = xxx and COL = xxx, which is great!
If your IDE does background parsing, it will indicate that your HTML template is malformed as you type it.

### Limitations
The number of maximum nodes and attributes per parse is hardcoded to 1024.

### Future plans
Add more complicated templating functionality with loops, conditionals and perhaps lambdas, and also allow this to be used on the frontend JS with emscripten.

Optimize the hell out of the templating engine

 
