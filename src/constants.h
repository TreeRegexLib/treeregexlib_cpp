#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

const std::string ESCAPE_SEQ="\\";
const std::string KLEENE_STAR="*";
const std::string UNION="|";
const std::string SUBREGEX_OPEN="(";
const std::string SUBREGEX_CLOSE=")";
const std::string CONCATENATION="";
const std::string CONTEXT_OPEN="(*"; // changed from (* to avoid the (@*) escaping issue
const std::string CONTEXT_CLOSE="*)";
const std::string EXACT_OPEN="(%";
const std::string EXACT_CLOSE="%)";
const std::string SUBTREE = "@";
const std::string HOLE = "$";
const std::string ALL_CHARS = ".";
const std::string CHAR_CLASS_OPEN = "[";
const std::string CHAR_CLASS_CLOSE = "]";
const std::string CHAR_CLASS_NEG = "^";
const std::string CHAR_CLASS_INC = "-";
#define TREE_OPEN EXACT_OPEN
#define TREE_CLOSE EXACT_CLOSE

//TODO: implement...
std::string escape(const std::string& in){
	return in;
}
std::string unescape(const std::string& in){
	std::string out;
	for(int i = 0; i < in.length(); ++i){
		if(i + ESCAPE_SEQ.length() >= in.length()){
			out+=in[i];
		} else if(in.substr(i, ESCAPE_SEQ.length()) == ESCAPE_SEQ){
			i+= ESCAPE_SEQ.length()-1;
		} else {
			out+= in[i];
		}
	}
	return out;
}

#endif /* CONSTANTS_H */
