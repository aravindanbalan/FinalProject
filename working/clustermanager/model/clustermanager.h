/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef CLUSTERMANAGER_H
#define CLUSTERMANAGER_H


#include "ns3/application.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/average.h"
#include "ns3/simulator.h"
#include <map>

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
#include "ns3/cluster.h"

namespace ns3 {

/* ... */

using namespace std;
class Socket;

class ClusterManager : public Application
{
	private:
		// Functions
		virtual void StartApplication (void);
		virtual void StopApplication (void);

		void ScheduleTransmit (Time dt);
		void Send(void);

		void HandleRead (Ptr<Socket> socket);
		
		// Variables
		Ipv4Address peerAddress;
		Ptr<Socket> m_socket;
		uint16_t peerPort;
		bool sending;
		uint32_t dataSize;
		uint32_t m_sent;
		uint32_t m_received;
		bool broadcast; 
		
		DataRate dataRate;
		EventId sendEvent;
		int packet_Type;
		
		static int numClusters;
		static bool instanceFlag;
		//cluster node to send and receive packets
		Ptr<Node> clusterMgrNode;
		//Ptr<Socket> clusterMgrSocket;
		NodeContainer clusterMgrNodeContainer;
		/////////
		static ClusterManager *clusterMgr;
		std::map<int, Cluster*> clusterMap;
		std::map<Ptr<Node>, int > nodeClusterIDAssociationMap;
		std::map<int, int > topicClusterIDAssociationMap;
		std::map<Ptr<Node>, int > nodeNodeIDAssociationMap;
		static TypeId tid;
		
		
	public:
		virtual ~ClusterManager ();
		void Setup (Ipv4Address address, uint16_t port, DataRate dr, bool toSend, bool broadcastaddr, int packetType);
		void join_Cluster(Ptr<Node> node, int nodeID, int topic);
		void leave_Cluster(Ptr<Node> node, int nodeID);
		Cluster* createNewCluster(int clusterID, int topic); 
		//void choose_Master(int clusterid);		---- have to do later in second phase
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
		void eraseAllMaps();
		ClusterManager();
		int getMasterNodeIDFromCluster(int clusterID);
		vector<Ptr<Node> > getSlaveNodesFromCluster(int clusterID);
		string getSlaveNodeIDsFromCluster(int clusterID);

		
};

}

#endif /* CLUSTERMANAGER_H */

