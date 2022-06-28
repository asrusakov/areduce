#pragma once
#include "ahash.h"
#include <forward_list>
#include <list>
#include <vector>

namespace asr {

	//dynamic bucket based priority queue
	template <class Netlist, class Node> class priority_bucket {
	public:
		priority_bucket(int maxBucket, Netlist &nl);
		void insert(const Node& n);
		void update(const Node& n, int old_node_degree);
		bool empty() const;
		size_t   size() const;
		void	 pop();
		NodeId   top();

	private:		
		std::vector< std::list<int> > buckets_;
		const Netlist& nl_;
		const int max_bucket_;
	    mutable int curr_min_bucket_ = 0;
		ahash < NodeId, std::list<int>::iterator > node_position_;
 	};

	template <class Netlist, class Node> 
	priority_bucket<Netlist, Node>::priority_bucket(int maxBucket, Netlist& nl) : max_bucket_(maxBucket), nl_(nl) {
		buckets_.resize(max_bucket_);
		curr_min_bucket_ = buckets_.size()-1;
	}

	template <class Netlist, class Node>
	void priority_bucket<Netlist, Node>::insert(const Node& n) {		
		const auto deg = n.degree();
		assert(deg >= 0);
		if (deg < max_bucket_) {
			buckets_[deg].push_front(n.id());
			node_position_[n.id()] = buckets_[deg].begin();
			if (deg < curr_min_bucket_) curr_min_bucket_ = deg;
		}
	}

	template <class Netlist, class Node>
	NodeId priority_bucket<Netlist, Node>::top() {
		if (!buckets_[curr_min_bucket_].empty()) {
			return *(buckets_[curr_min_bucket_].begin());
		}
		else {
			while (buckets_[curr_min_bucket_].empty() && curr_min_bucket_ < int (buckets_.size()) ) {
				curr_min_bucket_++;
			}
			if (!buckets_[curr_min_bucket_].empty()) {
				return *(buckets_[curr_min_bucket_].begin());
			}
			else {
				assert(0);
				return -1;
			}
		}		
	}

	template <class Netlist, class Node>
	void priority_bucket<Netlist, Node>::pop() {
		if (!buckets_[curr_min_bucket_].empty()) {
			buckets_[curr_min_bucket_].pop_front();
		}
		else {
			while ( buckets_[curr_min_bucket_].empty() && curr_min_bucket_ < int(buckets_.size()) ) {
				curr_min_bucket_++;
			}
			if (!buckets_[curr_min_bucket_].empty()) {
				buckets_[curr_min_bucket_].pop_front();
			}
			else {
				assert(0);				
			}
		}
	}

	template <class Netlist, class Node>
	bool  priority_bucket<Netlist, Node>::empty() const {
		if (!buckets_[curr_min_bucket_].empty()) return false;
		else {
			while (curr_min_bucket_ < int(buckets_.size() && buckets_[curr_min_bucket_].empty())) {
				curr_min_bucket_++;
			}
			return buckets_[curr_min_bucket_].empty();
		}
	}

	template <class Netlist, class Node>
	void priority_bucket<Netlist, Node>::update(const Node &n, int old_node_degree) {
		auto deg = n.degree();
		auto id = n.id();   
		if (deg == old_node_degree) return;

		auto it = node_position_.find(n.id());
		if (it != node_position_.end()) {		
			buckets_[old_node_degree].erase(it->second);
			buckets_[deg].push_front(id);
			it->second = buckets_[deg].begin();
		}
		else {
			buckets_[deg].push_front(id);
			node_position_[n.id()] = buckets_[deg].begin();
		}
		//assert( node_position_[n.id()].second == )
	}
}