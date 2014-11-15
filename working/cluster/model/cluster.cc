/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "cluster.h"

namespace ns3 {

/* ... */
using namespace std;
	NS_LOG_COMPONENT_DEFINE ("Cluster");

	NS_OBJECT_ENSURE_REGISTERED (Cluster);
	
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

	vector< Ptr<Node> > Cluster::getSlaveNodes(){

		vector < Ptr<Node> > slaveList = getNodeList();
		Ptr<Node> master = getMaster();

		/*
		vector<Ptr<Node> >::iterator position = std::find(slaveList.begin(), slaveList.end(), master);	
		if (position != slaveList.end())
			slaveList.erase(position);
			*/
			
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


}

