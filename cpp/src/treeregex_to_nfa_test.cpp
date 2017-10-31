#include <iostream>
#include <fstream>
#include "treeregex_to_nfa.h"

void nfa_to_dot(NFA&& nfa){
	static int i = 0;
	std::ofstream out("nfa"+std::to_string(i++)+".dot");

	out << "digraph NFA {\n";
	for(NFA_Node& n : nfa.g->nodes){
		std::cout << "For node id: " << n.val.id << '\n';
		for(int i = 0; i < n.val.epsilon_closure.size(); ++i){
			if(n.val.epsilon_closure[i]){
				std::cout << "\t"<< n.val.id << ':' << i << "\n";
			}
		}
		out << "\tnode" << n.val.id << "[label=\"" << n.val.id;
		if(n.val.matching_group>0){
			out << "(" << n.val.matching_group;
		} else if(n.val.matching_group < 0){
			out << ")" << n.val.matching_group;
		}
		out << "\", shape=\"";
		if(n.val.id == nfa.end->val.id){
			out << "doublecircle";
		} else {
			out << "circle";
		}
		out << "\"];\n";

		for(NFA_Edge* e : n.outgoing_edges){
			out << "\tnode" << n.val.id << "->" << "node" << e->end.val.id
				<< "[label=\"";
			e->val->print(out);
			out << "\"];\n";
		}
	}

	out << "\tstart[label=\"start\",style=\"invisible\"];\n";
	out << "\tstart->" << "node" << nfa.start->val.id << ";\n";
	out << "}\n";
}

int main(){
	nfa_to_dot(matcher_to_nfa(treeregex::parse("(@|.)*"))); //

	nfa_to_dot(matcher_to_nfa(treeregex::parse("(a(a)a)*|aaaa"))); //
	nfa_to_dot(matcher_to_nfa(treeregex::parse("abc(@|.|a*|b|c*|d)*def"))); //

	nfa_to_dot(matcher_to_nfa(treeregex::parse("abcd*def"))); //
	nfa_to_dot(matcher_to_nfa(treeregex::parse("abc(d)*def"))); //
	nfa_to_dot(matcher_to_nfa(treeregex::parse("ab(cd)*def"))); //
	nfa_to_dot(matcher_to_nfa(treeregex::parse("abc(d*)def"))); //
	nfa_to_dot(matcher_to_nfa(treeregex::parse("ab(cd*)def"))); //

	nfa_to_dot(matcher_to_nfa(treeregex::parse("(@|.)*")));
	nfa_to_dot(matcher_to_nfa(treeregex::parse("abcdef"))); //
	nfa_to_dot(matcher_to_nfa(treeregex::parse("abcd*def"))); //
	nfa_to_dot(matcher_to_nfa(treeregex::parse("abc(d)*def"))); //
	nfa_to_dot(matcher_to_nfa(treeregex::parse("abc(@|d)*def"))); //
	nfa_to_dot(matcher_to_nfa(treeregex::parse("abc(@|.|a*|b|c*|d)*def"))); //
	nfa_to_dot(matcher_to_nfa(treeregex::parse("(@*)")));

	nfa_to_dot(matcher_to_nfa(treeregex::parse("(%abc%)")));
	nfa_to_dot(matcher_to_nfa(treeregex::parse("(%a(%b%)c%)")));
	nfa_to_dot(matcher_to_nfa(treeregex::parse("(%a(%%)c%)")));
	nfa_to_dot(matcher_to_nfa(treeregex::parse("(%(%b%)c%)")));
	nfa_to_dot(matcher_to_nfa(treeregex::parse("(%(%b%)%)")));
	nfa_to_dot(matcher_to_nfa(treeregex::parse("(%(%%)%)")));
}
