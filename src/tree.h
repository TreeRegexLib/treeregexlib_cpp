#ifndef TREE_H
#define TREE_H
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include <ostream>
#include <sstream>
#include <map>
#include <memory>

#include "constants.h"


struct ListOfTrees;
struct Hole;
struct Context;
struct ContextHole;
struct StringLeaf;
struct Tree : std::enable_shared_from_this<Tree> {
	virtual void print(std::ostream& out, bool serialize=false, int tabs=0) = 0;
	virtual std::string str(){
		std::ostringstream ost;
		this->print(ost);
		return ost.str();
	}
	virtual std::shared_ptr<ListOfTrees> toListOfTrees(){ return nullptr; }
	virtual std::shared_ptr<Hole> toHole(){ return nullptr; }
	virtual std::shared_ptr<Context> toContext(){ return nullptr; }
	virtual std::shared_ptr<StringLeaf> toStringLeaf(){ return nullptr; }
	virtual std::shared_ptr<ContextHole> toContextHole(){ return nullptr; }
};

struct TreeSequence {
	std::vector<std::shared_ptr<Tree>> trees;
	void print(std::ostream& out, bool serialize=false, int tabs=0){
		for(std::shared_ptr<Tree> t : trees){
			t->print(out, serialize, tabs);
		}
	}
	std::string str(){
		std::ostringstream ost;
		this->print(ost);
		return ost.str();
	}
};

struct ListOfTrees : public Tree {
	std::vector<std::shared_ptr<Tree>> subtrees;
	virtual void print(std::ostream& out, bool serialize=false, int tabs=0){
		//for(int i = 0; i< tabs; ++i) { out << '\t'; }
		if (!serialize){ out << TREE_OPEN;}
		//out << '\n';
		for(std::shared_ptr<Tree> t : subtrees){
			t->print(out, serialize, tabs+1);
		}
		//for(int i = 0; i< tabs; ++i) { out << '\t'; }
		if(!serialize){out << TREE_CLOSE;}
		//out << '\n';
	}
	virtual std::shared_ptr<ListOfTrees> toListOfTrees(){ return std::static_pointer_cast<ListOfTrees>(shared_from_this()); }
};

struct Hole : public Tree {
	int hole_id;
	Hole(int id):hole_id(id){}
	virtual void print(std::ostream& out, bool serialize=false, int tabs=0){
		//for(int i = 0; i< tabs; ++i) { out << '\t'; }
		out << '$' << hole_id;
		//out << '\n';
	}
	virtual std::shared_ptr<Hole> toHole(){ return std::static_pointer_cast<Hole>(shared_from_this()); }
};

struct Context : public Tree {
	std::vector<std::shared_ptr<Tree>> subtrees;
	virtual void print(std::ostream& out, bool serialize=false, int tabs=0){
		//for(int i = 0; i< tabs; ++i) { out << '\t'; }
		if(!serialize){out << TREE_OPEN;}
		//out << '\n';
		for(std::shared_ptr<Tree> t : subtrees){
			t->print(out, serialize, tabs+1);
		}
		//for(int i = 0; i< tabs; ++i) { out << '\t'; }
		if(!serialize){out << TREE_CLOSE;}
		//out << '\n';
	}

