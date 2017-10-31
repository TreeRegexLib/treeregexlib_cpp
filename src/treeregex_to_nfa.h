#ifndef TREEREGEX_TO_NFA_H
#define TREEREGEX_TO_NFA_H

#include <bitset>
#include <utility>
#include <fstream>
#include <queue>
#include "treeregex_impl.h"
#include "graph.h"


namespace { static int Node_id_count = 0;}
struct Node_data {
	std::bitset<256> epsilon_closure;
	int id;
	int matching_group;
	Node_data():id(Node_id_count++),epsilon_closure(), matching_group(0){
		assert(id < epsilon_closure.size());
	}
};

typedef Graph<Node_data, std::shared_ptr<Matcher>> NFA_Graph;
typedef Node<Node_data, std::shared_ptr<Matcher>> NFA_Node;
typedef Edge<Node_data, std::shared_ptr<Matcher>> NFA_Edge;
struct NFA {
	std::shared_ptr<NFA_Graph> g;
	NFA_Node* start;
	NFA_Node* end;
	int captures;
	int highest_id;
};

namespace {
NFA_Node& addNodeWrap(NFA_Graph& g){
	NFA_Node& ret = g.addNode();
	assert(ret.val.id+1 == g.nodes.size());
	return ret;
}

int max_matching_group = 0;
NFA_Node& _to_nfa(std::shared_ptr<Matcher> m, NFA_Graph& g, NFA_Node& starts);
NFA_Node& to_nfa(std::shared_ptr<Matcher> m, NFA_Graph& g, NFA_Node& starts){
	auto& ret =  _to_nfa(m, g, starts);
	assert(starts.val.matching_group == 0 || m->matching_group == 0);
	if(m->matching_group){
		starts.val.matching_group = m->matching_group;
	}
	assert(ret.val.matching_group == 0 || m->matching_group == 0);
	if(m->matching_group){
		ret.val.matching_group = -m->matching_group;
	}
	/*if(starts.val.matching_group != 0){
		std::cout << "Assigning " << starts.val.id << " matching group " << starts.val.matching_group << '\n';
	}
	if(ret.val.matching_group != 0){
		std::cout << "Assigning " << ret.val.id << " matching group " << ret.val.matching_group << '\n';
	}*/
	max_matching_group = std::max(max_matching_group, m->matching_group);
	return ret;
}

NFA_Node& _to_nfa(std::shared_ptr<Matcher> m, NFA_Graph& g, NFA_Node& starts){
	std::shared_ptr<CharacterSetMatcher> cm = nullptr;
	std::shared_ptr<StringMatcher> sm = nullptr;
	std::shared_ptr<TreeMatcher> tm = nullptr;
	std::shared_ptr<EpsilonMatcher> em = nullptr;
	std::shared_ptr<MatcherOperator> mo = nullptr;

	sm = (m)->toStringMatcher();
	if(!sm){
	tm = (m)->toTreeMatcher();
	if(!tm){
	em = (m)->toEpsilonMatcher();
	if(!em){
	cm = (m)->toCharacterSetMatcher();
	if(!cm){
	mo = (m)->toMatcherOperator();
	}}}}
	//std::cout << "Working on:"; m->print(std::cout); std::cout <<'\n';

	std::vector<NFA_Node*> outs;
	if(sm != nullptr || tm != nullptr || em != nullptr || cm != nullptr){
		auto& new_n = addNodeWrap(g);
		g.addEdge(starts, new_n, m);
		return new_n;
	} else {
		//std::cout << mo << '\n';
		assert(mo);
		if(mo->type == &CONCATENATION){
			//std::cout << "CONCAT\n";

			auto& end = addNodeWrap(g);
			auto& end_tmp = to_nfa(mo->after_or_right, g, to_nfa(mo->before_or_left, g, starts));
			g.addEdge(end_tmp, end, EpsilonMatcher::get());
			return end;
		} else if(mo->type == &UNION){
			//std::cout << "UNION\n";
			auto& start1 = addNodeWrap(g);
			auto& start2 = addNodeWrap(g);
			g.addEdge(starts, start1, EpsilonMatcher::get());
			g.addEdge(starts, start2, EpsilonMatcher::get());
			auto& end1 = to_nfa(mo->before_or_left, g, start1);
			auto& end2  = to_nfa(mo->after_or_right, g, start2);
			auto& end = addNodeWrap(g);
			g.addEdge(end1, end, EpsilonMatcher::get());
			g.addEdge(end2, end,  EpsilonMatcher::get());
			return end;
		} else if(mo->type == &KLEENE_STAR){
			//std::cout << "KLEENE\n";
			auto& start_tmp = addNodeWrap(g);

			auto& end_tmp = to_nfa(mo->before_or_left, g, start_tmp);

			auto& end = addNodeWrap(g);

			g.addEdge(starts, end, EpsilonMatcher::get());
			g.addEdge(starts, start_tmp, EpsilonMatcher::get());
			g.addEdge(end_tmp, end, EpsilonMatcher::get());
			g.addEdge(end_tmp, start_tmp, EpsilonMatcher::get());

			return end;
		} else {
			assert(false);
		}
	}
}
}

NFA matcher_to_nfa(std::shared_ptr<Matcher> m){
	Node_id_count = 0;
	max_matching_group = 0;
	std::shared_ptr<NFA_Graph> g= std::make_shared<NFA_Graph>();
	auto& start = addNodeWrap(*g);
	auto& end = to_nfa(m, *g, start);

	// go through nodes and create epsilon closure
	std::bitset<256> done_nodes;

	// we could do this faster
	// for sizes like 256, it shouldnt matter too much....
	for(auto& n: g->nodes){
		std::queue<NFA_Node*> node_queue;
		node_queue.push(&n);
		while(!node_queue.empty()){
			auto* working_on = node_queue.front(); node_queue.pop();
			n.val.epsilon_closure[working_on->val.id] = true;
			for(auto* e : working_on->outgoing_edges){
				NFA_Node& to_add = e->end;

				// if we have already added it, we can skip it
				if(n.val.epsilon_closure[to_add.val.id]){
					continue;
				}
				assert(e->val);
				if((e->val->toEpsilonMatcher())){
					// if that node is done, we can just accumulate its epsilon closure
					if(done_nodes[to_add.val.id]){
						n.val.epsilon_closure |= to_add.val.epsilon_closure;
					} else {
						node_queue.push(&to_add);
					}
				}
			}
		}

		done_nodes[n.val.id] = true;
	}

	return {g, &start, &end, max_matching_group, Node_id_count-1};
}

void nfa_to_dot(NFA* nfa, int id = -1){
	static int i = 0;
	if(id == -1){ id = i++; }
	std::ofstream out("nfa"+std::to_string(id)+".dot");

	out << "digraph NFA {\n";
	for(NFA_Node& n : nfa->g->nodes){
		/*std::cout << "For node id: " << n.val.id << '\n';
		for(int i = 0; i < n.val.epsilon_closure.size(); ++i){
			if(n.val.epsilon_closure[i]){
				std::cout << "\t"<< n.val.id << ':' << i << "\n";
			}
		}*/
		out << "\tnode" << n.val.id << "[label=\"" << n.val.id;
		if(n.val.matching_group>0){
			out << "(" << n.val.matching_group;
		} else if(n.val.matching_group < 0){
			out << ")" << n.val.matching_group;
		}
		out << "\", shape=\"";
		if(n.val.id == nfa->end->val.id){
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
	out << "\tstart->" << "node" << nfa->start->val.id << ";\n";
	out << "}\n";
}

#endif /* TREEREGEX_TO_NFA_H */
