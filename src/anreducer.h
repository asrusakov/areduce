#pragma once
#include <utility>
#define _USE_MATH_DEFINES
#include <math.h>
#include <set>
#include <assert.h>

namespace asr {

	//setting for the reducer
	class anreducerControls {
	public:

		anreducerControls() {}

		double  Fmax() const { return Fmax_; }
		double& Fmax() { return Fmax_; }

		double  Eps() const { return Eps_; }
		double& Eps() { return Eps_; }

		bool  allowNegativeCaps() const { return allowNegativeCaps_; }
		bool& allowNegativeCaps() { return allowNegativeCaps_; }

	protected:
		double Fmax_ = 1e9;
		double Eps_ = 1;
		int    maxNodeDegree_ = 6;
		//Netlist *nl_;
		bool   allowNegativeCaps_ = false;
	};

	//Ticer node reducer
	template <class Netlist, class Node, class Edge>  class anreducer : public anreducerControls {
	public:

		anreducer() {};

		anreducer(const anreducerControls &cntls) : anreducerControls(cntls) {};

		~anreducer() {};

		void reduce(Netlist& nl);

		//reduce node with RC connection
		//returns nodes affected
		std::set<NodeId> reduceQuickNodeRC(Node* n, Netlist* nl, const double nodeGn);

		///
		/// tau = Cn/Gn
		/// Cn = Sum(Ci),  Gn = Sum(Gi)
		/// returns tau, Gn
		std::pair< double, double> NodeTimingConstant(const Node* n, Netlist* nl) const;

		///
		/// tau = Cn/Gn
		/// Cn = Sum(Ci),  Gn = Sum(Gi)
		/// returns tau, Gn
		bool IsFastNode(const Node* n, Netlist* nl, double tau) const {
			return 2 * M_PI * Fmax_ * tau <= Eps_;
		}
	};

}

#include "anreducer_imp.h"
