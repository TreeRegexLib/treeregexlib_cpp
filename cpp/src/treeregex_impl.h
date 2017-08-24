#ifndef TREEREGEX_IMPL_H
#define TREEREGEX_IMPL_H

#include <string>
#include <vector>
#include <cassert>
#include <ostream>
#include <bitset>

#include "constants.h"

const std::vector<std::string> large_tokens = {CONTEXT_OPEN, CONTEXT_CLOSE, EXACT_OPEN, EXACT_CLOSE};

//TODO: implement...
std::string escape(const std::string& in){ return in; }
std::string unescape(const std::string& in){ return in; }

namespace { int matcher_group = 1; }
struct Matcher {
	int matching_group;
	Matcher():matching_group(0){}
	virtual void print(std::ostream&) = 0;
	virtual ~Matcher(){}
};

struct StringMatcher : public Matcher {
	std::string regex; // for lack of a better variable name...
	StringMatcher(const std::string& reg): regex(unescape(reg)) {}

	void addToken(const std::string& add){
		regex+=unescape(add);
	}

	virtual void print(std::ostream& out){
		out << escape(regex);
	}
};

struct CharacterSetMatcher : public Matcher {
	std::bitset<256> values;
	bool negation;

	virtual void print(std::ostream& out){
		out << "[";
		if(negation){ out << "^"; }
		for(int i = 0; i < values.size(); ++i){
			if(values[i]) { out << char(i); }
		}
		out << "]";
	}
};

struct EpsilonMatcher : public Matcher {
	EpsilonMatcher(){}

	static void* operator new(std::size_t sz){
		static void* saved = nullptr;
		if(!saved){
			saved = ::operator new(sz);
		}
		return saved;
	}

	//TODO: is this actually being called on a Matcher* pointer?
	static void operator delete(void* ptr){
	}

	virtual void print(std::ostream& out){
		out << "<epsilon>";
	}
};

struct TreeMatcher :public Matcher {
	const std::string* type;
	Matcher* body;
	TreeMatcher(const std::string* t, Matcher* sub=nullptr): type(t), body(sub){}

	virtual void print(std::ostream& out){
		if(type == &SUBTREE){
			out << SUBTREE;
		} else if(type == &EXACT_OPEN){
			out << EXACT_OPEN;
			body->print(out);
			out << EXACT_CLOSE;
		} else if(type == &CONTEXT_OPEN){
			out << CONTEXT_OPEN;
			body->print(out);
			out << CONTEXT_CLOSE;
		}
	}
	~TreeMatcher(){ delete body; }
};

// supports star, union, concatenation
struct MatcherOperator : public Matcher {
	Matcher* before_or_left;
	Matcher* after_or_right;
	const std::string* type;
	MatcherOperator(const std::string* t, Matcher* l, Matcher* r = nullptr):type(t), before_or_left(l), after_or_right(r){}

	virtual void print(std::ostream& out){
		if(type == &CONCATENATION){
			before_or_left->print(out);
			after_or_right->print(out);
		} else if(type == &KLEENE_STAR){
			out << '(';
			before_or_left->print(out);
			out << ')';
			out << *type;
		} else {
			out << '(';
			before_or_left->print(out);
			out << *type;
			if(after_or_right != nullptr){
				after_or_right->print(out);
			}
			out << ')';
		}
	}
	~MatcherOperator(){ delete before_or_left; delete after_or_right; }
};

