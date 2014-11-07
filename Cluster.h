#include "ns3/core-module.h"
#include <iostream>
#include <map>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdlib>


using namespace ns3;
using namespace std;
 
class Cluster{

	private: 
		int clusterID;
		int numNodes;
		Ptr<Node> masterNode;
		vector<Ptr<Node> > nodeList;
	
	public:
	    
	    Cluster(int ID)
	    {
			clusterID = ID;
			numNodes = 0;
		}
		Ptr<Node> getMaster();
		bool isMaster(Ptr<Node> node);
		vector < Ptr<Node> > getNodeList();
		void addNodeToCluster(Ptr<Node> node);
		int getNumNodes();	
};

int Cluster::getNumNodes()
{
		return numNodes;
}
void Cluster::addNodeToCluster(Ptr<Node> node){
	
		nodeList.push_back(node);
		numNodes++;
		
		//by default set the first node as master as of now
		// instead in future use an algorithm for choosing the master which will be done by the clusterManager and not by the Cluster class
		
		if(numNodes == 1)
			masterNode = node;
		
}
Ptr<Node> Cluster::getMaster(){
	
		return masterNode;
}
bool Cluster::isMaster(Ptr<Node> node)
{
		return (masterNode == node);
}

vector < Ptr<Node> > Cluster::getNodeList(){
	
	return nodeList;
}
