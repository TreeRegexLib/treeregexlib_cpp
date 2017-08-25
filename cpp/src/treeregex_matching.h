#ifndef TREEREGEX_MATCHING_H
#define TREEREGEX_MATCHING_H

#include <map>
#include <cstdint>
#include "treeregex_to_nfa.h"
#include "tree.h"

struct match_results {
	bool result;
	std::map<int, TreeSequence> captures;
	operator bool(){ return result; }
};

match_results matches(Matcher* m, Tree* t);
namespace {
NFA* get_cached_nfa(Matcher* m){
	static std::map<Matcher*, NFA> cache;
	auto it= cache.find(m);
	if(it == cache.end()){
		cache[m] = matcher_to_nfa(m);
	}
	return &cache[m];
}

void get_reverse_epsilon_path(NFA* nfa, int start, int end, std::vector<int>& to_add_to){
	std::vector<std::array<char, 256>> history;
	std::bitset<256> set_of_positions;

	//std::cout << "GET_PATH: " << start <<  ' ' << end << '\n';
	set_of_positions[start] = true;
	int final_id = nfa->highest_id;
	//while(!set_of_positions[end]){
	do{
		//std::cout << "LOOP: " << history.size() << '\n';
		assert(set_of_positions.count());
		std::bitset<256> next_set_of_positions;
		history.push_back(std::array<char, 256>());
		for(int i = 0; i < final_id+1; ++i){
			if(!set_of_positions[i]) { continue; }
			NFA_Node& n = nfa->g->nodes[i];
			for(NFA_Edge* e : n.outgoing_edges){
				if(!(e->val->toEpsilonMatcher())){ continue; }
				next_set_of_positions[e->end.val.id] = true;
				history.back()[e->end.val.id] = i;
			}
		}
		set_of_positions = next_set_of_positions;
	}
	while(!set_of_positions[end]);

	assert(to_add_to.back() == end);
	for(int i = history.size()-1; i>=0; --i){
		end = history[i][end];
		to_add_to.push_back(end);
	}
	assert(to_add_to.back() == start);
}

match_results matches(NFA* nfa, Tree* t){
	static int count = 1;
	//std::cout << "MATCHING USING NFA!: " << count << "\n";
	//nfa_to_dot(nfa,  count);
	count++;
	ListOfTrees* lt = t->toListOfTrees();
	assert(lt);

	std::bitset<256> set_of_nfa_positions;
	//std::cout << nfa->start << '\n';
	set_of_nfa_positions[nfa->start->val.id] = true;

	struct path_frag {
		uint8_t non_epsilon_pos;
		uint8_t source;
	};
	std::vector<std::array<path_frag, 256>> history(lt->subtrees.size()+1);
	std::map<std::pair<int, int>, std::map<int, TreeSequence>> edge_to_captures;
	int final_id = nfa->highest_id;

	int tree_index = 0;
	for(Tree* t : lt->subtrees){
		std::bitset<256> next_set_of_nfa_positions;
		for(int i = 0; i < final_id+1; ++i){
			if(!set_of_nfa_positions[i]){ continue; }
			std::bitset<256> true_set_of_nfa_positions;
			true_set_of_nfa_positions[i] = true;

			NFA_Node& n_s = nfa->g->nodes[i];
			true_set_of_nfa_positions |= n_s.val.epsilon_closure;
			for(int j = 0; j < final_id+1; ++j){
				if(!true_set_of_nfa_positions[j]){continue; }

				//std::cout << '\t' << j << '\n';
				NFA_Node& n = nfa->g->nodes[j];
				for(NFA_Edge* e : n.outgoing_edges){
					if((e->val->toEpsilonMatcher())){
						continue;
					}
					match_results ret = matches(e->val, t);
					if(ret.result){
						next_set_of_nfa_positions[e->end.val.id] = true;
						if(ret.captures.size()){
							edge_to_captures.emplace(
								std::make_pair(std::make_pair(e->begin.val.id, e->end.val.id),
								std::move(ret.captures)));
							//std::cout << "INSERTING: " << e->begin.val.id << ' ' <<  e->end.val.id << ' ' << edge_to_captures.size() << '\n';
						}
						history[tree_index][e->end.val.id].non_epsilon_pos = j+1;
						history[tree_index][e->end.val.id].source = i+1;
					}
				}
			}
		}
		//std::cout << '\n';
		if(next_set_of_nfa_positions.count() == 0){
			return {false, {}};
		}
		set_of_nfa_positions = next_set_of_nfa_positions;
		tree_index++;
	}

	bool ret = false;
	for(int i = 0; i < final_id+1; ++i){
		if(!set_of_nfa_positions[i]){ continue; }
		std::bitset<256> true_set_of_nfa_positions;
		true_set_of_nfa_positions[i] = true;

		NFA_Node& n_s = nfa->g->nodes[i];
		true_set_of_nfa_positions |= n_s.val.epsilon_closure;
		if(true_set_of_nfa_positions[nfa->end->val.id]){
			ret = true;
			history[tree_index][nfa->end->val.id].non_epsilon_pos = nfa->end->val.id+1;
			history[tree_index][nfa->end->val.id].source = i+1;
			break;
		}
	}

	if(!ret){ return {ret,}; }
	int pos = nfa->end->val.id;
	std::vector<int> find_path_rev;

	struct capture {
		int tree_index_start;
		int tree_index_end;
	};
	std::map<int,capture> captures_by_index;
	std::map<int, int> dangling_capture_ends;
	int last_processed_find_path_pos = 0;
	std::map<int, TreeSequence> ret_caps;
	while(pos != nfa->start->val.id){
		auto& frag = history[tree_index][pos];
		//std::cout << pos << ' ' << (int)history[tree_index][pos].source-1 << ' ' << (int)history[tree_index][pos].non_epsilon_pos-1 << '\n';
		find_path_rev.push_back(frag.non_epsilon_pos-1);
		if(frag.source != frag.non_epsilon_pos){
			get_reverse_epsilon_path(nfa, frag.source-1, find_path_rev.back(), find_path_rev);
		}

		while(last_processed_find_path_pos != find_path_rev.size()){
			int node_id = find_path_rev[last_processed_find_path_pos];
			if(last_processed_find_path_pos>=1){
				int farther_along_node_id = find_path_rev[last_processed_find_path_pos-1];
				auto p = std::make_pair(node_id, farther_along_node_id);
				//std::cout << "EDGE CAPTURE CHECK: " << node_id << ' ' << farther_along_node_id << ' ' << edge_to_captures.size() << '\n';
				for(auto& captures : edge_to_captures[p]){
					//std::cout << "FOUND CAPTURE!\n";
					ret_caps.insert(captures);
				}
			}
			last_processed_find_path_pos++;
			NFA_Node& n = nfa->g->nodes[node_id];
			int mg = n.val.matching_group;
			if(mg){
				//std::cout << "NODE: " << node_id << " has matching group: " << mg << '\n';
				if(mg < 0){
					dangling_capture_ends[-mg] = tree_index+1;
				} else {
					if(dangling_capture_ends[mg]){
						captures_by_index[mg].tree_index_start = tree_index;
						captures_by_index[mg].tree_index_end = dangling_capture_ends[mg]-1;
					}
				}
			}
		}
		pos = history[tree_index][pos].source-1;
		tree_index--;
	}

	for(auto& p : captures_by_index){
		//std::cout << p.first << ": " << p.second.tree_index_start << ' ' << p.second.tree_index_end << '\n';
		ret_caps[p.first] = { std::vector<Tree*>(
				lt->subtrees.begin()+p.second.tree_index_start,
				lt->subtrees.begin()+p.second.tree_index_end
				)};
	}

	//std::cout << "NFA: " << ret_caps.size() << " from " << nfa->captures << '\n';
	return {ret, ret_caps};
}
}

