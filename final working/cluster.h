
#include "ns3/core-module.h"
#include <iostream>
#include <map>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <ns3/node.h>
using namespace ns3;
using namespace std;

class Cluster {

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

	
int Cluster::getNumNodes()
{
		return numNodes;
}
int Cluster::getTopic()
{
		return topic;
}
void Cluster::addNodeToCluster(Ptr<Node> node){
	
		nodeList.push_back(node);
		numNodes++;	
}

void Cluster::removeNodeFromCluster(Ptr<Node> node){
	
	vector <Ptr<Node> > nodeList = getNodeList();
	for( vector<Ptr<Node> >::iterator iter = nodeList.begin(); iter != nodeList.end(); ++iter )
	{
		if( *iter == node )
		{
			nodeList.erase( iter );
			numNodes--;
			break;
		}
	}
	
}

Ptr<Node> Cluster::getMaster(){
	
		return masterNode;
}
bool Cluster::isMaster(Ptr<Node> node)
{
		return (getMaster() == node);
}

vector < Ptr<Node> > Cluster::getNodeList(){
	
	return nodeList;
}

vector< Ptr<Node> > Cluster::getSlaveNodes(){

	vector < Ptr<Node> > slaveList = getNodeList();
	Ptr<Node> master = getMaster();
		
		for(vector<Ptr<Node> >::iterator it = slaveList.begin(); it != slaveList.end(); it++)
		{
		  if (*it == master)
		  {
			slaveList.erase(it);
			break;  //it is now invalud must break!
		  }
		}
		return slaveList;
}

	void Cluster::setMaster(Ptr<Node> master){
		
		masterNode = master;
	}


