const errs = [
  'Expecting_an_identifier',
  'Unexpected_character_inside_tag_content',
  'Expecting_a_tag_name_after_open_bracket',
  'Expecting_open_quote_for_attribute_value',
  'Empty_value_for_non_boolean_attribute',
  'Duplicate_ID_on_tag',
  'Expecting_a_value_for_attribute',
  'Missing_open_bracket',
  'Unknown_tag_name',
  'Missing_close_bracket_on_void_tag',
  'Missing_close_bracket_on_open_tag',
  'Expecting_a_close_tag',
  'Mismatched_Close_Tag',
  'Missing_close_bracket_in_close_tag',
];

function makeEnums(e) {return 'Error_' + e;}
const enums = errs.map(makeEnums).join(',\n  '); 

function makeStruct(e) {return 'struct ' + e + ' {};';}
const structs = errs.map(makeStruct).join('\n');


function makeMsgToType(e) {return `template<> struct MsgToType<Error_${e}>{typedef ${e} type;}; `}
const MsgToType = errs.map(makeMsgToType).join('\n');


function makeWarner(e) {return `spt::IF<w.m == spt::${'Error_' + e}, spt::Warning<w.row, w.col, spt::MsgToType<w.m>::type>> ();  \\`}
const warners = errs.map(makeWarner).join('\n  ');

var N = 20; 
function makeDump(e) {return `DUMP_WARNING(${e});                       \\`}
const dumps = Array.apply(null, {length: N}).map(Number.call, Number).map(makeDump).join('\n');

const template = `
enum Messages
{
  Error_None,
  ${enums}
};

struct None;
${structs}

template<Messages m> struct MsgToType{};

template<> struct MsgToType<Error_None>{typedef None type;};
${MsgToType}

#ifndef SPT_DEBUG

#define DUMP_WARNING(x)                                        \\
if(x < n)                                                      \\
{                                                              \\
  constexpr auto w = parser.warns[x];                          \\
  ${warners}
}

#define REPORT_ERRORS(parser)          \\
constexpr int n = parser.warns.size();          \\
${dumps}
constexpr bool hasErr = parser.errRow > -1 || parser.errCol > -1; \\
spt::IF<hasErr, spt::Error<parser.errRow, parser.errCol, spt::MsgToType<parser.err>::type>> {};

#else

#define REPORT_ERRORS(parser)   

#endif

`;

console.log(template);
