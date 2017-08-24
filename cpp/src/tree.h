#ifndef TREE_H
#define TREE_H
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include <ostream>
#include <sstream>
#include <map>

#include "constants.h"


struct Tree {
	virtual void print(std::ostream& out, int tabs=0) = 0;
	virtual std::string str(){
		std::ostringstream ost;
		this->print(ost);
		return ost.str();
	}
};

struct TreeSequence {
	std::vector<Tree*> trees;
	void print(std::ostream& out, int tabs=0){
		for(Tree* t : trees){
			t->print(out, tabs);
		}
	}
};

struct ListOfTrees : public Tree {
	std::vector<Tree*> subtrees;
	virtual void print(std::ostream& out, int tabs=0){
		//for(int i = 0; i< tabs; ++i) { out << '\t'; }
		out << TREE_OPEN;
		//out << '\n';
		for(Tree* t : subtrees){
			t->print(out, tabs+1);
		}
		//for(int i = 0; i< tabs; ++i) { out << '\t'; }
		out << TREE_CLOSE;
		//out << '\n';
	}
};

struct Hole : public Tree {
	int hole_id;
	Hole(int id):hole_id(id){}
	virtual void print(std::ostream& out, int tabs=0){
		//for(int i = 0; i< tabs; ++i) { out << '\t'; }
		out << '$' << hole_id;
		//out << '\n';
	}
};

struct Context : public Tree {
	std::vector<Tree*> subtrees;
	virtual void print(std::ostream& out, int tabs=0){
		//for(int i = 0; i< tabs; ++i) { out << '\t'; }
		out << TREE_OPEN;
		//out << '\n';
		for(Tree* t : subtrees){
			t->print(out, tabs+1);
		}
		//for(int i = 0; i< tabs; ++i) { out << '\t'; }
		out << TREE_CLOSE;
		//out << '\n';
	}

	virtual Tree* replace(Tree* v){
		ListOfTrees* out = new ListOfTrees();
		int context_count = 0;
		for(Tree* t : subtrees){
			Context* ct = dynamic_cast<Context*>(t);
			if(ct){
				context_count++;
				out->subtrees.push_back(ct->replace(v));
			} else{
				out->subtrees.push_back(t);
			}
		}
		assert(context_count==1);
		return out;
	}
};

struct ContextHole : public Context {
	virtual void print(std::ostream& out, int tabs=0){
		//for(int i = 0; i< tabs; ++i) { out << '\t'; }
		out << "<hole>";
		//out << '\n';
	}

	virtual Tree* replace(Tree* v){ return v;}
};

struct StringLeaf : public Tree {
	std::string text;
	StringLeaf(const std::string&& s):text(s){}
	StringLeaf(const std::string& s):text(s){}

	virtual void print(std::ostream& out, int tabs=0){
		//for(int i = 0; i< tabs; ++i) { out << '\t'; }
		out << text;
		//out << '\n';
	}
};


namespace tree {
namespace {
	//TODO move to utility file
bool isPrefix(const std::string& pref, const std::string& ret, int index){
	for(int i = 0 ; i < pref.length(); ++i){
		if(index+i > ret.length()){ return false; }
		if(pref[i] != ret[index+i]) {return false;}
	}
	return true;
}

// TODO: move to utility file and refactor
std::string nextToken(const std::string& ret, int index){
	//this is slow...
	if(isPrefix(ESCAPE_SEQ, ret, index)){
		int escape_seq_len = ESCAPE_SEQ.length();
		return ESCAPE_SEQ+nextToken(ret, index+escape_seq_len);
	}

	if(isPrefix(TREE_OPEN, ret, index)){
		return TREE_OPEN;
	}

	if(isPrefix(TREE_CLOSE, ret, index)){
		return TREE_CLOSE;
	}

	return {ret[index]};
}

//TODO: move to utility file
int assertAndEatNextToken(const std::string& str, int i, const std::string& v){
	assert(nextToken(str, i) == v);
	return i+v.length();
}

bool isAlpha(const std::string& s){
	return std::all_of(s.begin(), s.end(), [&](char a){
			return std::isdigit(a);
		});
}

std::pair<Tree*,int> _parse(const std::string& str, int i = 0, bool has_holes = false){
	//std::cout << "Working on: " << str.substr(i) << '\n';
	i = assertAndEatNextToken(str, i, TREE_OPEN);
	ListOfTrees* out = new ListOfTrees();

	while (i < str.length() && nextToken(str,i) != TREE_CLOSE) {
		if(nextToken(str,i) == TREE_OPEN){
			auto p = _parse(str, i, has_holes);
			out->subtrees.push_back(p.first);
			i = p.second;
		} else if(has_holes && nextToken(str,i) == HOLE){
			i = assertAndEatNextToken(str, i, HOLE);
			std::string num;
			while(isAlpha(nextToken(str, i))){
				std::string tok = nextToken(str, i);
				i+= tok.length();
				num+= tok;
			}
			out->subtrees.push_back(new Hole(stoi(num)));
		} else {
			std::string tok = nextToken(str, i);
			out->subtrees.push_back(new StringLeaf(tok));
			i += tok.length();
		}
	}
	i = assertAndEatNextToken(str, i, TREE_CLOSE);
	return {out,i};
}
}

Tree* parse_replacement(const std::string& str){
	auto p = _parse(str, 0, true);
	assert(p.second == str.length() && "Parsing finished, but characters remained!");
	return p.first;
}

Tree* parse(const std::string& str){
	auto p = _parse(str);
	assert(p.second == str.length() && "Parsing finished, but characters remained!");
	return p.first;
}

Tree* perform_replacement(Tree* in, std::map<int, TreeSequence>& captures){
	Hole* ho = dynamic_cast<Hole*>(in);
	if(ho){
		assert(captures[ho->hole_id].trees.size()==1);
		assert(!dynamic_cast<Context*>(captures[ho->hole_id].trees[0]));
		return captures[ho->hole_id].trees[0];
	}

	ListOfTrees* lt = dynamic_cast<ListOfTrees*>(in);
	if(!lt){
		assert(dynamic_cast<StringLeaf*>(in));
		return in;
	}

	ListOfTrees* out = new ListOfTrees();
	std::vector<int> positions;
	for(Tree* t : lt->subtrees){
		Hole* ht = dynamic_cast<Hole*>(t);
		ListOfTrees* ltt = dynamic_cast<ListOfTrees*>(t);
		if(ht){
			for(Tree* t2 : captures[ht->hole_id].trees){
				out->subtrees.push_back(t2);
				Context* co = nullptr;
				while(out->subtrees.size()>1 &&
						(co = dynamic_cast<Context*>(*(out->subtrees.end()-2))) &&
						!(dynamic_cast<Context*>(out->subtrees.back()))
						){
					Tree* t = out->subtrees.back();
					out->subtrees.pop_back();
					out->subtrees.back() = co->replace(t);
				}
			}
		} else if(ltt){
			out->subtrees.push_back(perform_replacement(ltt, captures));
		} else {
			assert(dynamic_cast<StringLeaf*>(t));
			out->subtrees.push_back(t);
		}

		Context* co = nullptr;
		while(out->subtrees.size()>1 &&
				(co = dynamic_cast<Context*>(*(out->subtrees.end()-2))) &&
				!(dynamic_cast<Context*>(out->subtrees.back()))
			){
			Tree* t = out->subtrees.back();
			out->subtrees.pop_back();
			out->subtrees.back() = co->replace(t);
		}
	}
	return out;
}
}

#endif /* TREE_H */
