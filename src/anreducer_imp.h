#pragma once
#pragma once
#include <utility>
#define _USE_MATH_DEFINES
#include <math.h>
#include <queue>
#include <set>
#include <assert.h>
#include "apriority_bucket.h"

//#define HARD_DEBUG

namespace asr {

	template <class Netlist, class Node, class Edge>
	void  anreducer<Netlist, Node, Edge>::reduce(Netlist& nl) {

#ifdef HARD_DEBUG
		for (auto& nit : nl) {
			auto node = nit.second;
			auto nodeConstant = NodeTimingConstant(&node, &nl);
			std::cout << " node id " << nit.first << " name " << nl.nodeName(nit.first) << " tau " << nodeConstant.first << " GN " << nodeConstant.second << std::endl;
			std::cout << "\t\t is Fast: " << IsFastNode(&node, &nl, nodeConstant.first) << std::endl;
		}
#endif
		//update node degree
		for (auto& nit : nl) {
			Node& node = nit.second;
			if (node.fixed() || node.port()) continue;
			nl.updateNodeDegree(node);
		}

		//handle 2 degree nodes only.
		for (auto& nit : nl) {
			auto& node = nit.second;
			if (node.fixed() || node.port() || node.invalid()) continue;
			if (node.degree() <= 2) {
				auto nodeConstant = NodeTimingConstant(&node, &nl);
				if (IsFastNode(&node, &nl, nodeConstant.first)) {
					const auto affectedNodes = reduceQuickNodeRC(&node, &nl, nodeConstant.second);
					//for some definitions of node degree we may not need such a change
					//keep it here to allow doing experiments in future
					for (auto& aff_node_id : affectedNodes) {
						auto aff_node = nl.node(aff_node_id);
						nl.updateNodeDegree(aff_node);
					}
				}
			}
		}

		//!!!TODO speedup!!!
		//queue of potentail nodes for removal
		//auto cmp = [&nl](NodeId left, NodeId right) { return nl.node(left).degree() < nl.node(left).degree(); };
		asr::priority_bucket<Netlist, Node> q(maxNodeDegree_, nl);

		for (auto& nit : nl) {
			auto& node = nit.second;
			if (node.fixed() || node.port() || node.invalid()) continue;
			nl.updateNodeDegree(node);
			if (node.degree() <= maxNodeDegree_) {
				auto nodeConstant = NodeTimingConstant(&node, &nl);
				if (IsFastNode(&node, &nl, nodeConstant.first)) {
					q.insert(node);
				}
			}
		}

		//
		//reduce other nodes
		//
		while (!q.empty()) {
			NodeId currId = q.top();
			q.pop();
			auto& node = nl.node(currId);
			auto nodeConstant = NodeTimingConstant(&node, &nl);
			if (IsFastNode(&node, &nl, nodeConstant.first)) {
				const auto old_degree = node.degree();
				auto affectedNodes = reduceQuickNodeRC(&node, &nl, nodeConstant.second);
				for (auto& aff_node_id : affectedNodes) {
					auto &aff_node = nl.node(aff_node_id);
					if (aff_node.fixed() || aff_node.port() || aff_node.invalid()) continue;
					auto old_deg = nl.updateNodeDegree(aff_node);
					q.update(aff_node, old_deg);
				}
			}
		}

		nl.removeInvalidNodes();
	}

