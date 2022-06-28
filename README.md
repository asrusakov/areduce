# areduce
A version of the realizable RC(LK?) reduction for interconnect.

It is supposed to be used for parasitic netlist reduction. 
The core library is developed to be used in spef2spef, dspf2dspf, spice2spice reduction.
Parser and dumper code are not added to github yet.


The algorithm of the fast node reductoin follows ideas of [2]. 
Nodes are choosed to minimize number of fill-ins. 
So a bucket version of the priority queue is developed. This queue is used in the kind of minimum degree node selection algorithm.
Ports are not reduce.

Code structure:

  anetlist   - passive element netlist implementation. 
      anode    - netlist node
      aedge    - netlist element. edge connecting 2 nodes. It is either G,C or (to be added L,K).
  
  anreducer  - reduction itself. It modifies anetlist.


Example of usage:

   test/reducer_study.cpp


TODO:
  spef parser
  floating node reduction
  L,K reduction
  parametric reduction



Reference:

  [1] B. Sheehan,  "TICER: Realizable Reduction of Extracted RC Circuits," in Computer-Aided Design, International Conference on, San   Jose, CA, 1999 pp. 200.doi: 10.1109/ICCAD.1999.810649 

  [2] Sheehan, B.N.: Realizable Reduction of RC Networks, in IEEE Transactions on Computer-Aided Design of
  Integrated Circuits and Systems, vol. 26, no. 8, pp. 1393-1407, (2007).

  [3] Gourary, M.M., Rusakov, S.G., Ulyanov, S.L., Zharov, M.M. (2011). Network Reduction by Inductance Elimination. In: Benner, P., Hinze, M., ter Maten, E. (eds) Model Reduction for Circuit Simulation. Lecture Notes in Electrical Engineering, vol 74. Springer,   Dordrecht. https://doi.org/10.1007/978-94-007-0089-5_8
