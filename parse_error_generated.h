
enum Messages
{
  Error_None,
  Error_Expecting_an_identifier,
  Error_Unexpected_character_inside_tag_content,
  Error_Expecting_a_tag_name_after_open_bracket,
  Error_Expecting_open_quote_for_attribute_value,
  Error_Empty_value_for_non_boolean_attribute,
  Error_Duplicate_ID_on_tag,
  Error_Expecting_a_value_for_attribute,
  Error_Missing_open_bracket,
  Error_Unknown_tag_name,
  Error_Missing_close_bracket_on_void_tag,
  Error_Missing_close_bracket_on_open_tag,
  Error_Expecting_a_close_tag,
  Error_Mismatched_Close_Tag,
  Error_Missing_close_bracket_in_close_tag
};

struct None;
struct Expecting_an_identifier {};
struct Unexpected_character_inside_tag_content {};
struct Expecting_a_tag_name_after_open_bracket {};
struct Expecting_open_quote_for_attribute_value {};
struct Empty_value_for_non_boolean_attribute {};
struct Duplicate_ID_on_tag {};
struct Expecting_a_value_for_attribute {};
struct Missing_open_bracket {};
struct Unknown_tag_name {};
struct Missing_close_bracket_on_void_tag {};
struct Missing_close_bracket_on_open_tag {};
struct Expecting_a_close_tag {};
struct Mismatched_Close_Tag {};
struct Missing_close_bracket_in_close_tag {};

template<Messages m> struct MsgToType{};

template<> struct MsgToType<Error_None>{typedef None type;};
template<> struct MsgToType<Error_Expecting_an_identifier>{typedef Expecting_an_identifier type;}; 
template<> struct MsgToType<Error_Unexpected_character_inside_tag_content>{typedef Unexpected_character_inside_tag_content type;}; 
template<> struct MsgToType<Error_Expecting_a_tag_name_after_open_bracket>{typedef Expecting_a_tag_name_after_open_bracket type;}; 
template<> struct MsgToType<Error_Expecting_open_quote_for_attribute_value>{typedef Expecting_open_quote_for_attribute_value type;}; 
template<> struct MsgToType<Error_Empty_value_for_non_boolean_attribute>{typedef Empty_value_for_non_boolean_attribute type;}; 
template<> struct MsgToType<Error_Duplicate_ID_on_tag>{typedef Duplicate_ID_on_tag type;}; 
template<> struct MsgToType<Error_Expecting_a_value_for_attribute>{typedef Expecting_a_value_for_attribute type;}; 
template<> struct MsgToType<Error_Missing_open_bracket>{typedef Missing_open_bracket type;}; 
template<> struct MsgToType<Error_Unknown_tag_name>{typedef Unknown_tag_name type;}; 
template<> struct MsgToType<Error_Missing_close_bracket_on_void_tag>{typedef Missing_close_bracket_on_void_tag type;}; 
template<> struct MsgToType<Error_Missing_close_bracket_on_open_tag>{typedef Missing_close_bracket_on_open_tag type;}; 
template<> struct MsgToType<Error_Expecting_a_close_tag>{typedef Expecting_a_close_tag type;}; 
template<> struct MsgToType<Error_Mismatched_Close_Tag>{typedef Mismatched_Close_Tag type;}; 
template<> struct MsgToType<Error_Missing_close_bracket_in_close_tag>{typedef Missing_close_bracket_in_close_tag type;}; 

#ifndef SPT_DEBUG

#define DUMP_WARNING(x)                                        \
if(x < n)                                                      \
{                                                              \
  constexpr auto w = parser.warns[x];                          \
  spt::IF<w.m == spt::Error_Expecting_an_identifier, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Unexpected_character_inside_tag_content, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Expecting_a_tag_name_after_open_bracket, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Expecting_open_quote_for_attribute_value, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Empty_value_for_non_boolean_attribute, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Duplicate_ID_on_tag, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Expecting_a_value_for_attribute, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Missing_open_bracket, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Unknown_tag_name, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Missing_close_bracket_on_void_tag, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Missing_close_bracket_on_open_tag, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Expecting_a_close_tag, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Mismatched_Close_Tag, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Missing_close_bracket_in_close_tag, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
}

#define REPORT_ERRORS(parser)          \
constexpr int n = parser.warns.size();          \
DUMP_WARNING(0);                       \
DUMP_WARNING(1);                       \
DUMP_WARNING(2);                       \
DUMP_WARNING(3);                       \
DUMP_WARNING(4);                       \
DUMP_WARNING(5);                       \
DUMP_WARNING(6);                       \
DUMP_WARNING(7);                       \
DUMP_WARNING(8);                       \
DUMP_WARNING(9);                       \
DUMP_WARNING(10);                       \
DUMP_WARNING(11);                       \
DUMP_WARNING(12);                       \
DUMP_WARNING(13);                       \
DUMP_WARNING(14);                       \
DUMP_WARNING(15);                       \
DUMP_WARNING(16);                       \
DUMP_WARNING(17);                       \
DUMP_WARNING(18);                       \
DUMP_WARNING(19);                       \
constexpr bool hasErr = parser.errRow > -1 || parser.errCol > -1; \
spt::IF<hasErr, spt::Error<parser.errRow, parser.errCol, spt::MsgToType<parser.err>::type>> p;

#else

#define REPORT_ERRORS(parser)   

#endif