	//reduce node with RC connection
	//returns nodes affected 
	template <class Netlist, class Node, class Edge>
	std::set<NodeId> anreducer<Netlist, Node, Edge>::reduceQuickNodeRC(Node* n, Netlist* nl, const double nodeGn) {
		assert(n->port() == false && n->fixed() == false);
		std::set<NodeId> affected_nodes;
		const double nodeGnInv = 1.0 / nodeGn;
		double nodeCap = 0;
		if (allowNegativeCaps()) {
			for (size_t i = 0; i < n->edges().size(); i++) {
				auto& edgeid = n->edges()[i];
				auto& edge = nl->edge(edgeid);
				nodeCap += edge.getCap();
			}
		}

#ifdef HARD_DEBUG
		std::cout << " reduce node: " << n->id() << " name " << nl->nodeName(n->id()) << " ";
		nl->info(*n, std::cout);
#endif

		//cycle over edges coming to node
		const double eps = 1e-16;
		auto nodeid = n->id();
		const auto& edges = n->edges();
		std::vector<Edge> edges_to_add;
		edges_to_add.reserve(std::min(100, int(edges.size() * edges.size())));
		for (size_t i = 0; i < n->edges().size(); i++) {
			auto& edgeid = n->edges()[i];
			auto& edge = nl->edge(edgeid);
			if (edge.type() == EL_NONE) continue;

			//g edge multiplied on c  and c. 
			//c edge multipled only on g
			auto othernodeid = edge.node(0);
			if (othernodeid == nodeid)  othernodeid = edge.node(1);
			if (othernodeid == nodeid)  continue;
			auto nodeidI = othernodeid;
			const auto gin = edge.value();
			if (abs(gin) > eps) {
				for (size_t j = i+1; j < n->edges().size(); j++) {
					auto& edgeidJ = n->edges()[j];
					//if (edgeidJ == edge.id()) continue;
					auto& edgeJ = nl->edge(edgeidJ);

					if (edgeJ.type() == EL_NONE) continue;
					if (edge.type() == EL_C && edgeJ.type() != EL_G) continue;

					auto nodeidJ = edgeJ.node(0);
					if (nodeidJ == nodeid)  nodeidJ = edgeJ.node(1);
					if (nodeidJ == nodeid)  continue;

					if (nodeidJ == nodeidI) continue;

					const auto gc_jn = edgeJ.value();
					if (abs(gc_jn) < eps) continue;

					//!!!TODO - avoid double insertion for speeding up!!!
					double val = gin * gc_jn * nodeGnInv ;

					assert(nodeidI != nodeidJ);
					auto newEdgeType = edgeJ.type();
					if (edge.type() == EL_C) newEdgeType = EL_C;

					aedge e(newEdgeType, nodeidI, nodeidJ, val);
					//nl->addEdge(e);
					edges_to_add.push_back(e);
					affected_nodes.insert(nodeidJ);

					if (allowNegativeCaps() && edge.type() == EL_G) {
						if (edgeJ.type() == EL_G) {
							double v = -gin * gc_jn * nodeGnInv * nodeGnInv * nodeCap;
							aedge e(EL_C, nodeidI, nodeidJ, v);
							edges_to_add.push_back(e);
						}
					}
				}
			}

		}


		auto nodeedges = n->edges();
		for (size_t i = 0; i < nodeedges.size(); i++) {
			auto& edgeid = nodeedges[i];
			nl->removeEdge(edgeid);
		}
		nl->removeNode(nodeid);

		for (const auto& e : edges_to_add) {
			nl->addEdge(e);
#ifdef HARD_DEBUG		
			std::cout << " add edge between node: " << nl->nodeName(e.node(0))
				<< " and node : " << nl->nodeName(e.node(1))
				<< " of type " << e.type() << " and value " << e.value() << std::endl;
#endif
		}
#ifdef HARD_DEBUG		
		anetlistWriterTrivial wr(std::cout);
		wr.write(*nl);
#endif

		return affected_nodes;
	}

	///
	/// tau = Cn/Gn
	/// Cn = Sum(Ci),  Gn = Sum(Gi)
	/// returns tau, Gn
	template <class Netlist, class Node, class Edge>
	std::pair< double, double> anreducer<Netlist, Node, Edge>::NodeTimingConstant(const Node* n, Netlist* nl) const {
		double Cn = 0;
		double Gn = 0;
		double Rn = 0;
		for (auto&& eid : n->edges()) {
			const auto& e = nl->edge(eid);
			Gn += e.getConductance();
			Rn += e.getResistance();
			Cn += e.getCap();
		}
		//double tau = Cn * Rn;
		double tau = Cn / Gn;

		return std::make_pair(tau, Gn);
	}
};



