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
		int topic;
		int numNodes;
		Ptr<Node> masterNode;
		vector<Ptr<Node> > nodeList;
	
	public:
	    
	    Cluster(int ID, int top)
	    {
			clusterID = ID;
			numNodes = 0;
			topic = top;
		}
		Ptr<Node> getMaster();
		bool isMaster(Ptr<Node> node);
		vector < Ptr<Node> > getNodeList();
		void addNodeToCluster(Ptr<Node> node);
		void removeNodeFromCluster(Ptr<Node> node);
		int getNumNodes();
		int getTopic();	
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
		
		//by default set the first node as master as of now
		// instead in future use an algorithm for choosing the master which will be done by the clusterManager and not by the Cluster class
		
		if(numNodes == 1)
			masterNode = node;
		
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