namespace  {
bool isPrefix(const std::string& pref, const std::string& ret, int index){
	for(int i = 0 ; i < pref.length(); ++i){
		if(index+i > ret.length()){ return false; }
		if(pref[i] != ret[index+i]) {return false;}
	}
	return true;
}

std::string nextToken(const std::string& ret, int index){
	//this is slow...
	if(isPrefix(ESCAPE_SEQ, ret, index)){
		int escape_seq_len = ESCAPE_SEQ.length();
		return ESCAPE_SEQ+nextToken(ret, index+escape_seq_len);
	}

	for(const auto& t : large_tokens){ // tokens must be sorted by size -> longest first
		if(isPrefix(t, ret, index)){
			return t;
		}
	}

	return {ret[index]};
}

Matcher* joinMatcher(Matcher* a, Matcher* b){
	if(!a){ return b;}
	if(!b) { return a;}
	return new MatcherOperator(&CONCATENATION, a, b);
}

int assertAndEatNextToken(const std::string& str, int i, const std::string& v){
	assert(nextToken(str, i) == v);
	return i+v.length();
}

std::pair<CharacterSetMatcher*, int> parse_char_set(const std::string& str, int i=0){
	bool is_first_token = true;
	auto* out = new CharacterSetMatcher();
	while (i < str.length()) {
		if(is_first_token && nextToken(str, i) == CHAR_CLASS_NEG){
			out->negation = true;
			i+= CHAR_CLASS_NEG.length();
		} else if(nextToken(str, i) == CHAR_CLASS_CLOSE){
			break;
		} else {
			std::string to_add = nextToken(str, i);
			i+= to_add.length();
			for(char c: unescape(to_add)){
				out->values[static_cast<uint8_t>(c)] = true;
			}
		}

		is_first_token = false;
	}
	return {out, i};
}

std::pair<Matcher*,int> __parse(const std::string& str, int i = 0);
std::pair<Matcher*,int> _parse(const std::string& str, int i = 0){
	auto ret = __parse(str, i);
	//ret.first->print(std::cout); std::cout << '\n';
	return ret;
}

std::pair<Matcher*,int> __parse(const std::string& str, int i ){
	Matcher* out = nullptr;
	Matcher* last_out = nullptr;
	while (i < str.length()) {
		if(nextToken(str, i) == CONTEXT_OPEN){
			int tmp_matcher = matcher_group++;
			auto p = _parse(str, i+CONTEXT_OPEN.length());
			i = p.second;
			i = assertAndEatNextToken(str, i, CONTEXT_CLOSE);
			last_out = out;
			auto* tm = new TreeMatcher(&CONTEXT_OPEN, p.first);
			tm->matching_group = tmp_matcher;
			out = joinMatcher(out, tm);
		} else if (nextToken(str, i) == EXACT_OPEN){
			auto p = _parse(str, i+EXACT_OPEN.length());
			i = p.second;
			i= assertAndEatNextToken(str, i, EXACT_CLOSE);
			last_out = out;
			out = joinMatcher(out, new TreeMatcher(&EXACT_OPEN, p.first));
		} else if (nextToken(str, i) == KLEENE_STAR){
			i++;
			assert(out);
			if(last_out == nullptr){
				last_out = out;
				out = new MatcherOperator(&KLEENE_STAR, out);
			} else {
				MatcherOperator* mo = dynamic_cast<MatcherOperator*>(out);
				assert(mo);
				if(mo->type == &KLEENE_STAR){
					last_out = out;
					out = new MatcherOperator(&KLEENE_STAR, out);
				} else {
					mo->after_or_right = new MatcherOperator(&KLEENE_STAR, mo->after_or_right);
				}
			}
		} else if (nextToken(str, i) == UNION){
			auto p = _parse(str, i+UNION.length());
			i = p.second;
			last_out = out;
			out = new MatcherOperator(&UNION, out, p.first);
		} else if(nextToken(str, i) == SUBTREE){
			i++;
			last_out = out;
			auto* tmp = new TreeMatcher(&SUBTREE);
			tmp->matching_group = matcher_group++;
			//std::cout << "SUBTREE CREATE: " << tmp->matching_group << '\n';
			out = joinMatcher(out, tmp);
		} else if(nextToken(str, i) == SUBREGEX_OPEN){
			int tmp_matcher = matcher_group++;
			auto p = _parse(str, i+SUBREGEX_OPEN.length());
			i = p.second;
			i = assertAndEatNextToken(str, i, SUBREGEX_CLOSE);
			p.first->matching_group = tmp_matcher;
			last_out = out;
			out = joinMatcher(out, p.first);
		} else if(nextToken(str, i) == SUBREGEX_CLOSE ||
			nextToken(str, i) == CONTEXT_CLOSE ||
			nextToken(str, i) == EXACT_CLOSE){
			break;
		} else if(nextToken(str, i) == CHAR_CLASS_OPEN){
			auto p = parse_char_set(str, i+CHAR_CLASS_OPEN.length());
			i=p.second;
			i = assertAndEatNextToken(str, i, CHAR_CLASS_CLOSE);
			last_out = out;
			out = joinMatcher(out, p.first);
		} else if(nextToken(str, i) == ALL_CHARS){
				last_out = out;
				auto* tmp = new CharacterSetMatcher();
				tmp->negation = true;
				out = joinMatcher(out, tmp);
				i+=ALL_CHARS.length();
		} else {
			// semi-hack to save on space
			/*if(StringMatcher* out_str = dynamic_cast<StringMatcher*>(out)){
				std::string tok = nextToken(str, i);
				out_str->addToken(tok);
				i+=tok.length();
			} else*/ {
				std::string tok = nextToken(str, i);
				last_out = out;
				out = joinMatcher(out, new StringMatcher(tok));
				i+=tok.length();
			}
		}
	}
	if(!out){
		last_out = out;
		out = new EpsilonMatcher();
	}
	return {out,i};
}
}

namespace treeregex {
Matcher* parse(const std::string& str){
	matcher_group = 1;
	auto p = _parse(str);
	assert(p.second == str.length() && "Parsing finished, but characters remained!");
	//std::cout << "CAPTURES: " << matcher_group << '\n';
	return p.first;
}
}

#endif /*TREEREGEX_IMPL_H*/
