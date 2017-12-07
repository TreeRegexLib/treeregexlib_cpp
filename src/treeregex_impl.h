#ifndef TREEREGEX_IMPL_H
#define TREEREGEX_IMPL_H

#include <string>
#include <vector>
#include <cassert>
#include <ostream>
#include <bitset>
#include <memory>

#include "constants.h"

const std::vector<std::string> large_tokens = {CONTEXT_OPEN, CONTEXT_CLOSE, EXACT_OPEN, EXACT_CLOSE};

namespace { int matcher_group = 1; }

struct StringMatcher;
struct EpsilonMatcher;
struct TreeMatcher;
struct MatcherOperator;
struct CharacterSetMatcher;

struct Matcher : std::enable_shared_from_this<Matcher>{
	int matching_group;
	Matcher():matching_group(0){}
	virtual void print(std::ostream&) = 0;
	virtual std::shared_ptr<StringMatcher> toStringMatcher(){ return nullptr; }
	virtual std::shared_ptr<EpsilonMatcher> toEpsilonMatcher(){ return nullptr; }
	virtual std::shared_ptr<TreeMatcher> toTreeMatcher(){ return nullptr; }
	virtual std::shared_ptr<MatcherOperator> toMatcherOperator(){ return nullptr; }
	virtual std::shared_ptr<CharacterSetMatcher> toCharacterSetMatcher(){ return nullptr; }
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
	virtual std::shared_ptr<StringMatcher> toStringMatcher(){ return std::static_pointer_cast<StringMatcher>(shared_from_this()); }
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
	virtual std::shared_ptr<CharacterSetMatcher> toCharacterSetMatcher(){ return std::static_pointer_cast<CharacterSetMatcher>(shared_from_this()); }
};

struct EpsilonMatcher : public Matcher {
	static std::shared_ptr<EpsilonMatcher> get(){ static std::shared_ptr<EpsilonMatcher> e = std::make_shared<EpsilonMatcher>(); return e; }

	virtual void print(std::ostream& out){
		out << "<epsilon>";
	}
	virtual std::shared_ptr<EpsilonMatcher> toEpsilonMatcher(){ return std::static_pointer_cast<EpsilonMatcher>(shared_from_this()); }
};


struct TreeMatcher :public Matcher {
	const std::string* type;
	std::shared_ptr<Matcher> body;
	TreeMatcher(const std::string* t, std::shared_ptr<Matcher> sub=nullptr): type(t), body(sub){}

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
	virtual std::shared_ptr<TreeMatcher> toTreeMatcher(){ return std::static_pointer_cast<TreeMatcher>(shared_from_this()); }
};

// supports star, union, concatenation
struct MatcherOperator : public Matcher {
	std::shared_ptr<Matcher> before_or_left;
	std::shared_ptr<Matcher> after_or_right;
	const std::string* type;
	MatcherOperator(const std::string* t, std::shared_ptr<Matcher> l, std::shared_ptr<Matcher> r = nullptr):type(t), before_or_left(l), after_or_right(r){}

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
	virtual std::shared_ptr<MatcherOperator> toMatcherOperator(){ return std::static_pointer_cast<MatcherOperator>(shared_from_this()); }
};

