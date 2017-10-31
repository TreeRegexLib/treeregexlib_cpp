#include <iostream>
#include "treeregex_matching.h"

void print_matching_results(match_results&& mr){
	std::cout << "MATCHING RESULT: " << mr.result << '\n';
	if(mr.result){
		std::cout << "GOING THROUGH CAPTURES:\n";
	for(auto& p : mr.captures){
		std::cout << "\tCAPTURE: " << p.first << " : "; p.second.print(std::cout, 0); std::cout << '\n';
	}
	}
}

void print_perform_matches(match_results&& mr, std::shared_ptr<Tree> replacement_tree){
	if(mr.result)
	tree::perform_replacement(replacement_tree, mr.captures)->print(std::cout); std::cout << '\n';
}

int main(){
	std::cout << "TEST: " << matches(treeregex::parse("(%aaaa%)"), tree::parse("(%aaaa%)")) << '\n';
	std::cout << "TEST: " << matches(treeregex::parse("(%(a(a)a)*|aaaa%)"), tree::parse("(%aaaa%)")) << '\n';
	std::cout << "TEST: " << matches(treeregex::parse("(@|.)*"), tree::parse("(%abc%)")) << '\n';
	std::cout << "TEST: " << matches(treeregex::parse("((%abc%)|.)*"), tree::parse("(%abc%)")) << '\n';
	std::cout << "TEST: " << matches(treeregex::parse("((&abc&)|.)*"), tree::parse("(%abc%)")) << '\n';
	std::cout << "TEST: " << matches(treeregex::parse("(&abc&)"), tree::parse("(%abc%)")) << '\n';
	std::cout << "TEST: " << matches(treeregex::parse("(&abc&)"), tree::parse("(%(%abc%)%)")) << '\n';
	std::cout << "TEST: " << matches(treeregex::parse("(&abc&)"), tree::parse("(%abc(%abc%)%)")) << '\n';

	std::cout << "NEG TEST: " << matches(treeregex::parse("(%(aaa)*|(aaaa)*%)"), tree::parse("(%aaaaaaa%)")) << '\n';

	std::cout << "CAPTURES TEST:\n";
	print_matching_results(matches(treeregex::parse("(&(@|a)*&)"), tree::parse("(%(%a(%b%)%)bc%)")));
	print_matching_results(matches(treeregex::parse("(%(@|.)*%)"), tree::parse("(%aaa%)")));
	print_matching_results(matches(treeregex::parse("(%([a-z]*)%)"), tree::parse("(%bc%)")));
	print_matching_results(matches(treeregex::parse("(%([a-z]*)%)"), tree::parse("(%abz%)")));
	print_matching_results(matches(treeregex::parse("(%\\(\\)%)"), tree::parse("(%()%)")));
	print_matching_results(matches(treeregex::parse("(%\\(\\)%)"), tree::parse("(%\\(\\)%)")));
	print_matching_results(matches(treeregex::parse("(%()%)"), tree::parse("(%\\(\\)%)")));
	print_matching_results(matches(treeregex::parse("(%BC--:return\\(@\\)%)"),
				tree::parse("(%BC--:return((%BC--:(%BC--:s%)((%BC--:(%BC--:(%BC--:(%BC--:x%)%)%)%))%))%)")));

	std::cout << "REPLACING TEST:\n";
	print_perform_matches(
			matches(treeregex::parse("(&(@|a)*&)"), tree::parse("(%(%a(%b%)%)bc%)")),
			tree::parse_replacement("(%$1$3$2%)")
			);
	print_perform_matches(
			matches(treeregex::parse("(@|a)*"), tree::parse("(%(%a(%b%)%)bc%)")),
			tree::parse_replacement("(%$1%)")
			);
	print_perform_matches(
			matches(treeregex::parse("(%(@|.)*%)"), tree::parse("(%abc%)")),
			tree::parse_replacement("(%$1%)")
			);
}
