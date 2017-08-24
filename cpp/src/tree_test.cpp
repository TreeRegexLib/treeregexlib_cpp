#include <iostream>

#include "tree.h"

int main(){
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
	std::vector<TreeSequence*> captures1, captures2;
	TreeSequence* empty = new TreeSequence();
	TreeSequence* something = new TreeSequence();
	something->trees.push_back(tree::parse("(%abc%)"));
	captures1.push_back(empty);captures1.push_back(empty); captures1.push_back(empty);
	captures2.push_back(empty);captures2.push_back(empty); captures2.push_back(something);
	tree::perform_replacement(tree::parse_replacement("(%b$2a%)"), captures1)->print(std::cout); std::cout << '\n';
	tree::perform_replacement(tree::parse_replacement("(%(%a%)$2a%)"), captures1)->print(std::cout); std::cout << '\n';
	tree::perform_replacement(tree::parse_replacement("(%(%a%)$2(%bc%)%)"), captures1)->print(std::cout); std::cout << '\n';
	tree::perform_replacement(tree::parse_replacement("(%a$2(%bc%)%)"), captures1)->print(std::cout); std::cout << '\n';
	tree::perform_replacement(tree::parse_replacement("(%b$2a%)"), captures2)->print(std::cout); std::cout << '\n';
	tree::perform_replacement(tree::parse_replacement("(%(%a%)$2a%)"), captures2)->print(std::cout); std::cout << '\n';
	tree::perform_replacement(tree::parse_replacement("(%(%a%)$2(%bc%)%)"), captures2)->print(std::cout); std::cout << '\n';
	tree::perform_replacement(tree::parse_replacement("(%a$2(%bc%)%)"), captures2)->print(std::cout); std::cout << '\n';
}
