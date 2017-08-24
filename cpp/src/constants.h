#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

const std::string ESCAPE_SEQ="/";
const std::string KLEENE_STAR="*";
const std::string UNION="|";
const std::string SUBREGEX_OPEN="(";
const std::string SUBREGEX_CLOSE=")";
const std::string CONCATENATION="";
const std::string CONTEXT_OPEN="(&"; // changed from (* to avoid the (@*) escaping issue
const std::string CONTEXT_CLOSE="&)";
const std::string EXACT_OPEN="(%";
const std::string EXACT_CLOSE="%)";
const std::string SUBTREE = "@";
const std::string HOLE = "$";
const std::string ALL_CHARS = ".";
const std::string CHAR_CLASS_OPEN = "[";
const std::string CHAR_CLASS_CLOSE = "]";
const std::string CHAR_CLASS_NEG = "^";
#define TREE_OPEN EXACT_OPEN
#define TREE_CLOSE EXACT_CLOSE

#endif /* CONSTANTS_H */
