// reducer_study.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include "../src/anetlist.h"
#include "../src/awriter.h"
#include "../src/anreducer.h"

using namespace std;
using namespace asr;


//circuit from ticer paper
anetlist netlist1() {
    anetlist nl;
 
    const auto gndNodeName = nl.groundName();

    nl.addElement(aelement(EL_G, "gd", 1.0, gndNodeName, "v1"));    
    nl.addElement(aelement(EL_G, "g13", 1.0, "v3", "v1"));
    nl.addElement(aelement(EL_G, "g23", 1.0, "v3", "v2"));
    
    nl.addElement(aelement(EL_C, "c1", 1.0, gndNodeName, "v1"));
    nl.addElement(aelement(EL_C, "c2", 1.0, gndNodeName, "v2"));
    nl.addElement(aelement(EL_C, "c3", 0.01, gndNodeName, "v3"));

    nl.setPort("v1", true);
    anetlistWriterTrivial wr(std::cout);
    wr.write(nl);
    return nl;
}

anetlist transmisonLine() {
    const double R = 250;
    const double C = 2e-12; //2pf
    const double Cc = C / 2;
    const int nnodes = 3;
    const int nlines = 2;
    const string out = "out";
    anetlist nl;
    const auto gndNodeName = nl.groundName();
    for (int line = 0; line < nlines; line++) {
        string n1 = "in";
        int node = 0;
        string n2 = "v" + to_string(line) + "_" + to_string(node);
        nl.addElement(aelement(EL_G, "Rdrv_"+to_string(line), 1.0/R,  n1, n2));
        nl.setPort(n1, true);

        for (int node = 0; node < nnodes; node++) {
            string n1 = "v" + to_string(line) + "_" + to_string(node);
            string n2 = "v" + to_string(line) + "_" + to_string(node+1);
            string nmr = "R" + to_string(line) + "_" + to_string(node);
            string nmc = "C" + to_string(line) + "_" + to_string(node);
            nl.addElement(aelement(EL_G, nmr, 1 /(R/nnodes), n1, n2));
            nl.addElement(aelement(EL_C, nmc, (C/nnodes), n1, gndNodeName));
            
            if (line) {
                string n1_other_line = "v" + to_string(line-1) + "_" + to_string(node);
                string nmc = "Cc" + to_string(line) + "_to_" + to_string(line-1) + "_" + to_string(node);
                nl.addElement(aelement(EL_C, nmc, (C / nnodes), n1, n1_other_line));
            }

            if (node == 0)  nl.setPort(n1, true);

            if (node == nnodes - 1) {
                nl.addElement(aelement(EL_G, out, 1.0/R, n2, out));
                nl.setPort(out, true);
            }
        }
    }
    return nl;
}

anetlist t2c() {
    anetlist nl;
    const auto gndNodeName = nl.groundName();

    //    C13 1 3 1.0e-12
    //    C23 2 3 1.0e-12
    //    C30 3 0 1.0e-12
    double v = 1.0e-12;
    nl.addElement(aelement(EL_C, "C13", v, "1", "3"));
    nl.addElement(aelement(EL_C, "C23", v, "2", "3"));
    nl.addElement(aelement(EL_C, "C30", v, "3", gndNodeName));

    nl.setPort("1", true);
    nl.setPort("2", true);

    return nl;
}


anetlist t2r() {
    anetlist nl;
    const auto gndNodeName = nl.groundName();

    //    C13 1 3 1.0e-12
    //    C23 2 3 1.0e-12
    //    C30 3 0 1.0e-12
    double v = 1.0;
    nl.addElement(aelement(EL_G, "R13", v, "1", "3"));
    nl.addElement(aelement(EL_G, "R23", v, "2", "3"));
    nl.addElement(aelement(EL_G, "R30", v, "3", gndNodeName));

    nl.setPort("1", true);
    nl.setPort("2", true);

    return nl;
}


void test1(anetlist &nl) {
    anreducer<anetlist, anode, aedge> nodeReducer;
    nodeReducer.Fmax() = 2e1;

    for (auto nit : nl) {
        auto &node = nit.second;
        auto nodeConstant = nodeReducer.NodeTimingConstant(&node, &nl);
        std::cout << " node id " << nit.first << " name " << nl.nodeName(nit.first) << " tau " << nodeConstant.first << " GN " << nodeConstant.second << std::endl;
        std::cout << "\t\t is Fast: " << nodeReducer.IsFastNode(&node, &nl, nodeConstant.first) << std::endl; 
    }

    for (auto nit : nl) {
        auto &node = nit.second;
        auto nodeConstant = nodeReducer.NodeTimingConstant(&node, &nl);
        if (nodeReducer.IsFastNode(&node, &nl, nodeConstant.first)) {
            nodeReducer.reduceQuickNodeRC(&node, &nl, nodeConstant.second);
            break;
        }
    }
}


void test2(anetlist& nl) {
    anreducer<anetlist, anode, aedge> nodeReducer;
    nodeReducer.Fmax() = 2e6;
    netlistStatistics stats(&nl);
    cout << "RCReduce test start" << endl;
    stats.info(cout);
    std::cout << "****************************************************\n";
    std::cout << "do reduce" << std::endl;
    std::cout << "****************************************************\n";    
    
    nodeReducer.reduce(nl);   

    std::cout << "****************************************************\n";
    std::cout << "after reduce" << std::endl;
    std::cout << "****************************************************\n";
    stats.info(cout);
    cout << "RCReduce test completed" << endl;
}

int main()
{
    std::cout << "Start test\n";
    if (0) {
        std::cout << "Start test1\n";
        anetlist nl = netlist1();
        //test1(nl); 
        test2(nl);
        anetlistWriterTrivial wr(std::cout);
        wr.write(nl);
    }
    

    if (0) {
        std::cout << "Start test2, t2c  \n";
        anetlist nl = t2c();
        anetlistWriterTrivial wr0(std::cout);
        wr0.write(nl);

        test2(nl);
        anetlistWriterTrivial wr(std::cout);
        wr.write(nl);
    }
    if (0) {
        std::cout << "Start test2, t2r  \n";
        anetlist nl = t2r();
        anetlistWriterTrivial wr0(std::cout);
        wr0.write(nl);

        test2(nl);
        anetlistWriterTrivial wr(std::cout);
        wr.write(nl);
    }

    {
        std::cout << "Start test2, transmission line \n";
        anetlist nl = transmisonLine();
        ofstream f("tl.cir");
        anetlistWriterTrivial wr0(f);
        wr0.write(nl);

        test2(nl);
        ofstream fr("tl.r.cir");

        anetlistWriterTrivial wr(fr);
        wr.write(nl);
    }
    return 0;
}
