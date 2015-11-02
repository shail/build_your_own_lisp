# Write down some more examples of strings the Doge language contains.

"so book much build many language"
"wow c many lisp such build"

# Why are there back slashes \ in front of the quote marks " in the grammar?

You need to escape the double quotes because then whatever reads this file wouldn't know when the string began/ended.

# Why are there back slashes \ at the end of the line in the grammar?

This is to signify that the string will continue on a new line.

# Describe textually a grammar for decimal numbers such 0.01 or 52.221

mpc_parser_t* Number = mpc_new("number");
mpc_parser_t* Decimal = mpc_new("decimal");

mpca_lang(MPCA_LANG_DEFAULT,
"                                                    \
  number: 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9;     \
  decimal: <number>+.<digit>* | <digit>*;            \
"

# Describe textually a grammar for web URLs such as http://www.buildyourownlisp.com

mpc_parser_t* Protocol = mpc_new("protocol");
mpc_parser_t* Tld = mpc_new("tld");
mpc_parser_t* Hostname = mpc_new("hostname");
mpc_parser_t* Url = mpc_new("url");

mpca_lang(MPCA_LANG_DEFAULT,
"
  protocol: \"http\";                                \
  tld: \"com\" | \"net\" | \"org\";                  \
  hostname: [a-z]+;                                  \
  url: <protocol>://www.(<hostname>.)+<tld>;
"

# Describe textually a grammar for simple English sentences such as "the cat sat on the math"

mpc_parser_t* Article = mpc_new("article");
mpc_parser_t* Noun = mpc_new("noun");
mpc_parser_t* Verb = mpc_new("verb");
mpc_parser_t* Preposition = mpc_new("preposition");
mpc_parser_t* Prepositional = mpc_new("prepositional");
mpc_parser_t* NounPhrase = mpc_new("nounphrase");
mpc_parser_t* Sentence = mpc_new("sentence");

mpca_lang(MPCA_LANG_DEFAULT,
"
  article: \"the\" | \"a\" | \"an\";                 \
  noun: \"cat\" | \"mat\";                           \
  verb: \"sat\";                                     \
  preposition: \"on\";                               \
  prepositional: <preposition> <article> <noun>;     \
  nounphrase: <article> <noun>;                      \
  sentence: <nounphrase> <verb> <prepositional>;     \
"
