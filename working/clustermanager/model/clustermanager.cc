/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "clustermanager.h"
#include "ns3/icmpv4.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "MyTag.h"
namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("ClusterManager");

	NS_OBJECT_ENSURE_REGISTERED (ClusterManager);


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
		NS_LOG_FUNCTION (this);
		m_socket = 0;
		sendEvent = EventId ();
		sending = false;
		broadcast = false;
		dataSize = 256;
		m_sent = 0;
		m_received = 0;
		instanceFlag = false;
		numClusters = 0;
	}

	ClusterManager::~ClusterManager ()
	{
		NS_LOG_FUNCTION (this);
		instanceFlag = false;
		numClusters = 0;
	}

	void ClusterManager::StartApplication (void)
	{
		NS_LOG_FUNCTION (this);

		if (m_socket == 0)
		{
			TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
			m_socket = Socket::CreateSocket (GetNode (), tid);
		
			/*
			if (sending)
			{
				if (Ipv4Address::IsMatchingType(peerAddress) == true)
				{
					if(broadcast){
						m_socket->SetAllowBroadcast (true);
						m_socket->Connect (InetSocketAddress(Ipv4Address("255.255.255.255")));
					}
					else
					{
						m_socket->Bind();
						m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(peerAddress), peerPort));
					}
				}
				else if (Ipv6Address::IsMatchingType(peerAddress) == true)
				{
					if(broadcast){
						m_socket->SetAllowBroadcast (true);
						m_socket->Connect (InetSocketAddress(Ipv4Address("255.255.255.255")));
					}
					else
					{
						m_socket->Bind6();
						m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(peerAddress), peerPort));
					}
				}
			}
			else
			{
				InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), peerPort);
				m_socket->Bind (local);
			}
		
		}
		m_socket->SetRecvCallback (MakeCallback (&ClusterManager::HandleRead, this));
		* 
		* */
		if(broadcast)
			m_socket->SetAllowBroadcast (true);
		
		if (sending)
			{
				if (Ipv4Address::IsMatchingType(peerAddress) == true)
				{
					m_socket->Bind();
					m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(peerAddress), peerPort));
				}
				else if (Ipv6Address::IsMatchingType(peerAddress) == true)
				{
					m_socket->Bind6();
					m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(peerAddress), peerPort));
				}
			}
			else
			{
				InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), peerPort);
				m_socket->Bind (local);
			}
		
		}
		m_socket->SetRecvCallback (MakeCallback (&ClusterManager::HandleRead, this));
		
		if (sending)
			ScheduleTransmit (Seconds (0.));
	}

	void ClusterManager::StopApplication (void)
	{
		NS_LOG_FUNCTION (this);

		if (m_socket != 0)
		{
			m_socket->Close ();
			m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
			m_socket = 0;
		}

		Simulator::Cancel (sendEvent);

	}

	void ClusterManager::Setup (Ipv4Address address, uint16_t port, DataRate dr, bool toSend, bool broadcastaddr, int packetType)
	{
		peerAddress = address;
		peerPort = port;
		dataRate = dr;
		sending = toSend;
		broadcast = broadcastaddr;
		packet_Type = packetType;
	}

	void ClusterManager::HandleRead (Ptr<Socket> socket)
	{
		
		std::cout << "Yes------------------- ClusterManager" << std::endl;
		/*
		NS_LOG_FUNCTION (this << socket);
		Ptr<Packet> packet;
		Address from;

		

		while (packet = socket->RecvFrom (from))
		{
			if (InetSocketAddress::IsMatchingType (from))
		{
				NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s peer received " << packet->GetSize () << " bytes from " <<
						   InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
						   InetSocketAddress::ConvertFrom (from).GetPort ());
		}
			
			packet->RemoveAllPacketTags ();
		packet->RemoveAllByteTags ();

		NS_LOG_LOGIC ("Echoing packet");
		socket->SendTo (packet, 0, from);      
			if (InetSocketAddress::IsMatchingType (from))
		{
			NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
						   InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
						   InetSocketAddress::ConvertFrom (from).GetPort ());
		}
		}
		* 
		* */
	}

	void ClusterManager::ScheduleTransmit (Time dt)
	{
		NS_LOG_FUNCTION (this << dt);
		sendEvent = Simulator::Schedule (dt, &ClusterManager::Send, this);
	}

	void ClusterManager::Send (void)
	{
		NS_LOG_FUNCTION (this);

		NS_ASSERT (sendEvent.IsExpired ());

		std::cout<<"Inside clustermanager send function"<<std::endl;
		MyTag sendTag;
		sendTag.SetPacketType(packet_Type);
    
		Ptr<Packet> p;
		p = Create<Packet> (dataSize);
		p->AddPacketTag(sendTag);

		// May want to add a trace sink
		m_socket->Send (p);

		NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s peer sent " << dataSize << " bytes to " <<
					   Ipv4Address::ConvertFrom (peerAddress) << " port " << peerPort);

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

}

