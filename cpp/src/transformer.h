#ifndef TRANSFORMER_H
#define TRANSFORMER_H


#include <utility>
#include <functional>
#include "tree.h"
#include "treeregex_impl.h"
#include "treeregex_matching.h"

std::function<bool(std::shared_ptr<Tree>)> default_pred = [](std::shared_ptr<Tree>){return true;};
std::function<void(std::map<int, TreeSequence>&)> default_mod = [](std::map<int, TreeSequence>&){};

class Transformer {
public:
	std::vector<std::function<std::shared_ptr<Tree>(std::shared_ptr<Tree>)>> pre;
	std::vector<std::function<std::shared_ptr<Tree>(std::shared_ptr<Tree>)>> post;

	void add(std::function<bool(std::shared_ptr<Tree>)> pred, std::shared_ptr<Matcher> m, std::function<void(std::map<int, TreeSequence>&)> mod, std::shared_ptr<Tree> r, bool is_pre){
		auto& to_do = (is_pre)? pre: post;
		to_do.push_back([=](std::shared_ptr<Tree> t){
				if(pred(t)){
					auto ret = matches(m, t);
					//m->print(std::cout); std::cout << " with "; t->print(std::cout); std::cout << '\n';
					//std::cout << "ret : " << ret.result << '\n';
					if(!ret.result){ return t; }
					mod(ret.captures);
					//std::cout << "Captures:\n";
					//for(auto& p : ret.captures){ std::cout << p .first ; p.second.print(std::cout); std::cout << '\n';}
					std::shared_ptr<Tree> out = tree::perform_replacement(r, ret.captures);
					//std::cout << "AFTER: " ; out->print(std::cout) ; std::cout <<'\n';
					//exit(1);
					return out;
				}
				return t;
			});
	}

	void add(std::function<std::shared_ptr<Tree>(std::shared_ptr<Tree>)> to_change, bool is_pre){
		auto& to_do = (is_pre)? pre: post;
		to_do.push_back(to_change);
	}

	void add(std::function<bool(std::shared_ptr<Tree>)> pred, const std::string m, std::function<void(std::map<int, TreeSequence>&)> mod, const std::string r, bool is_pre){
		add(pred, treeregex::parse(m), mod, tree::parse_replacement(r), is_pre);
	}

	void add(std::shared_ptr<Matcher> m, std::function<void(std::map<int, TreeSequence>&)> mod, std::shared_ptr<Tree> r, bool is_pre){
		add(default_pred, m, mod, r, is_pre);
	}

	void add(const std::string m, std::function<void(std::map<int, TreeSequence>&)> mod, const std::string r, bool is_pre){
		return add(treeregex::parse(m), mod, tree::parse_replacement(r), is_pre);
	}

	void add(std::shared_ptr<Matcher> m, std::shared_ptr<Tree> r, bool is_pre){
		add(default_pred, m, default_mod, r, is_pre);
	}

	void add(const std::string m, const std::string r, bool is_pre){
		add(default_pred, treeregex::parse(m), default_mod, tree::parse_replacement(r), is_pre);
	}

	void add(std::shared_ptr<Matcher> m, std::function<void(std::map<int, TreeSequence>&)> mod, std::shared_ptr<Tree> r){
		add(default_pred, m, mod, r, false);
		add(default_pred, m, mod, r, true);
	}

	void add(const std::string m, std::function<void(std::map<int, TreeSequence>&)> mod, const std::string r){
		add(treeregex::parse(m), mod, tree::parse_replacement(r));
	}

	void add(std::shared_ptr<Matcher> p, std::shared_ptr<Tree> r){
		add(p, r, false);
		add(p, r, true);
	}

	void add(const std::string p, const std::string r){
		add(treeregex::parse(p), tree::parse_replacement(r));
	}

	void add(std::shared_ptr<Matcher> m, std::function<std::shared_ptr<Tree>(std::map<int, TreeSequence>&)> mod, bool is_pre){
		auto& to_do = (is_pre)? pre: post;
		to_do.push_back([=](std::shared_ptr<Tree> t){
				auto ret = matches(m, t);
				if(ret.result){
					t = mod(ret.captures);
				}
				return t;
			});
	}

	void add(const std::string m , std::function<std::shared_ptr<Tree>(std::map<int, TreeSequence>&)> mod, bool is_pre){
		add(treeregex::parse(m), mod, is_pre);
	}

	void add(std::shared_ptr<Matcher> m, std::function<void(std::map<int, TreeSequence>&, bool)> mod, bool is_pre){
		auto& to_do = (is_pre)? pre: post;
		to_do.push_back([=](std::shared_ptr<Tree> t){
				auto ret = matches(m, t);
				if(ret.result){
					mod(ret.captures, true);
				}
				return t;
			});
	}

	void add(const std::string m , std::function<void(std::map<int, TreeSequence>&, bool)> mod, bool is_pre){
		add(treeregex::parse(m), mod, is_pre);
	}

	void add(std::shared_ptr<Matcher> m, std::function<std::shared_ptr<Tree>(std::map<int, TreeSequence>&)> mod){
		add(m, mod, false);
		add(m, mod, true);
	}

	void add(const std::string m_, std::function<std::shared_ptr<Tree>(std::map<int, TreeSequence>&)> mod){
		add(treeregex::parse(m_), mod);
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
	std::shared_ptr<Tree> recurse(std::shared_ptr<Tree> val){
		std::shared_ptr<ListOfTrees> lt = val->toListOfTrees();
		if(!lt){
			assert(val->toStringLeaf());
			return val;
		}
		for(int i = 0; i < lt->subtrees.size(); ++i){
			lt->subtrees[i]=this->operator()(lt->subtrees[i]);
		}
		return lt;
	}

public:
	std::shared_ptr<Tree> operator()(std::shared_ptr<Tree> val){
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

#endif /*TRANSFORMER_H*/
