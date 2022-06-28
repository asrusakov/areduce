#include "anetlist.h"
#include "assert.h"
#include <ostream>
#include <algorithm>
using namespace std;

namespace asr {

	const char* eltype_char[] = { "G", "C", "L", "K", "N" };

	anetlist::anetlist() {
		groundId_ = addNode(groundName());		
		auto gnode = node(groundId_);
		setPort(groundName(), true);
	}

	int anetlist::updateNodeDegree(anode& n) {
		int old_degree = n.degree();
		const auto& es = n.edges();
		int degree = 0;
		for (auto eid : es) {
			const auto& e = edge(eid);
			if (e.type() == EL_NONE) continue;
			if (e.node(0) == groundId_ || e.node(1) == groundId_) {
				continue;
			}
			degree++;
		}
		n.degree() = degree;
		return old_degree; 
	}

	//sets node to unremovable port
	void anetlist::setPort(const NodeName& nm, bool val) {
		auto it = names2nodes_.find(nm);
		assert(it != names2nodes_.end());
		node(it->second).port() = val;
	}

	//access to node
	anode& anetlist::node(NodeId id) {
		auto it = nodes_.find(id);
		assert(it != nodes_.end());
		return it->second;
	}

	aedge& anetlist::edge(EdgeId id) { 
		auto it = edges_.find(id);
		assert(it != edges_.end());
		return it->second;
	}

	//access to node
	NodeId anetlist::findNode(const NodeName& nm) const {
		auto it = names2nodes_.find(nm);
		if (it != names2nodes_.end()) return it->second;
		else return NodeInvalidId;
	}

	void anetlist::addElement(const aelement& el) {
		auto n1 = addNode(el.nodeName(0));
		auto n2 = addNode(el.nodeName(1));
		assert(n1 != n2);
		if (n1 > n2) {
			auto t = n2;
			n2 = n1;
			n1 = t;
		}
		aedge e(el.type(), n1, n2, el.value(), el.name());
		addEdge(e);
	}

	//returns id of existing or a new node
	NodeId anetlist::addNode(const NodeName& nm) {
		auto id = findNode(nm);
		if (id != NodeInvalidId) return id;
		id = max_id_++;
		nodes_[id] = anode(id);
		nodes2names_[id] = nm;
		names2nodes_[nm] = id;
		return id;
	}

	EdgeId anetlist::addEdge(const aedge& e) {
		auto& node1 = node(e.node(0));
		
		//merge parallel
		auto toId = e.node(1);
		for (auto& oeId : node1.edges()) {
			aedge& oe = edge(oeId);

			if (e.type() != oe.type()) continue;
			if ((oe.node(0) == e.node(0) && oe.node(1) == e.node(1)) ||
				(oe.node(0) == e.node(1) && oe.node(1) == e.node(0))) {
				//merge
				oe.value() = oe.value() + e.value();
				return oeId;
			}
		}

		auto& node2 = node(e.node(1));

		EdgeId id = max_id_++;
		edges_[id] = e;
		//TODO speedup
		edges_[id].id_ = id;

		node1.addEdge(id);
		node2.addEdge(id);

		return id;
	}

	NodeName anetlist::nodeName(NodeId id) const {
		auto it = nodes2names_.find(id);
		assert(it != nodes2names_.end());
		return it->second;
	}

	void   anetlist::removeEdge(EdgeId id) {
		auto &e = edge(id);
		e.type() = EL_NONE;

		auto& node1 = node(e.node(0));
		node1.removeEdge(id);
		auto& node2 = node(e.node(1));
		node2.removeEdge(id);		
	}

	void anode::removeEdge(EdgeId id) {
		auto it = std::find(edges_.begin(), edges_.end(), id);
		assert(it != edges_.end());
		*it = std::move(edges_.back());
		edges_.pop_back();
	}

	void anetlist::removeNode(NodeId id) {
		auto &n = node(id);
		assert(n.edges().size() == 0);
		n.invalid() = true;
	}

	void anetlist::removeInvalidNodes() {		
		for (auto it = nodes_.begin(); it != nodes_.end(); ) {
			if (it->second.invalid()) {
				it = nodes_.erase(it);
			}
			else {
				++it;
			}
		}
	}

	std::ostream& anetlist::info(const anode& n, std::ostream& s) const {
		s << "node: " << n.id() << " with edges " << n.edges().size() << " is port " << n.port() << endl;
		return s;
	}

	std::ostream& anetlist::info(const aedge& e, std::ostream& s) const {		
		string nm = e.name().empty() ? to_string(e.id()) : e.name();
		if (e.type() == EL_G) { //spice wants R
			s << "r";
			s << nm << " " << nodeName(e.node(0)) << " " << nodeName(e.node(1)) << " " << 1.0/e.value() << "\n";
		}
		else {
			s << eltype_char[e.type()];
			s << nm << " " << nodeName(e.node(0)) << " " << nodeName(e.node(1)) << " " << e.value() << "\n";
		}
		return s;
	}
	/*
	std::ostream& anetlist::info(const anode& n, std::ostream& s) {
		return s;
	}*/

	aelement::aelement(ELTYPE t, const ElementName& n, Value val, const NodeName& n1, const NodeName& n2) :
		type_(t), val_(val), nodes_{ n1, n2 }, name_(n) {

	}

	//statistics collections
	std::ostream& netlistStatistics::info(std::ostream& s) {
		collectStats();
		s << "Netlist: nodes " << node_count_ << " elements " << edge_count_ << " resistors " << count_[EL_G] << " capacitors " << count_[EL_C] << " ports " << port_count_ << endl;
		return s;
	}
	
	void netlistStatistics::collectStats() {
		node_count_ = 0;
		port_count_ = 0;
		edge_count_ = 0;
		count_[EL_G] = 0;
		count_[EL_C] = 0;

		for (auto& nit : *nl_) {
			auto& node = nit.second;
			if (node.invalid()) continue;
			node_count_++;
			if (node.port()) port_count_++;				
		}

		for (const auto& e : netlistEdgeIteratorWrap(*nl_)) {			
			const aedge& edge = e.second;
			if (edge.type() == EL_NONE) continue;
			edge_count_++;
			count_[edge.type()]++;
		}
	}

}