match_results matches(Matcher* m, Tree* t){
	//std::cout << "MATCHING: "; m->print(std::cout); std::cout << " against "; t->print(std::cout); std::cout << '\n';
	ListOfTrees* lt = t->toListOfTrees();
	StringLeaf* sl = nullptr;
	if(!lt){
		sl = t->toStringLeaf();
	}

	StringMatcher* sm = nullptr;
	EpsilonMatcher* em = nullptr;
	TreeMatcher* tm  = nullptr;
	MatcherOperator* mo = nullptr;
	CharacterSetMatcher* cm = nullptr;

	sm = (m)->toStringMatcher();
	if(!sm){
	em = (m)->toEpsilonMatcher();
	if(!em){
	tm = (m)->toTreeMatcher();
	if(!tm){
	mo = (m)->toMatcherOperator();
	if(!mo){
	cm = (m)->toCharacterSetMatcher();
	}}}}

	assert(sm || em || tm || mo || cm);

	if(mo){
		//get or do compilation
		// run graph matching algo
		NFA* p = get_cached_nfa(mo);
		ListOfTrees* lt_temp = new ListOfTrees();
		lt_temp->subtrees.push_back(t);
		return matches(p, lt_temp);
	} else if(sm){
		if(!sl){ return {false,{}}; }
		//return {sl->text == sm->regex, {}};
		return {sl->text[0] == sm->regex[0], {}};
	} else if(cm){
		if(!sl){ return {false,{}}; }
		if(sl->text.length() != 1){ return {false, {}};}
		int char_val = sl->text[0];
		bool ret = (!cm->negation && cm->values[char_val] ) || (cm->negation && !cm->values[char_val]);
		return {ret, {}};
	} else if(tm){
		if(!lt){ return {false, {}}; }
		if(tm->type == &SUBTREE){
			assert(tm->matching_group != 0);
			match_results ret;
			ret.result = true;
			//std::cout << "SUBTREE: " << tm->matching_group << '\n';
			ret.captures[tm->matching_group] = {{t}};
			return ret;
		}

		assert(tm->body);
		NFA* cached_nfa = get_cached_nfa(tm->body);
		auto ret = matches(cached_nfa, lt);
		if(tm->type == &EXACT_OPEN){
			//std::cout << "EXACT: " << ret.captures.size() << '\n';
			return ret; }
		else if(ret.result){
			assert(tm->type == &CONTEXT_OPEN);
			assert(tm->matching_group != 0);
			ret.captures[tm->matching_group] = {{ new ContextHole() }};
			return ret;
		}
		assert(tm->matching_group != 0);

		Context* co = new Context();
		co->subtrees = lt->subtrees;
		assert(tm->type == &CONTEXT_OPEN);
		int tree_index = 0;
		for(Tree* t : lt->subtrees){
			if((t->toListOfTrees())){
				ret = matches(tm, t);
				if(ret.result) {
					assert(ret.captures[tm->matching_group].trees.size()==1);
					Context* co_inner = (ret.captures[tm->matching_group].trees[0])->toContext();
					assert(co_inner);
					co->subtrees[tree_index] = co_inner;
					ret.captures[tm->matching_group] = {{co}};
					return ret;
				}
			}
			tree_index++;
		}
		delete co;
		return {false, {}};
	} else if(em){
		// its not meaningful to match an empty string to a non-empty anything
		// technically it would always return false for a non-empty tree
		// but I prefer to be safe
		assert(false);
	} else {
		assert(false);
	}
}

#endif /* TREEREGEX_MATCHING_H */

// two problematic cases
// (1) a matcher matches more than one subtree
// (2) a subtree requires more than one matcher
// position in matcher
// position in tree
// OR
// DOING
// restrict matcher to be a single character
// restrict stringleaf to be a single character
// OR
// since a matcher has no metacharacters
// and a stringleaf is the entire string
// iff they match the regex must be a prefix of the stringleaf
// so for matching against a string leaf, we can just have an index in addition
// so we want a tree position
