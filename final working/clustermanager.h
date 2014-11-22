#include "ns3/socket.h" 
#include "ns3/application.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/average.h"
#include "ns3/simulator.h"
#include <map>

#include "cluster.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/event-id.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/log.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdlib>

using namespace ns3;
using namespace std;

class ClusterManager
{
	private:
		vector<int> chosenMastersSet;
		static int numClusters;
		static bool instanceFlag;
		Ptr<Node> clusterMgrNode;
		NodeContainer clusterMgrNodeContainer;
		static ClusterManager *clusterMgr;
		std::map<int, Cluster*> clusterMap;
		std::map<Ptr<Node>, int > nodeClusterIDAssociationMap;
		std::map<int, int > topicClusterIDAssociationMap;
		std::map<Ptr<Node>, int > nodeNodeIDAssociationMap;
		static TypeId tid;
		map<Ptr<Node>,int> nodeMap;
		bool done;
		
	public:
		virtual ~ClusterManager ();
		void join_Cluster(Ptr<Node> node, int nodeID, int topic);
		void leave_Cluster(Ptr<Node> node, int nodeID);
		Cluster* createNewCluster(int clusterID, int topic); 
		void choose_Master(int clusterid);		//---- have to do later in second phase
		bool isMaster(Ptr<Node> node);
		static ClusterManager* getInstance();
		int getNumberOfClusters();
		int getClusterIDFromTopic(int topic);
		int getClusterIDFromNode(Ptr<Node> node);
		void putClusterIDForTopic(int topic, int clusterID);
		void putClusterIDForNode( Ptr<Node> node, int clusterID); 
		void removeClusterIDForNode(Ptr<Node> node);
		void removeClusterIDForTopic(int topic);
		void removeClusterIDFromClusterMap(int clusterID);
		int getTopicFromNode(Ptr<Node> node);
		
		void putNodeInMap(Ptr<Node> node,int index);
		int getNodeFromMap(Ptr<Node> node);
	
		Cluster* getClusterFromClusterID(int clusterID);
		void putClusterForClusterID(int clusterID, Cluster* cluster);	
		NodeContainer getNodeContainer();
		void setNodeContainer(NodeContainer nodeC);
		Ptr<Node> getClusterMgrNode();
		void setClusterMgrNode(Ptr<Node> node);
		
		int getNodeIDForNode(Ptr<Node> node);
		void setNodeIDForNode(Ptr<Node> node, int nodeID);
		void eraseAllMaps();
		ClusterManager();
		int getMasterNodeIDFromCluster(int clusterID);
		vector<Ptr<Node> > getSlaveNodesFromCluster(int clusterID);
		string getSlaveNodeIDsFromCluster(int clusterID);
		int getMasterNodeIDFromSlaveID(Ptr<Node> slaveNode);
		bool isDone();
		void setDone(bool value);
		int numofMastersrecv;
		
};

void ClusterManager::putNodeInMap(Ptr<Node> node,int index)
{
	nodeMap.insert(pair<Ptr<Node>,int>(node,index));
}

