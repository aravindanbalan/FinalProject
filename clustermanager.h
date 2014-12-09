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
#include <math.h> 
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
		std::map<Ptr<Node>, string > MasterSlaveAsssociationMap;
		static TypeId tid;
		map<Ptr<Node>,int> nodeMap;
		map<Ptr<Node>,int> nodeSelfishMap;
		bool done;
		int distanceCheck;
		double threshold;
		std::map<Ptr<Node>, Vector> currentNodeLocation;
		vector<int> chosenMastersset;
 
	public:
		virtual ~ClusterManager ();
		void join_Cluster(Ptr<Node> node, int nodeID, int topic, int selfish);
		void leave_Cluster(Ptr<Node> node, int nodeID);
		Cluster* createNewCluster(int clusterID, int topic); 
		bool choose_Master(int clusterid, int numMasters,NodeContainer nodes);
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
		void putNodeInSelfishMap(Ptr<Node> node,int index);
		int getNodeFromSelfishMap(Ptr<Node> node);
	
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
		vector<int> getMasterNodeIDsFromCluster(int clusterID);
		vector<Ptr<Node> > getSlaveNodesFromCluster(int clusterID);
		string getSlaveNodeIDsFromCluster(int clusterID);
		vector<int> getMasterNodeIDsFromSlaveID(Ptr<Node> slaveNode);
		bool isDone();
		void setDone(bool value);
		int numofMastersrecv;
		void setCurrentNodeLocation(Ptr<Node> node,Vector value);
		Vector getCurrentNodeLocation(Ptr<Node> node);
		void setDistanceCheck(bool value);
		void setMasterCheckParameters(int check, double distthreshold);
		bool checkDistance(Vector pos, Vector BSpos);

		void putMasterSlaveInMap(Ptr<Node> master,string slavestr);
		string getSlaveStrForMasterFromMap(Ptr<Node> node);
		bool checkIfAlreadyChosenBefore(int slave);
};

bool ClusterManager::checkIfAlreadyChosenBefore(int slave){
		
	for(uint32_t i =0;i< chosenMastersset.size();i++)
		{
			if(chosenMastersset[i] == slave)
				return true;
		}
		
	return false;
}


void ClusterManager::setMasterCheckParameters(int check, double distthreshold){
	
	distanceCheck = check;
	threshold = distthreshold;
}


void ClusterManager::setDistanceCheck(bool value){
	distanceCheck = value; 
}

void ClusterManager::setCurrentNodeLocation(Ptr<Node> node,Vector value){
	
	map<Ptr<Node>,Vector>::iterator p;
		p = currentNodeLocation.find(node);
		if(p != currentNodeLocation.end())
			currentNodeLocation[node] = value;
		else
			currentNodeLocation.insert(pair<Ptr<Node>,Vector>(node,value));
}
Vector ClusterManager::getCurrentNodeLocation(Ptr<Node> node){
	
	map<Ptr<Node>,Vector>::iterator p;
		p = currentNodeLocation.find(node);
		return p->second;
}
		

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


void ClusterManager::putNodeInSelfishMap(Ptr<Node> node,int index)
{
	nodeSelfishMap.insert(pair<Ptr<Node>,int>(node,index));
}

