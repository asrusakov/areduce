#pragma once
#include "ahash.h"
#include <string>
#include <iosfwd>
#include <vector>

///
/// netlist is a hash of nodes and edges.
///

namespace asr {


	typedef long int NodeId;
	typedef long int EdgeId;
	typedef double   Value; 
	typedef std::string NodeName;
	typedef std::string ElementName;

	enum   ELTYPE : short { EL_G=0, EL_C, EL_L, EL_K, EL_NONE };

	//elements in the netlist , useful for netlist creation
	class aelement {
	public:
		aelement(ELTYPE t, const ElementName& n, Value val, const NodeName& n1, const NodeName& n2);
		const NodeName& nodeName(int idx) const { return nodes_[idx]; }
		ELTYPE type() const { return type_; }
		Value  value() const { return val_; }
		ElementName name() const { return name_; };
	private:
		ELTYPE type_;
		Value val_;
		NodeName nodes_[2];
		//name of element
		ElementName name_;
	};

	//Nodes in the netlist
	class anode {
	public:
		anode() {};
		anode(NodeId id) : id_(id) {}

		//same as port now
		bool  fixed() const { return port(); }
		//port must be preserved
		bool  port() const { return is_port_; }
		bool& port() { return is_port_; };
		
		const std::vector<EdgeId>& edges() const { return edges_; }
		
		NodeId id() const { return id_;  }

		void addEdge(EdgeId eid) { edges_.push_back(eid); }
		void removeEdge(EdgeId eid);

		//node degree - kept by reducer
		int  degree() const {return degree_;}
		int& degree()       {return degree_;}
			
		//removed nodes are marked as invalid
		bool  invalid() const { return is_invalid_; }
		bool& invalid()       { return is_invalid_; }

	private:
		NodeId id_ = -1;
		int degree_ = 0;
		bool is_port_	 = false;
		bool is_invalid_ = false;
		std::vector<EdgeId> edges_;
		static NodeId    groundId_;
	};

	
	//Edges (r,l,c,k) in the netlist - internal structure
	class aedge {		
	public:
		aedge(){}
		aedge(ELTYPE t, NodeId n1, NodeId n2, Value val, const ElementName& n = "") : name_(n), nodes_{n1,n2}, val_(val), type_(t) {}
		NodeId node(int idx) const { return nodes_[idx]; }

		EdgeId id() const { return id_; }
		
		const ElementName& name() const { return name_; }
		ELTYPE  type()  const { return type_; }
		ELTYPE& type()        { return type_; }
		Value   value() const { return val_; }
		Value&  value()       { return val_; }


		//zero if no conductance is defined
		Value getConductance() const { return  (type_ == EL_G ? value() : 0); }
		Value getResistance() const { return  (type_ == EL_G ? 1.0/value() : 0); }
		Value getCap()         const { return  (type_ == EL_C ? value() : 0); }
	private:
		friend class anetlist; 

		ElementName name_;
		NodeId nodes_[2] = { -1, -1 };
		Value  val_  = 0.0;
		ELTYPE type_ = EL_NONE;
		EdgeId id_ = -1;
	};

	class anetlist {
	public:

		anetlist();

		//reader part -- to do . constness
		typedef ahash<NodeId, anode>::iterator	nodeIter;
		typedef ahash<NodeId, aedge>::iterator	edgeIter;
		typedef ahash<NodeId, anode>::const_iterator	const_nodeIter;
		typedef ahash<NodeId, aedge>::const_iterator	const_edgeIter;
		
		//iterate over nodes
		const_nodeIter begin() const { return nodes_.cbegin(); }
		const_nodeIter end()   const { return nodes_.cend(); }
		
		nodeIter begin()  { return nodes_.begin(); }
		nodeIter end()    { return nodes_.end(); }
		
		//iterate over edges
		const_edgeIter edges_cbegin() const { return edges_.cbegin(); }
		const_edgeIter edges_cend()   const { return edges_.cend(); }				

		//ground id
		const NodeId    groundId() const { return groundId_; };
		const NodeName& groundName() const { return groundName_; };

		//returns invalid nodeId if node does not exist
		NodeId findNode(const NodeName& nm) const; 

		//nodes by id. error if node does not exists
		anode& node(NodeId id);

		//edge by id
		aedge& edge(EdgeId id);


		////////////////////////////////////////////
		//editor part
		////////////////////////////////////////////

		//sets node to unremovable port
		void setPort(const NodeName &nm, bool val);
		//adds elements, creates nodes if needed
		void addElement(const aelement& el);		
		
		//node/edge removal		
		void   removeEdge(EdgeId id);
		//mark node as invalid
		void   removeNode(NodeId id);
		//remove all nodes from hash marked as invalid. 
		void   removeInvalidNodes();

		//debug dumps
		std::ostream& info(const aedge& e, std::ostream& s) const;
		std::ostream& info(const anode& n, std::ostream& s) const;

		NodeName nodeName(NodeId id) const;

		//return NodeId for a given node name. Does nothing if node already exists. Creates a new node if node does not exist
		NodeId   addNode(const NodeName& nm);
		EdgeId   addEdge(const aedge& e);

		int updateNodeDegree(anode &n) ;

	private:
		//data
		ahash<NodeId, anode> nodes_;
		ahash<EdgeId, aedge> edges_;
		ahash<NodeName, NodeId> names2nodes_;	
		ahash<NodeId, NodeName> nodes2names_;

		NodeId  max_id_ = 0;
		
		      NodeId    groundId_   = 0; 
		//const NodeName  groundName_ = "GND";
		const NodeName  groundName_ = "0";
		const NodeId NodeInvalidId = -1;
	};

	//just a wrap for traversing over edges
	class netlistEdgeIteratorWrap {
	public:
		netlistEdgeIteratorWrap(const anetlist& nl) : nl_(nl) {}

		anetlist::const_edgeIter cbegin() const { return nl_.edges_cbegin(); }
		anetlist::const_edgeIter cend() const { return nl_.edges_cend(); }
		anetlist::const_edgeIter begin() const { return nl_.edges_cbegin(); }
		anetlist::const_edgeIter end() const { return nl_.edges_cend(); }
		//edgeIter begin() { return nl.edges_cbegin(); }
		//edgeIter end() { return nl.edges_cend(); }
	
	private:
		const anetlist & nl_;
	};

	//collect netlist statistics 	
	class netlistStatistics {
	public:
		netlistStatistics(anetlist* nl) : nl_(nl) {};
		std::ostream& info(std::ostream& s);
	protected:
		void collectStats();
	private:
		const anetlist* nl_;
		size_t node_count_ = 0;
		size_t edge_count_ = 0;
		size_t port_count_ = 0;
		size_t count_[EL_NONE];
	};


};