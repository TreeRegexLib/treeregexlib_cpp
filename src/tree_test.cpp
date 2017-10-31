#include <iostream>

#include "tree.h"

int main(){
	tree::parse("(%()%)")->print(std::cout); std::cout << '\n';
	tree::parse("(%\\(\\)%)")->print(std::cout); std::cout << '\n';
	tree::parse("(%abc%)")->print(std::cout); std::cout << '\n';
	tree::parse("(%a(%b%)c%)")->print(std::cout); std::cout << '\n';
	tree::parse("(%a(%%)c%)")->print(std::cout); std::cout << '\n';
	tree::parse("(%(%b%)c%)")->print(std::cout); std::cout << '\n';
	tree::parse("(%(%b%)%)")->print(std::cout); std::cout << '\n';
	tree::parse("(%(%%)%)")->print(std::cout); std::cout << '\n';

	tree::parse("(%$1%)")->print(std::cout); std::cout << '\n';
	tree::parse("(%$10%)")->print(std::cout); std::cout << '\n';
	tree::parse("(%b$10a%)")->print(std::cout); std::cout << '\n';
	tree::parse("(%(%a%)$10a%)")->print(std::cout); std::cout << '\n';
	tree::parse("(%(%a%)$10(%bc%)%)")->print(std::cout); std::cout << '\n';
	tree::parse("(%a$10(%bc%)%)")->print(std::cout); std::cout << '\n';

	std::cout << "STARTING REPLACEMENT PARSING!\n";
	tree::parse_replacement("(%$1%)")->print(std::cout); std::cout << '\n';
	tree::parse_replacement("(%$10%)")->print(std::cout); std::cout << '\n';
	tree::parse_replacement("(%b$10a%)")->print(std::cout); std::cout << '\n';
	tree::parse_replacement("(%(%a%)$10a%)")->print(std::cout); std::cout << '\n';
	tree::parse_replacement("(%(%a%)$10(%bc%)%)")->print(std::cout); std::cout << '\n';
	tree::parse_replacement("(%a$10(%bc%)%)")->print(std::cout); std::cout << '\n';
	std::cout << "STARTING REPLACEMENT REPLACING!\n";
	std::map<int, TreeSequence> captures1, captures2;
	TreeSequence empty;// = new TreeSequence();
	TreeSequence something;// = new TreeSequence();
	something.trees.push_back(tree::parse("(%abc%)"));
	captures1[0] = empty ;captures1[1]=empty; captures1[2] = empty;
	captures2[0] = empty ;captures2[1]=empty; captures2[2] = something;
	tree::perform_replacement(tree::parse_replacement("(%b$2a%)"), captures1)->print(std::cout); std::cout << '\n';
	tree::perform_replacement(tree::parse_replacement("(%(%a%)$2a%)"), captures1)->print(std::cout); std::cout << '\n';
	tree::perform_replacement(tree::parse_replacement("(%(%a%)$2(%bc%)%)"), captures1)->print(std::cout); std::cout << '\n';
	tree::perform_replacement(tree::parse_replacement("(%a$2(%bc%)%)"), captures1)->print(std::cout); std::cout << '\n';
	tree::perform_replacement(tree::parse_replacement("(%b$2a%)"), captures2)->print(std::cout); std::cout << '\n';
	tree::perform_replacement(tree::parse_replacement("(%(%a%)$2a%)"), captures2)->print(std::cout); std::cout << '\n';
	tree::perform_replacement(tree::parse_replacement("(%(%a%)$2(%bc%)%)"), captures2)->print(std::cout); std::cout << '\n';
	tree::perform_replacement(tree::parse_replacement("(%a$2(%bc%)%)"), captures2)->print(std::cout); std::cout << '\n';
}