int ClusterManager::getNodeFromSelfishMap(Ptr<Node> node)
{
	map<Ptr<Node>,int>::iterator p;
	p = nodeSelfishMap.find(node);
	if(p != nodeSelfishMap.end())
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
		
		map<int, Cluster* >::iterator p;
		p = clusterMap.find(clusterID);
		if(p != clusterMap.end())
		{
			return p->second;
		}
		else 
		{
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

	void ClusterManager::join_Cluster(Ptr<Node> node, int nodeID, int topic, int selfish){
		
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
			putNodeInSelfishMap(node,selfish);
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
			putNodeInSelfishMap(node,selfish);
			
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
	}
	
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

	vector<int> ClusterManager::getMasterNodeIDsFromCluster(int clusterID){
		
		Cluster* cluster = getClusterFromClusterID(clusterID);
		vector<Ptr<Node> > masters  = cluster->getMasters();
		vector<int> nodeIDs;
		for(uint32_t i = 0;i < masters.size();i++)
			nodeIDs.push_back(getNodeIDForNode(masters[i]));
			
		return nodeIDs;
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

	bool ClusterManager::checkDistance(Vector pos, Vector BSpos)
	{
		double val = pow ((BSpos.x - pos.x),2.0) + pow((BSpos.y - pos.y),2.0) + pow((BSpos.z - pos.z),2.0); 
		double dist = sqrt(val);
		return (dist < threshold);
	}
/*
	bool ClusterManager::choose_Master(int clusterID, int numMasters, NodeContainer nodes){

		Cluster* cluster = getClusterFromClusterID(clusterID);

		std::vector < Ptr<Node> > clusterNodes = cluster->getNodeList();
		
		/////// New Algorithm /////////////////////
		unsigned int numNodes = clusterNodes.size();
		int random_master_index;
		Ptr<Node> masterNode;
		int numChosenMasters = 0;
		vector<Ptr<Node> > newMasters; 
		int retries = 0;
		int find_master_retry_threshold = 10;
		
			while(1 && numChosenMasters < numMasters)
			{
				random_master_index = randomNumberGenerator(numNodes); 
				//std::cout<<"random : "<<random_master_index<<std::endl;
				masterNode = clusterNodes.at(random_master_index);
				int nodeID = getNodeIDForNode(masterNode);
				Vector pos = getCurrentNodeLocation(masterNode);
				Vector BSpos = getCurrentNodeLocation(getClusterMgrNode());
				std::cout.precision(5);
				//cout << Simulator::Now () << " Master choice NODE: " << nodeID << "; POS: x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << std::endl;
				//cout << Simulator::Now () << " Base Station NODE: " << 50 << "; POS: x=" << BSpos.x << ", y=" << BSpos.y << ", z=" << BSpos.z << std::endl;
				
				
				vector<int>::iterator got;
				
				// check the distance of the current master from the BS and check for the threshold
					
				if(distanceCheck){
					
						if(checkDistance(pos,BSpos)){
							
							got = find (chosenMastersSet.begin(), chosenMastersSet.end(), nodeID);
							if(got == chosenMastersSet.end()){ // element not found, then insert it
								chosenMastersSet.push_back(nodeID);
								numChosenMasters++;
								retries = 0;
								//cout<<"pushing chosen masters :" <<nodeID<<endl;
								newMasters.push_back(masterNode);
								//break;
							}
						}
						else
						{
							//incr number of retries as we didnt chose a master now because of range issues
							retries++;
							if(retries > find_master_retry_threshold )
								return false;
						}
				}	
				else{
					
					got = find (chosenMastersSet.begin(), chosenMastersSet.end(), nodeID);
							if(got == chosenMastersSet.end()){ // element not found, then insert it
								chosenMastersSet.push_back(nodeID);
								numChosenMasters++;
								//cout<<"pushing chosen masters :" <<nodeID<<endl;
								newMasters.push_back(masterNode);
								//break;
							}
				}
			}//while ends here
			
		
		cluster->setMaster(newMasters);
		
		// set the slaves fro this cluster
		
		std::vector < Ptr<Node> > slaveNodes = clusterNodes;
		std::vector < Ptr<Node> > newSlaveList;
		for(vector<Ptr<Node> >::iterator it = slaveNodes.begin(); it != slaveNodes.end(); it++)
		{
		  if(!(std::find(newMasters.begin(), newMasters.end(), *it) != newMasters.end()))
		  {
			  newSlaveList.push_back(*it);
		  } 
		}
		
		cout<<"********************************************************Slave size : "<<newSlaveList.size()<<endl;
	
		cluster->setSlaveNodes(newSlaveList);
		
		
		if(chosenMastersSet.size() == numNodes)
			chosenMastersSet.erase (chosenMastersSet.begin(),chosenMastersSet.end());
			
		//////////////////////////////////////
		
		
		putClusterForClusterID(clusterID, cluster);
		//cout<<"1111111111111111"<<endl;
		string slaveString = getSlaveNodeIDsFromCluster(clusterID);
		//cout<<"11111111111111122222222221"<<endl;
		
		vector<int> masterNodeIDs = getMasterNodeIDsFromCluster(clusterID);
		for(uint32_t i =0;i < masterNodeIDs.size();i++)
		{
			//cout<<" "<<masterNodeIDs[i];
			int masterID =  masterNodeIDs[i];
			
			putMasterSlaveInMap(nodes.Get(masterID),slaveString);
		}
		
		return true;
		
	}
	
	*/
	
	bool ClusterManager::choose_Master(int clusterID, int numMasters, NodeContainer nodes){
		
		Cluster* cluster = getClusterFromClusterID(clusterID);
		std::vector < Ptr<Node> > clusterNodes = cluster->getNodeList();
		std::vector < Ptr<Node> > candidates;
		vector<Ptr<Node> > newMasters; 
		
		Vector BSpos = getCurrentNodeLocation(getClusterMgrNode());
		int numNodes = clusterNodes.size();
		
		if(distanceCheck){
			
			
			//step 1 - get all possible candidates
			for(int i=0;i<numNodes;i++){
				
				Ptr<Node> node = clusterNodes.at(i);
				int nodeID = getNodeIDForNode(node);
				Vector pos = getCurrentNodeLocation(node);
				vector<int>::iterator got;
				
				// check distance between current node and base station
				if(checkDistance(pos,BSpos)){
					
					got = find (chosenMastersSet.begin(), chosenMastersSet.end(), nodeID);
					if(got == chosenMastersSet.end()){ 
						// element not found, then insert it in the candidate set
						candidates.push_back(node);
					}
				}	
			}
			
			//step 2 - check if candidate size > = 10
			
			int size = candidates.size();
			if(size >= numMasters)
			{
				int i = 0;
				vector<Ptr<Node> >::iterator chosenAlready;
				while(i < numMasters)
				{
					int random_master_index = randomNumberGenerator(candidates.size()); 					
					Ptr<Node> current_candidate = candidates.at(random_master_index);
					chosenAlready = find (newMasters.begin(), newMasters.end(), current_candidate);
					if(chosenAlready == newMasters.end())
					{ 
						// not found 
						newMasters.push_back(current_candidate);
						int nodeID = getNodeIDForNode(current_candidate);
						chosenMastersSet.push_back(nodeID);
						chosenMastersset.push_back(nodeID);
						i++;
					}				
				}
			}
			else
			{
				// if candidate size < 10, then we need to choose a few from previous rounds
				
				// push everything first as all are eligible
				for(vector<Ptr<Node> >::iterator it = candidates.begin(); it != candidates.end(); it++)
				{
					  newMasters.push_back(*it); 
					  int nodeID = getNodeIDForNode(*it); 
					  chosenMastersset.push_back(nodeID);
				}
				
				// pick the remaining values from the chosen master set, once that is done clear chosen master set	
				int i=0;
				while(i < (numMasters - candidates.size()))
				{	
					int random_master_index = randomNumberGenerator(chosenMastersSet.size()); 					
					int current_candidate_index = chosenMastersSet.at(random_master_index);
					Ptr<Node> current_candidate_node = nodes.Get(current_candidate_index);
					newMasters.push_back(current_candidate_node);
					i++;
				}
				
				// now clear the contents of chosen master set
				chosenMastersSet.erase (chosenMastersSet.begin(),chosenMastersSet.end());
					
			}			
		}
		else{
			//normal method without distance check
	
			// step 1 - get the candidates
			for(int i=0;i<numNodes;i++){
				
				Ptr<Node> node = clusterNodes.at(i);
				int nodeID = getNodeIDForNode(node);
				vector<int>::iterator got;
					
					got = find (chosenMastersSet.begin(), chosenMastersSet.end(), nodeID);
					if(got == chosenMastersSet.end()){ 
						// element not found, then insert it in the candidate set
						candidates.push_back(node);
					}
			}
			
			// step 2 - check if candidate size >= nummasters
			int size = candidates.size();
			if(size >= numMasters)
			{
				int i = 0;
				vector<Ptr<Node> >::iterator chosenAlready;
				while(i < numMasters)
				{
					int random_master_index = randomNumberGenerator(candidates.size()); 					
					Ptr<Node> current_candidate = candidates.at(random_master_index);
					chosenAlready = find (newMasters.begin(), newMasters.end(), current_candidate);
					if(chosenAlready == newMasters.end())
					{ 
						// not found 
						newMasters.push_back(current_candidate);
						int nodeID = getNodeIDForNode(current_candidate);
						chosenMastersSet.push_back(nodeID);
						chosenMastersset.push_back(nodeID);
						i++;
					}				
				}
			}
			else{
				
				// if candidate size < 10, then we need to choose a few from previous rounds
				
				// push everything first as all are eligible
				for(vector<Ptr<Node> >::iterator it = candidates.begin(); it != candidates.end(); it++)
				{
					  newMasters.push_back(*it); 
					  int nodeID = getNodeIDForNode(*it); 
					  chosenMastersset.push_back(nodeID);
				}
				
				// pick the remaining values from the chosen master set, once that is done clear chosen master set	
				int i=0;
				while(i < (numMasters - candidates.size()))
				{	
					int random_master_index = randomNumberGenerator(chosenMastersSet.size()); 					
					int current_candidate_index = chosenMastersSet.at(random_master_index);
					Ptr<Node> current_candidate_node = nodes.Get(current_candidate_index);
					newMasters.push_back(current_candidate_node);
					i++;
				}
				
				// now clear the contents of chosen master set
				chosenMastersSet.erase (chosenMastersSet.begin(),chosenMastersSet.end());
				
			}
			
			
		}// else ends here
		
		//set new master set for the cluster
		
		cluster->setMaster(newMasters);
		// set the slaves for this cluster
		
		std::vector < Ptr<Node> > slaveNodes = clusterNodes;
		std::vector < Ptr<Node> > newSlaveList;
		for(vector<Ptr<Node> >::iterator it = slaveNodes.begin(); it != slaveNodes.end(); it++)
		{
		  if(!(std::find(newMasters.begin(), newMasters.end(), *it) != newMasters.end()))
		  {
			  newSlaveList.push_back(*it);
		  } 
		}
		
		cluster->setSlaveNodes(newSlaveList);
		
		
		// update the cluster in map
		putClusterForClusterID(clusterID, cluster);
		
		string slaveString = getSlaveNodeIDsFromCluster(clusterID);;	
		vector<int> masterNodeIDs = getMasterNodeIDsFromCluster(clusterID);
		for(uint32_t i =0;i < masterNodeIDs.size();i++)
		{
			int masterID =  masterNodeIDs[i];
			putMasterSlaveInMap(nodes.Get(masterID),slaveString);
		}
		
		for(uint32_t i =0;i < chosenMastersSet.size();i++)
		{
			cout<<" "<<chosenMastersSet.at(i);
		}
		cout<<endl;
		
		
		return true;
	}//function ends here
	
	int ClusterManager::getTopicFromNode(Ptr<Node> node){
		
		int clusterID = getClusterIDFromNode(node);
		Cluster* cluster = getClusterFromClusterID(clusterID);
		return cluster->getTopic();
		
	}
	
	vector<int> ClusterManager::getMasterNodeIDsFromSlaveID(Ptr<Node> slaveNode){
		
		int clusterID = getClusterIDFromNode(slaveNode);
		return getMasterNodeIDsFromCluster(clusterID);
	}

	void ClusterManager::putMasterSlaveInMap(Ptr<Node> master,string slavestr){
		
		
	MasterSlaveAsssociationMap.insert(pair<Ptr<Node>,string>(master,slavestr));
		
	}
	string ClusterManager::getSlaveStrForMasterFromMap(Ptr<Node> node){
		
		map<Ptr<Node>,string>::iterator p;
		p = MasterSlaveAsssociationMap.find(node);
		if(p != MasterSlaveAsssociationMap.end())
			return p->second;
		else 
			return "";	
	}
