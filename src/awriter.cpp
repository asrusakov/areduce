#include "awriter.h"
#include "anetlist.h"
#include <ostream>

using namespace std; 
namespace asr {

	//only for debug 
	void anetlistWriterTrivial::write(const anetlist& nl) {
		s_ << "Trivial test\n";
		s_ << "* Nodes: \n";
		std::vector<string> ports;
		string ports_string;		
		for (const auto& nit : nl) {
			auto& node = nit.second;
			if (node.invalid()) continue;
			if (node.port()) {
				string pname = nl.nodeName(node.id());
				ports.push_back(pname);
				ports_string += " ";
				ports_string += pname;
			}
		}

		s_ << "*.SUBCKT interconnect " << ports_string << endl;
		
		for (const auto& e : netlistEdgeIteratorWrap(nl)) {
			EdgeId k = e.first;
			const aedge& edge = e.second;			
			if (edge.type() != EL_NONE) nl.info(edge, s_);
		}
		s_ << "V in 0 AC 1\n";
		s_ << endl;
		s_ << "*.tran 0 100 1\n";
		s_ << ".ac   DEC 10 1000 10E9\n";
		s_ << ".plot ac v(out) " << std::endl;
		s_ << ".END\n";

	}
}