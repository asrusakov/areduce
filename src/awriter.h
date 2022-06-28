#pragma once
#include <iosfwd>
namespace asr {

	class anetlist; 

	//basic abstract class for netlist writer
	class awriter
	{
	public:
		awriter(std::ostream& s) : s_(s) {}

		//write to the stream 
		virtual void write(const anetlist &nl) = 0;

	protected:
		std::ostream &s_;
	};


	//debug purpose writer
	class anetlistWriterTrivial : public awriter 
	{
	public:
		
		anetlistWriterTrivial(std::ostream& s) : awriter(s) {}

		void write(const anetlist& nl);
	};
}

