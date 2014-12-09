FinalProject
============
Classes 
=======
Cluster.h - Cluster related functions <br/>
ClusterManager.h - Cluster Manager related functions like forming the cluster, cluster selection etc.<br/>
FinalTry.h - Header file for FinalTry.cc and has some utility functions<br/>
FinalTry.cc - Main Methods with Simulation and round code.<br/>
MyTag.h - Packet header class<br/>
mobsmple.tcl - TCL file describing the mobility of nodes<br/><br/><br/>

Command to Run : <br/>
./waf --run "FinalTry --traceFile=scratch/mobsmple.tcl --topics=1 --duration=3000.0 --selfish=0 --distanceCheck=1 --verbose=false --masters=5 --range=30.0 --threshold=30.0 --rounds=9"

