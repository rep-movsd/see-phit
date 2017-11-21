
#ifndef SEEPHIT_PARSE_ERROR_GENERATED_H
#define SEEPHIT_PARSE_ERROR_GENERATED_H

enum Messages
{
  Error_None,
  Error_Expecting_an_identifier,
  Error_Unexpected_character_inside_tag_content,
  Error_Expecting_a_tag_name_after_open_bracket,
  Error_Empty_value_for_non_boolean_attribute,
  Error_Duplicate_ID_on_tag,
  Error_Expecting_a_value_for_attribute,
  Error_Missing_open_bracket,
  Error_Unknown_tag_name,
  Error_Missing_close_bracket_on_void_tag,
  Error_Missing_close_bracket_on_open_tag,
  Error_Expecting_a_close_tag,
  Error_Mismatched_Close_Tag,
  Error_Missing_close_bracket_in_close_tag,
  Error_Missing_close_brace_in_template,
  Error_Unexpected_end_of_stream,
  Error_Invalid_syntax_in_for_tag,
  Error_Invalid_syntax_in_if_tag,
  Error_Infinite_loop_in_for_tag
};

struct None;
struct Expecting_an_identifier {};
struct Unexpected_character_inside_tag_content {};
struct Expecting_a_tag_name_after_open_bracket {};
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
struct Missing_close_brace_in_template {};
struct Unexpected_end_of_stream {};
struct Invalid_syntax_in_for_tag {};
struct Invalid_syntax_in_if_tag {};
struct Infinite_loop_in_for_tag {};

template<Messages m> struct MsgToType{};

template<> struct MsgToType<Error_None>{using type = None;};
template<> struct MsgToType<Error_Expecting_an_identifier>{using type = Expecting_an_identifier;}; 
template<> struct MsgToType<Error_Unexpected_character_inside_tag_content>{using type = Unexpected_character_inside_tag_content;}; 
template<> struct MsgToType<Error_Expecting_a_tag_name_after_open_bracket>{using type = Expecting_a_tag_name_after_open_bracket;}; 
template<> struct MsgToType<Error_Empty_value_for_non_boolean_attribute>{using type = Empty_value_for_non_boolean_attribute;}; 
template<> struct MsgToType<Error_Duplicate_ID_on_tag>{using type = Duplicate_ID_on_tag;}; 
template<> struct MsgToType<Error_Expecting_a_value_for_attribute>{using type = Expecting_a_value_for_attribute;}; 
template<> struct MsgToType<Error_Missing_open_bracket>{using type = Missing_open_bracket;}; 
template<> struct MsgToType<Error_Unknown_tag_name>{using type = Unknown_tag_name;}; 
template<> struct MsgToType<Error_Missing_close_bracket_on_void_tag>{using type = Missing_close_bracket_on_void_tag;}; 
template<> struct MsgToType<Error_Missing_close_bracket_on_open_tag>{using type = Missing_close_bracket_on_open_tag;}; 
template<> struct MsgToType<Error_Expecting_a_close_tag>{using type = Expecting_a_close_tag;}; 
template<> struct MsgToType<Error_Mismatched_Close_Tag>{using type = Mismatched_Close_Tag;}; 
template<> struct MsgToType<Error_Missing_close_bracket_in_close_tag>{using type = Missing_close_bracket_in_close_tag;}; 
template<> struct MsgToType<Error_Missing_close_brace_in_template>{using type = Missing_close_brace_in_template;}; 
template<> struct MsgToType<Error_Unexpected_end_of_stream>{using type = Unexpected_end_of_stream;}; 
template<> struct MsgToType<Error_Invalid_syntax_in_for_tag>{using type = Invalid_syntax_in_for_tag;}; 
template<> struct MsgToType<Error_Invalid_syntax_in_if_tag>{using type = Invalid_syntax_in_if_tag;}; 
template<> struct MsgToType<Error_Infinite_loop_in_for_tag>{using type = Infinite_loop_in_for_tag;}; 

#ifndef SPT_DEBUG

#define DUMP_WARNING(x)                                        \
if((x) < n)                                                      \
{                                                              \
  constexpr auto w = parser.m_arrWarns[(x)];                          \
  spt::IF<w.m == spt::Error_Expecting_an_identifier, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Unexpected_character_inside_tag_content, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Expecting_a_tag_name_after_open_bracket, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
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
  spt::IF<w.m == spt::Error_Missing_close_brace_in_template, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Unexpected_end_of_stream, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Invalid_syntax_in_for_tag, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Invalid_syntax_in_if_tag, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
  spt::IF<w.m == spt::Error_Infinite_loop_in_for_tag, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \
}

#define REPORT_ERRORS(parser)          \
constexpr int n = parser.m_arrWarns.size();          \
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
constexpr bool hasErr = parser.m_iErrRow > -1 || parser.m_iErrCol > -1; \
spt::IF<hasErr, spt::Error<parser.m_iErrRow, parser.m_iErrCol, spt::MsgToType<parser.m_arrErrs>::type>> {};

#else

#define REPORT_ERRORS(parser)   

#endif

#endif

