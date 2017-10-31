#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
template<typename TN, typename TE> struct Edge;

template<typename TN, typename TE>
struct Node {
	TN val;
	std::vector<Edge<TN,TE>*> incoming_edges;
	std::vector<Edge<TN,TE>*> outgoing_edges;
	Node(TN v = TN()):val(v){}
};

template<typename TN, typename TE>
struct Edge {
	TE val;
	Node<TN,TE>& begin;
	Node<TN, TE>& end;
	Edge(Node<TN, TE>& b, Node<TN, TE>& e, TE v = TE()):begin(b), end(e), val(v){}

	void finalize(){
		begin.outgoing_edges.push_back(this);
		end.incoming_edges.push_back(this);
	}
};

template<typename TN, typename TE>
struct Graph {
	std::deque<Node<TN, TE>> nodes;
	std::deque<Edge<TN, TE>> edges;
	Graph():nodes(), edges(){}

	Edge<TN,TE>& addEdge(Node<TN, TE>& n1, Node<TN, TE>& n2, TE v = TE()){
		edges.push_back(Edge<TN,TE>(n1, n2, v));
		edges.back().finalize();
		return edges.back();
	}
	Node<TN,TE>& addNode(TN v = TN()){
		nodes.push_back(Node<TN,TE>(v));
		return nodes.back();
	}
};

/* Graph builder
	->
	takes matcher, returns graph
	-> run optimization pass
	-> returns that graph
The optimization pass is out of scope for this paper - it needs its own paper
b/c it must include a proof of correctness, which is non-trivial

	*/

#endif /* GRAPH_H */