int ClusterManager::getNodeFromMap(Ptr<Node> node)
{
	map<Ptr<Node>,int>::iterator p;
	p = nodeMap.find(node);
	if(p != nodeMap.end())
		return p->second;
	else 
		return -1;	
}

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
ClusterManager::ClusterManager ()
	{
		
		instanceFlag = false;
		numClusters = 0;
		done = false;
		numofMastersrecv = 0;
	}

	ClusterManager::~ClusterManager ()
	{
	
		instanceFlag = false;
		numClusters = 0;
	}

	bool ClusterManager::isDone(){
		return done;
	}

	void ClusterManager::setDone(bool value){
		
		done = value;
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
		if(p != nodeClusterIDAssociationMap.end())
		{
			return p->second;
		}
		else 
		{
			//cout<<"no cluster id for node\n";
			return -999;
		}	
		
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
		
		//std::cout<<"*******************"<<endl;
		map<int, Cluster* >::iterator p;

		p = clusterMap.find(clusterID);
		//std::cout<<"11*******************"<<endl;
		if(p != clusterMap.end())
		{
			//std::cout<<"is there*******************"<<endl;
			return p->second;
		}
		else 
		{
			//std::cout<<"null*******************"<<endl;
			return NULL;
		}	
		
	}

	void ClusterManager::putClusterForClusterID(int clusterID, Cluster* cluster){
		
		map<int, Cluster* >::iterator p;
		p = clusterMap.find(clusterID);
		if(p != clusterMap.end())
			clusterMap[clusterID] = cluster;
		else
			clusterMap.insert(pair<int,Cluster* >(clusterID,cluster));
	}

	Cluster* ClusterManager::createNewCluster(int clusterID, int topic){
		
		Cluster* newCluster = new Cluster(clusterID, topic);
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
			putClusterForClusterID(clusterID,cluster);
			putClusterIDForTopic(topic,clusterID);
		}
		else{
			
			int clusterID = numClusters;
			Cluster* cluster = createNewCluster(clusterID, topic);
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

	void ClusterManager::leave_Cluster(Ptr<Node> node, int nodeID){
		
		int clusterID = getClusterIDFromNode(node);
		if(clusterID != -999)
		{
			Cluster *cluster = getClusterFromClusterID(clusterID);
			cluster->removeNodeFromCluster(node);
			int clustersize = cluster->getNumNodes();
			if(clustersize == 0)
			{
				//remove all cluster references
				removeClusterIDForNode(node);
				int topic = cluster->getTopic();
				removeClusterIDForTopic(topic);
				removeClusterIDFromClusterMap(clusterID);		
			}
			
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

	void ClusterManager::eraseAllMaps(){
		
		 map<int, Cluster*>::iterator p;
		 clusterMap.erase ( p, clusterMap.end() );
		 
		 map<Ptr<Node>, int >::iterator p1;
		 nodeClusterIDAssociationMap.erase ( p1, nodeClusterIDAssociationMap.end() );
		 
		 map<int, int >::iterator p2;
		 topicClusterIDAssociationMap.erase ( p2, topicClusterIDAssociationMap.end() );
		 
	}

	void ClusterManager::removeClusterIDForNode(Ptr<Node> node){
		
		nodeClusterIDAssociationMap.erase(node);
	}
	void ClusterManager::removeClusterIDForTopic(int topic){
		
		topicClusterIDAssociationMap.erase(topic); 
	}
	void ClusterManager::removeClusterIDFromClusterMap(int clusterID){
		
		clusterMap.erase(clusterID); 
		numClusters--;
	}

	int ClusterManager::getMasterNodeIDFromCluster(int clusterID){
		
		Cluster* cluster = getClusterFromClusterID(clusterID);
		return getNodeIDForNode(cluster->getMaster());
	}

	vector<Ptr<Node> > ClusterManager::getSlaveNodesFromCluster(int clusterID){
		
		Cluster* cluster = getClusterFromClusterID(clusterID);
		return cluster->getSlaveNodes();
	}
	
	string ClusterManager::getSlaveNodeIDsFromCluster(int clusterID){
		
		vector<Ptr<Node> > slaves = getSlaveNodesFromCluster(clusterID);
		//vector<int> slaveIDs;
		
		string slaveString;
		
		for( vector<Ptr<Node> >::iterator iter = slaves.begin(); iter != slaves.end(); ++iter )
		{
			Ptr<Node> node = *iter;
			//slaveIDs.push_back(getNodeIDForNode(node));
			int nodeID = getNodeIDForNode(node);
			std::ostringstream s;
			s<<nodeID;
			std::string ss(s.str());
			slaveString.append(ss);
			slaveString.append(1,',');
			
		}
		slaveString = slaveString.substr(0, slaveString.size()-1);
		
		return slaveString;
	}
	
	int randomNumberGenerator(int numNodes)
	{
			int v1 = rand() % numNodes; 
			return v1;
	}

	void ClusterManager::choose_Master(int clusterID){
		
		// as of now choosing the first node as master in each cluster
		//std::cout<<"1111"<<std::endl;
		Cluster* cluster = getClusterFromClusterID(clusterID);
		//std::cout<<"222"<<std::endl;
		std::vector < Ptr<Node> > clusterNodes = cluster->getNodeList();
		
		//std::cout<<"1333"<<std::endl;
		
		// setting the first node in the list to be the master
		// replace this with the algorithm needed below
		
		//cluster->setMaster(clusterNodes.front());
		
		
		/////// New Algorithm /////////////////////
		unsigned int numNodes = clusterNodes.size();
		int random_master_index;
		Ptr<Node> masterNode;
		
		while(1)
		{
			random_master_index = randomNumberGenerator(numNodes); 
			//std::cout<<"random : "<<random_master_index<<std::endl;
			masterNode = clusterNodes.at(random_master_index);
			
			vector<int>::iterator got;
			
			got = find (chosenMastersSet.begin(), chosenMastersSet.end(), random_master_index);
			if(got == chosenMastersSet.end()){ // element not found, then insert it
				chosenMastersSet.push_back(random_master_index);
				break;
			}
		}
		
		cluster->setMaster(masterNode);
		if(chosenMastersSet.size() == numNodes)
			chosenMastersSet.erase (chosenMastersSet.begin(),chosenMastersSet.end());
		
		//////////////////////////////////////
		
		putClusterForClusterID(clusterID, cluster);
		
	}
	int ClusterManager::getTopicFromNode(Ptr<Node> node){
		
		int clusterID = getClusterIDFromNode(node);
		Cluster* cluster = getClusterFromClusterID(clusterID);
		return cluster->getTopic();
		
	}
	
	int ClusterManager::getMasterNodeIDFromSlaveID(Ptr<Node> slaveNode){
		
		int clusterID = getClusterIDFromNode(slaveNode);
		return getMasterNodeIDFromCluster(clusterID);
	}


