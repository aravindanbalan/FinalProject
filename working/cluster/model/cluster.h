/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef CLUSTER_H
#define CLUSTER_H

#include "ns3/application.h"
#include "ns3/core-module.h"
#include <iostream>
#include <map>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <ns3/node.h>

namespace ns3 {

/* ... */

class Cluster  : public Application{

	private: 
		int clusterID;
		int topic;
		int numNodes;
		Ptr<Node> masterNode;
		std::vector<Ptr<Node> > nodeList;
		std::vector<Ptr<Node> > slaveNodes;
	
	public:
	    
	    Cluster(int ID, int top)
	    {
			clusterID = ID;
			numNodes = 0;
			topic = top;
		}
		Ptr<Node> getMaster();
		std::vector< Ptr<Node> > getSlaveNodes();
		bool isMaster(Ptr<Node> node);
		std::vector < Ptr<Node> > getNodeList();
		void addNodeToCluster(Ptr<Node> node);
		void removeNodeFromCluster(Ptr<Node> node);
		int getNumNodes();
		int getTopic();	
		void setMaster(Ptr<Node> master);
};


}

#endif /* CLUSTER_H */