	virtual std::shared_ptr<Tree> replace(std::shared_ptr<Tree> v){
		std::shared_ptr<ListOfTrees> out = std::make_shared<ListOfTrees>();
		int context_count = 0;
		for(std::shared_ptr<Tree> t : subtrees){
			std::shared_ptr<Context> ct = t->toContext();
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
	virtual std::shared_ptr<Context> toContext(){ return std::static_pointer_cast<Context>(shared_from_this()); }
};

struct ContextHole : public Context {
	virtual void print(std::ostream& out, bool serailize=false, int tabs=0){
		//for(int i = 0; i< tabs; ++i) { out << '\t'; }
		out << "<hole>";
		//out << '\n';
	}

	virtual std::shared_ptr<Tree> replace(std::shared_ptr<Tree> v){ return v;}
	virtual std::shared_ptr<ContextHole> toContextHole(){ return std::static_pointer_cast<ContextHole>(shared_from_this()); }
};

struct StringLeaf : public Tree {
	std::string text;
	StringLeaf(const std::string&& s):text(unescape(s)){}
	StringLeaf(const std::string& s):text(unescape(s)){}

	virtual void print(std::ostream& out, bool serialize=false, int tabs=0){
		//for(int i = 0; i< tabs; ++i) { out << '\t'; }
		if(!serialize){out << escape(text);}
		else { out << text; }
		//out << '\n';
	}
	virtual std::shared_ptr<StringLeaf> toStringLeaf(){ return std::static_pointer_cast<StringLeaf>(std::static_pointer_cast<StringLeaf>(shared_from_this())); }
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

std::pair<std::shared_ptr<Tree>,int> _parse(const std::string& str, int i = 0, bool has_holes = false){
	//std::cout << "Working on: " << str.substr(i) << '\n';
	//std::cout << "Working on: " << i << ' ' << str.substr(i, std::min(10, int(str.length())-i-1)) << '\n';
	i = assertAndEatNextToken(str, i, TREE_OPEN);
	std::shared_ptr<ListOfTrees> out = std::make_shared<ListOfTrees>();

	while (i < str.length() && nextToken(str,i) != TREE_CLOSE) {
		if(nextToken(str,i) == TREE_OPEN){
			auto p = _parse(str, i, has_holes);
			out->subtrees.push_back(p.first);
			i = p.second;
		} else if(has_holes && nextToken(str,i) == HOLE){
			int temp = i;
			i = assertAndEatNextToken(str, i, HOLE);
			std::string num;
			while(isAlpha(nextToken(str, i))){
				std::string tok = nextToken(str, i);
				i+= tok.length();
				num+= tok;
			}
			//std::cout << "NUM: " << num << ' ' << temp << ' ' << str.substr(temp, 10) << std::endl;
			out->subtrees.push_back(std::make_shared<Hole>(stoi(num)));
		} else {
			std::string tok = nextToken(str, i);
			out->subtrees.push_back(std::make_shared<StringLeaf>(tok));
			i += tok.length();
		}
	}
	i = assertAndEatNextToken(str, i, TREE_CLOSE);
	return {out,i};
}
}

std::shared_ptr<Tree> parse_replacement(const std::string& str){
	auto p = _parse(str, 0, true);
	assert(p.second == str.length() && "Parsing finished, but characters remained!");
	return p.first;
}
std::shared_ptr<Tree> parse(const std::string& str){
	auto p = _parse(str);
	//std::cerr << str.substr(p.second) << '\n';
	//std::cerr << int(str[p.second]) << '\n';
	//std::cerr<< (p.second == str.length()) << ' ' << p.second << ' '<< str.length() << std::endl;
	if (p.second == str.length()-1 && str[p.second] != '\n'){
		assert(p.second == str.length() && "Parsing finished, but characters remained!");
	}
	return p.first;
}

std::shared_ptr<Tree> perform_replacement(std::shared_ptr<Tree> in, std::map<int, TreeSequence>& captures){
	std::shared_ptr<Hole> ho = in->toHole();
	if(ho){
		auto it = captures.find(ho->hole_id);
		assert(it != captures.end());
		assert(it->second.trees.size()==1);
		assert(!(it->second.trees[0]->toContext()));
		return it->second.trees[0];
	}

	std::shared_ptr<ListOfTrees> lt = in->toListOfTrees();
	if(!lt){
		assert(in->toStringLeaf());
		return in;
	}

	std::shared_ptr<ListOfTrees> out = std::make_shared<ListOfTrees>();
	std::vector<int> positions;
	for(std::shared_ptr<Tree> t : lt->subtrees){
		std::shared_ptr<Hole> ht = t->toHole();
		std::shared_ptr<ListOfTrees> ltt = t->toListOfTrees();
		if(ht){
			auto it = captures.find(ht->hole_id);
			assert(it != captures.end());
			for(std::shared_ptr<Tree> t2 : it->second.trees){
				out->subtrees.push_back(t2);
				std::shared_ptr<Context> co = nullptr;
				while(out->subtrees.size()>1 &&
						(co = (*(out->subtrees.end()-2))->toContext()) &&
						!((out->subtrees.back()->toContext()))
						){
					std::shared_ptr<Tree> t = out->subtrees.back();
					out->subtrees.pop_back();
					out->subtrees.back() = co->replace(t);
				}
			}
		} else if(ltt){
			out->subtrees.push_back(perform_replacement(ltt, captures));
		} else {
			assert((t->toStringLeaf()));
			out->subtrees.push_back(t);
		}

		std::shared_ptr<Context> co = nullptr;
		while(out->subtrees.size()>1 &&
				(co = (*(out->subtrees.end()-2))->toContext()) &&
				!((out->subtrees.back())->toContext())
			){
			std::shared_ptr<Tree> t = out->subtrees.back();
			out->subtrees.pop_back();
			out->subtrees.back() = co->replace(t);
		}
	}
	return out;
}
}

#endif /* TREE_H */
