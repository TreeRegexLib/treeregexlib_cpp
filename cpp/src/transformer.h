#include <utility>
#include <functional>
#include "tree.h"
#include "treeregex_impl.h"
#include "treeregex_matching.h"

std::function<bool(Tree*)> default_pred = [](Tree*){return true;};
std::function<void(std::map<int, TreeSequence>&)> default_mod = [](std::map<int, TreeSequence>&){};

class Transformer {
public:
	std::vector<std::function<Tree*(Tree*)>> pre;
	std::vector<std::function<Tree*(Tree*)>> post;

	void add(std::function<bool(Tree*)> pred, Matcher* m, std::function<void(std::map<int, TreeSequence>&)> mod, Tree* r, bool is_pre){
		auto& to_do = (is_pre)? pre: post;
		to_do.push_back([=](Tree* t){
				if(pred(t)){
					auto ret = matches(m, t);
					if(!ret.result){ return t; }
					mod(ret.captures);
					return tree::perform_replacement(t, ret.captures);
				}
				return t;
			});
	}

	void add(Matcher* m, std::function<void(std::map<int, TreeSequence>&)> mod, Tree* r, bool is_pre){
		add(default_pred, m, mod, r, is_pre);
	}

	void add(Matcher* m, Tree* r, bool is_pre){
		add(default_pred, m, default_mod, r, is_pre);
	}

	void add(Matcher* m, std::function<void(std::map<int, TreeSequence>&)> mod, Tree* r){
		add(default_pred, m, mod, r, false);
		add(default_pred, m, mod, r, true);
	}

	void add(Matcher* p, Tree* r){
		add(p, r, false);
		add(p, r, true);
	}

	void add(Matcher* m, std::function<Tree*(std::map<int, TreeSequence>&)> mod, bool is_pre){
		auto& to_do = (is_pre)? pre: post;
		to_do.push_back([=](Tree* t){
				auto ret = matches(m, t);
				if(ret.result){
					t = mod(ret.captures);
				}
				return t;
			});
	}

	void add(Matcher* m, std::function<Tree*(std::map<int, TreeSequence>&)> mod){
		add(m, mod, false);
		add(m, mod, true);
	}

	template<typename T1, typename T2,  typename... Rest>
	void add(std::tuple<T1, T2>& t, Rest... r){
		add(std::get<0>(t), std::get<1>(t));
		add(r...);
	}

	template<typename T1, typename T2, typename T3, typename... Rest>
	void add(std::tuple<T1, T2, T3>& t, Rest... r){
		add(std::get<0>(t), std::get<1>(t), std::get<2>(t));
		add(r...);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename... Rest>
	void add(std::tuple<T1, T2, T3, T4>& t, Rest... r){
		add(std::get<0>(t), std::get<1>(t), std::get<2>(t), std::get<3>(t));
		add(r...);
	}

	template<typename T1, typename T2>
	void add(std::tuple<T1, T2>& t){
		add(std::get<0>(t), std::get<1>(t));
	}

	template<typename T1, typename T2, typename T3>
	void add(std::tuple<T1, T2, T3>& t){
		add(std::get<0>(t), std::get<1>(t), std::get<2>(t));
	}

	template<typename T1, typename T2, typename T3, typename T4>
	void add(std::tuple<T1, T2, T3, T4>& t){
		add(std::get<0>(t), std::get<1>(t), std::get<2>(t), std::get<3>(t));
	}
private:
	Tree* recurse(Tree* val){
		ListOfTrees* lt = dynamic_cast<ListOfTrees*>(val);
		if(!lt){
			assert(dynamic_cast<StringLeaf*>(val));
			return val;
		}
		for(int i = 0; i < lt->subtrees.size(); ++i){
			lt->subtrees[i]=this->operator()(lt->subtrees[i]);
		}
		return lt;
	}
public:
	Tree* operator()(Tree* val){ // inplace modification
		for(auto& f : pre){
			val = f(val);
		}
		val = recurse(val);
		for(auto& f: post){
			val = f(val);
		}
		return val;
	}
};

