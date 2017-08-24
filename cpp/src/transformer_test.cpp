#include <iostream>
#include <map>
#include <cassert>
#include <string>
#include "transformer.h"


int main(int argc, char** argv){
	std::ios_base::sync_with_stdio(false);
	Tree* tree = tree::parse("(%let t1 = (% let b = (% 3 %) %)%)");

	Transformer t;
	std::map<std::string, std::string> mapping = {{"t1", "t0"}};

	t.add(treeregex::parse("(%let ([^ ]/*) = @ in @%)"), [&](std::map<int, TreeSequence>& v){
			auto it =  mapping.find(v[0].trees[0]->str());
			if(it != mapping.end()){
				v[0].trees[0] = tree::parse(it->second);
			}
			}, tree::parse_replacement("(%let $1 = $2 in $3%)"), false);
	t(tree)->print(std::cout); std::cout << '\n';
}
