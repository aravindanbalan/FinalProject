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
		static ClusterManager *clusterMgr;
		map<int, Cluster*> clusterMap;
		map<Ptr<Node>, int > nodeClusterIDAssociationMap;
		map<int, int > topicClusterIDAssociationMap;
		ClusterManager()
		{
		 //private constructor
		}
	
	public:
		
		void join_Cluster(Ptr<Node> node, int topic);
		//void leave_Cluster(Ptr<Node> node);
		Cluster* createNewCluster(int clusterID); 
		//void choose_Master(int clusterid);		---- have to do later in second phase
		bool isMaster(Ptr<Node> node);
		static ClusterManager* getInstance();
		int getClusterIDFromTopic(int topic);
		int getClusterIDFromNode(Ptr<Node> node);
		void putClusterIDForTopic(int topic, int clusterID);
		void putClusterIDForNode( Ptr<Node> node, int clusterID); 
		Cluster* getClusterFromClusterID(int clusterID);
		void putClusterForClusterID(int clusterID, Cluster* cluster);	
	
        ~ClusterManager()
        {
			instanceFlag = false;
			numClusters = -1;
        }
		
};

bool ClusterManager::instanceFlag = false;
int ClusterManager::numClusters = -1;
ClusterManager* ClusterManager::clusterMgr = NULL;

ClusterManager* ClusterManager::getInstance()
{
	if(!instanceFlag)
        {		
		clusterMgr = new ClusterManager();
		instanceFlag = true;
		numClusters = -1;
	}
        return clusterMgr;
    
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

void ClusterManager::join_Cluster(Ptr<Node> node, int topic){
	
	map<int, int>::iterator p;
	p = topicClusterIDAssociationMap.find(topic);
	if(p != topicClusterIDAssociationMap.end())
	{
		int clusterID = topicClusterIDAssociationMap[topic];
		
		//get the cluster from ClsuterMap and add the node to the cluster
		Cluster* cluster = getClusterFromClusterID(clusterID);
		cluster->addNodeToCluster(node);
		putClusterIDForNode(node,clusterID);
	}
	else{
		
		int clusterID = numClusters + 1;
		Cluster* cluster = createNewCluster(clusterID);
		cluster->addNodeToCluster(node);
		
		// add in three places
		//1. topic<--> clusterID map
		// 2. clusterMap
		// 3. node<--> clusterID map
		
		putClusterIDForTopic(topic,clusterID);
		putClusterForClusterID(clusterID,cluster);
		putClusterIDForNode(node,clusterID);
		
		//increment the number of clusters in the system
		numClusters++;
		
	}
}

bool ClusterManager::isMaster(Ptr<Node> node){
	
	int clusterID = getClusterIDFromNode(node);
	Cluster* cluster = getClusterFromClusterID(clusterID);
	return cluster->isMaster(node);
}