namespace  {
bool isPrefix(const std::string& pref, const std::string& ret, int index){
	for(int i = 0 ; i < pref.length(); ++i){
		if(index+i > ret.length()){ return false; }
		if(pref[i] != ret[index+i]) {return false;}
	}
	return true;
}

bool isEndToken(const std::string& ret){
	return ret == SUBREGEX_CLOSE || ret == CONTEXT_CLOSE || ret == EXACT_CLOSE;
}

std::string nextToken(const std::string& ret, int index, const std::string* end = nullptr){
	//this is slow...
	if(isPrefix(ESCAPE_SEQ, ret, index)){
		int escape_seq_len = ESCAPE_SEQ.length();
		return ESCAPE_SEQ+nextToken(ret, index+escape_seq_len);
	}

	for(const auto& t : large_tokens){ // tokens must be sorted by size -> longest first
		if(isPrefix(t, ret, index) && (end==nullptr || !isEndToken(t) || *end==t)){
			return t;
		}
	}

	return {ret[index]};
}

std::shared_ptr<Matcher> joinMatcher(std::shared_ptr<Matcher> a, std::shared_ptr<Matcher> b){
	if(!a){ return b;}
	if(!b) { return a;}
	return std::make_shared<MatcherOperator>(&CONCATENATION, a, b);
}

int assertAndEatNextToken(const std::string& str, int i, const std::string& v, const std::string* end){
	//std::cerr << "HELLO:" << str.substr(i) << '\t' << nextToken(str, i, end) << "END\n";
	assert(nextToken(str, i, end) == v);
	return i+v.length();
}

std::pair<std::shared_ptr<CharacterSetMatcher>, int> parse_char_set(const std::string& str, int i=0){
	bool is_first_token = true;
	auto out = std::make_shared<CharacterSetMatcher>();
	char last_added;
	while (i < str.length()) {
		if(is_first_token && nextToken(str, i) == CHAR_CLASS_NEG){
			out->negation = true;
			i+= CHAR_CLASS_NEG.length();
		} else if(nextToken(str, i) == CHAR_CLASS_CLOSE){
			break;
		} else if(nextToken(str, i) == CHAR_CLASS_INC){
			i+= CHAR_CLASS_INC.length();
			std::string to_add = nextToken(str, i);
			i+= to_add.length();
			assert(unescape(to_add).length() == 1);
			for(char c = last_added; c <= unescape(to_add)[0]; ++c){
				last_added = c;
				out->values[static_cast<uint8_t>(c)] = true;
			}
		} else {
			std::string to_add = nextToken(str, i);
			i+= to_add.length();
			for(char c: unescape(to_add)){
				last_added = c;
				out->values[static_cast<uint8_t>(c)] = true;
			}
		}

		is_first_token = false;
	}
	return {out, i};
}

std::pair<std::shared_ptr<Matcher>,int> __parse(const std::string& str, int i, const std::string* end );
std::pair<std::shared_ptr<Matcher>,int> _parse(const std::string& str, int i = 0, const std::string* end = nullptr){
	//std::cerr << "Going deep!\n";
	auto ret = __parse(str, i, end);
	//ret.first->print(std::cout); std::cout << '\n';
	//std::cerr << "Returning!\n";
	return ret;
}

std::pair<std::shared_ptr<Matcher>,int> __parse(const std::string& str, int i, const std::string* end ){
	std::shared_ptr<Matcher> out = nullptr;
	std::shared_ptr<Matcher> last_out = nullptr;
	while (i < str.length()) {
		auto nextTok = nextToken(str, i, end);
		//std::cerr << "Parsing: " << str.substr(i) << '\t' << (end?*end:"nullptr") << '\t' << nextToken(str, i, end) << '\t' << i << '\n';
		if(nextToken(str, i, end) == SUBREGEX_CLOSE ||
			nextToken(str, i, end) == CONTEXT_CLOSE ||
			nextToken(str, i, end) == EXACT_CLOSE){
			//std::cerr << "End: " << (end?*end:"Nullptr") << '\n';
			//std::cerr << "Found end!\n";
			break;
		} else if(nextTok == CONTEXT_OPEN){
			int tmp_matcher = matcher_group++;
			auto p = _parse(str, i+CONTEXT_OPEN.length(), &CONTEXT_CLOSE);
			i = p.second;
			i = assertAndEatNextToken(str, i, CONTEXT_CLOSE, &CONTEXT_CLOSE);
			last_out = out;
			auto tm = std::make_shared<TreeMatcher>(&CONTEXT_OPEN, p.first);
			tm->matching_group = tmp_matcher;
			out = joinMatcher(out, tm);
		} else if (nextTok == EXACT_OPEN){
			auto p = _parse(str, i+EXACT_OPEN.length(), &EXACT_CLOSE);
			i = p.second;
			i = assertAndEatNextToken(str, i, EXACT_CLOSE, &EXACT_CLOSE);
			last_out = out;
			out = joinMatcher(out, std::make_shared<TreeMatcher>(&EXACT_OPEN, p.first));
		} else if (nextTok == KLEENE_STAR){
			i++;
			assert(out);
			if(last_out == nullptr){
				last_out = out;
				out = std::make_shared<MatcherOperator>(&KLEENE_STAR, out);
			} else {
				std::shared_ptr<MatcherOperator> mo = (out)->toMatcherOperator();
				assert(mo);
				if(mo->type == &KLEENE_STAR){
					last_out = out;
					out = std::make_shared<MatcherOperator>(&KLEENE_STAR, out);
				} else {
					mo->after_or_right = std::make_shared<MatcherOperator>(&KLEENE_STAR, mo->after_or_right);
				}
			}
		} else if (nextTok == UNION){
			auto p = _parse(str, i+UNION.length(), end);
			i = p.second;
			last_out = out;
			out = std::make_shared<MatcherOperator>(&UNION, out, p.first);
		} else if(nextTok == SUBTREE){
			i++;
			last_out = out;
			auto tmp = std::make_shared<TreeMatcher>(&SUBTREE);
			tmp->matching_group = matcher_group++;
			//std::cout << "SUBTREE CREATE: " << tmp->matching_group << '\n';
			out = joinMatcher(out, tmp);
		} else if(nextTok == SUBREGEX_OPEN){
			int tmp_matcher = matcher_group++;
			auto p = _parse(str, i+SUBREGEX_OPEN.length(), &SUBREGEX_CLOSE);
			i = p.second;
			i = assertAndEatNextToken(str, i, SUBREGEX_CLOSE, &SUBREGEX_CLOSE);
			p.first->matching_group = tmp_matcher;
			last_out = out;
			out = joinMatcher(out, p.first);
		} else if(nextTok == CHAR_CLASS_OPEN){
			auto p = parse_char_set(str, i+CHAR_CLASS_OPEN.length());
			i=p.second;
			i = assertAndEatNextToken(str, i, CHAR_CLASS_CLOSE, nullptr);
			last_out = out;
			out = joinMatcher(out, p.first);
		} else if(nextTok == ALL_CHARS){
				last_out = out;
				auto tmp = std::make_shared<CharacterSetMatcher>();
				tmp->negation = true;
				out = joinMatcher(out, tmp);
				i+=ALL_CHARS.length();
		} else {
			// semi-hack to save on space
			/*if(std::shared_ptr<StringMatcher> out_str = (out)->toStringMatcher()){
				std::string tok = nextTok;
				out_str->addToken(tok);
				i+=tok.length();
			} else*/ {
				std::string tok = nextTok;
				last_out = out;
				out = joinMatcher(out, std::make_shared<StringMatcher>(tok));
				i+=tok.length();
			}
		}
	}
	//std::cerr << "END:" << i << '\n';
	if(!out){
		last_out = out;
		out = EpsilonMatcher::get();
	}
	return {out,i};
}
}

namespace treeregex {
std::shared_ptr<Matcher> parse(const std::string& str){
	matcher_group = 1;
	auto p = _parse(str, 0, nullptr);
	assert(p.second == str.length() && "Parsing finished, but characters remained!");
	//std::cout << "CAPTURES: " << matcher_group << '\n';
	return p.first;
}
}

#endif /*TREEREGEX_IMPL_H*/
