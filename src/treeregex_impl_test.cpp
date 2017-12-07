#include <iostream>
#include "treeregex_impl.h"

int main(){
	treeregex::parse("(%if\\@\\)@%)")->print(std::cout); std::cout << '\n';
	treeregex::parse("\\(\\)")->print(std::cout); std::cout << '\n';
	treeregex::parse("()")->print(std::cout); std::cout << '\n';
	treeregex::parse("[a-z]*")->print(std::cout); std::cout << '\n';
	treeregex::parse("(@|.)*")->print(std::cout); std::cout << '\n';
	treeregex::parse("abc(@|.|a*|b|c*|d)*def")->print(std::cout); std::cout << '\n';
	treeregex::parse("(@*)")->print(std::cout); std::cout << '\n';

	treeregex::parse("(%abc%)")->print(std::cout); std::cout << '\n';
	treeregex::parse("(%a(%b%)c%)")->print(std::cout); std::cout << '\n';
	treeregex::parse("(%a(%%)c%)")->print(std::cout); std::cout << '\n';
	treeregex::parse("(%(%b%)c%)")->print(std::cout); std::cout << '\n';
	treeregex::parse("(%(%b%)%)")->print(std::cout); std::cout << '\n';
	treeregex::parse("(%(%%)%)")->print(std::cout); std::cout << '\n';
	treeregex::parse("(a(a)a)*|aaaa")->print(std::cout); std::cout << '\n';
	treeregex::parse("(a(a)*a)*|aaaa")->print(std::cout); std::cout << '\n';
	treeregex::parse("(aa*a)*|aaaa")->print(std::cout); std::cout << '\n';

	treeregex::parse("(%@%)")->print(std::cout); std::cout << '\n';
	treeregex::parse("(%(&@(a)&)%)")->print(std::cout); std::cout << '\n';

	treeregex::parse("[^abc]")->print(std::cout); std::cout << '\n';
	treeregex::parse("[abc]")->print(std::cout); std::cout << '\n';
	treeregex::parse(".")->print(std::cout); std::cout << '\n';
}
