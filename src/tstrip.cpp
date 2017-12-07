#include <iostream>
#include <map>
#include <cassert>
#include <string>
#include <streambuf>
#include <string>
#include "transformer.h"


int main(int argc, char** argv){
	std::ios_base::sync_with_stdio(false);
	std::string tree_str((std::istreambuf_iterator<char>(std::cin)), std::istreambuf_iterator<char>());
	if(tree_str[tree_str.length()-1] == '\n'){
		tree_str.pop_back();
	}
	auto tree = tree::parse(tree_str);
	tree->print(std::cout, true); std::cout << '\n';

}
