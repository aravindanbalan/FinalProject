#include "ns3/core-module.h"
#include <iostream>
#include <map>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include "Cluster.h"

using namespace ns3;
using namespace std;
 
class ClusterManager{
	
	private:
		static int numClusters;
		static bool instanceFlag;
		//cluster node to send and receive packets
		Ptr<Node> clusterMgrNode;
		//Ptr<Socket> clusterMgrSocket;
		NodeContainer clusterMgrNodeContainer;
		/////////
		static ClusterManager *clusterMgr;
		map<int, Cluster*> clusterMap;
		map<Ptr<Node>, int > nodeClusterIDAssociationMap;
		map<int, int > topicClusterIDAssociationMap;
		map<Ptr<Node>, int > nodeNodeIDAssociationMap;
		static TypeId tid;
		ClusterManager()
		{
		 //private constructor
		}
	
	public:
		
		void join_Cluster(Ptr<Node> node, int nodeID, int topic);
		//void leave_Cluster(Ptr<Node> node);
		Cluster* createNewCluster(int clusterID); 
		//void choose_Master(int clusterid);		---- have to do later in second phase
		bool isMaster(Ptr<Node> node);
		static ClusterManager* getInstance();
		int getNumberOfClusters();
		int getClusterIDFromTopic(int topic);
		int getClusterIDFromNode(Ptr<Node> node);
		void putClusterIDForTopic(int topic, int clusterID);
		void putClusterIDForNode( Ptr<Node> node, int clusterID); 
		Cluster* getClusterFromClusterID(int clusterID);
		void putClusterForClusterID(int clusterID, Cluster* cluster);	
		NodeContainer getNodeContainer();
		void setNodeContainer(NodeContainer nodeC);
		Ptr<Node> getClusterMgrNode();
		void setClusterMgrNode(Ptr<Node> node);
		//Ptr<Socket> getClusterMgrSocket();
		//void setClusterMgrSocket(Ptr<Node> node);
		int getNodeIDForNode(Ptr<Node> node);
		void setNodeIDForNode(Ptr<Node> node, int nodeID);
		
        ~ClusterManager()
        {
			instanceFlag = false;
			numClusters = 0;
        }
		
};

bool ClusterManager::instanceFlag = false;
int ClusterManager::numClusters = 0;
TypeId ClusterManager::tid;
ClusterManager* ClusterManager::clusterMgr = NULL;

ClusterManager* ClusterManager::getInstance()
{
	if(!instanceFlag)
        {		
		clusterMgr = new ClusterManager();
		instanceFlag = true;
		numClusters = 0;
		tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	}
        return clusterMgr;
    
}	

int ClusterManager::getNumberOfClusters(){
	return numClusters;
}

int ClusterManager::getClusterIDFromTopic(int topic){
	
	map<int,int>::iterator p;
	p = topicClusterIDAssociationMap.find(topic);
	return p->second;
}

int ClusterManager::getClusterIDFromNode(Ptr<Node> node){

	map<Ptr<Node>,int>::iterator p;
	p = nodeClusterIDAssociationMap.find(node);
	return p->second;
	
}

void ClusterManager::putClusterIDForTopic(int topic, int clusterID){
	
	map<int,int>::iterator p;
	p = topicClusterIDAssociationMap.find(topic);
	if(p != topicClusterIDAssociationMap.end())
		topicClusterIDAssociationMap[topic] = clusterID;
	else
	topicClusterIDAssociationMap.insert(pair<int,int>(topic,clusterID));
}

void ClusterManager::putClusterIDForNode(Ptr<Node> node, int clusterID){
	
	map<Ptr<Node>,int>::iterator p;
	p = nodeClusterIDAssociationMap.find(node);
	if(p != nodeClusterIDAssociationMap.end())
		nodeClusterIDAssociationMap[node] = clusterID;
	else
	nodeClusterIDAssociationMap.insert(pair<Ptr<Node>,int>(node,clusterID));
}	

Cluster* ClusterManager::getClusterFromClusterID(int clusterID){
	
	map<int, Cluster* >::iterator p;
	p = clusterMap.find(clusterID);
	return p->second;
}

void ClusterManager::putClusterForClusterID(int clusterID, Cluster* cluster){
	
	map<int, Cluster* >::iterator p;
	p = clusterMap.find(clusterID);
	if(p != clusterMap.end())
		clusterMap[clusterID] = cluster;
	else
		clusterMap.insert(pair<int,Cluster* >(clusterID,cluster));
}

Cluster* ClusterManager::createNewCluster(int clusterID){
	
	Cluster* newCluster = new Cluster(clusterID);
	return newCluster;
}

void ClusterManager::join_Cluster(Ptr<Node> node, int nodeID, int topic){
	
	map<int, int>::iterator p;
	p = topicClusterIDAssociationMap.find(topic);
	if(p != topicClusterIDAssociationMap.end())
	{
		int clusterID = topicClusterIDAssociationMap[topic];
		
		//get the cluster from ClsuterMap and add the node to the cluster
		Cluster* cluster = getClusterFromClusterID(clusterID);
		cluster->addNodeToCluster(node);
		
		setNodeIDForNode(node,nodeID);
		putClusterIDForNode(node,clusterID);
	}
	else{
		
		int clusterID = numClusters;
		Cluster* cluster = createNewCluster(clusterID);
		//increment the number of clusters in the system as we created a new cluster
		numClusters++;
		cluster->addNodeToCluster(node);
		
		// add in three places
		//1. topic<--> clusterID map
		// 2. clusterMap
		// 3. node<--> clusterID map
		
		putClusterIDForTopic(topic,clusterID);
		putClusterForClusterID(clusterID,cluster);
		putClusterIDForNode(node,clusterID);
		setNodeIDForNode(node,nodeID);
		
	}

}

bool ClusterManager::isMaster(Ptr<Node> node){
	
	int clusterID = getClusterIDFromNode(node);
	Cluster* cluster = getClusterFromClusterID(clusterID);
	return cluster->isMaster(node);
}


NodeContainer ClusterManager::getNodeContainer(){
	
	return clusterMgrNodeContainer;
}

void ClusterManager::setNodeContainer(NodeContainer nodeC){
	
	clusterMgrNodeContainer = nodeC;
	setClusterMgrNode(nodeC.Get (0));
}

Ptr<Node> ClusterManager::getClusterMgrNode(){
	
	return clusterMgrNode;
}

void ClusterManager::setClusterMgrNode(Ptr<Node> node){
	
	clusterMgrNode = node;
	//setClusterMgrSocket(node);
}
/*
Ptr<Socket> ClusterManager::getClusterMgrSocket(){
	
	return clusterMgrSocket;
}
void ClusterManager::setClusterMgrSocket(Ptr<Node> node){
	
	 clusterMgrSocket = Socket::CreateSocket (node, tid);
}
*/
int ClusterManager::getNodeIDForNode(Ptr<Node> node){
	
	map<Ptr<Node>, int >::iterator p;
	p = nodeNodeIDAssociationMap.find(node);
	return p->second;
}
void ClusterManager::setNodeIDForNode(Ptr<Node> node, int nodeID){
	
	map<Ptr<Node>, int >::iterator p;
	p = nodeNodeIDAssociationMap.find(node);
	if(p != nodeNodeIDAssociationMap.end())
		nodeNodeIDAssociationMap[node] = nodeID;
	else
		nodeNodeIDAssociationMap.insert(pair<Ptr<Node>,int>(node,nodeID));
}
