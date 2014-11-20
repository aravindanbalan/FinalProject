/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "clustermember.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/icmpv4.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/inet-socket-address.h"
#include "ns3/clustermanager.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "MyTag.h"

namespace ns3 {

/* ... */
using namespace std;
	NS_LOG_COMPONENT_DEFINE ("ClusterMember");

	NS_OBJECT_ENSURE_REGISTERED (ClusterMember);


	ClusterMember::ClusterMember ()
	{
		NS_LOG_FUNCTION (this);
		m_socket = 0;
		sendEvent = EventId ();
		sending = false;
		dataSize = 256;
		m_sent = 0;
		m_received = 0;
	}

	ClusterMember::~ClusterMember ()
	{
		NS_LOG_FUNCTION (this);
	}

	void ClusterMember::StartApplication (void)
	{
		NS_LOG_FUNCTION (this);

		if (m_socket == 0)
		{
			TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
			m_socket = Socket::CreateSocket (GetNode (), tid);
		
			
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
		m_socket->SetRecvCallback (MakeCallback (&ClusterMember::HandleRead, this));
		
		if (sending)
			ScheduleTransmit (Seconds (0.));
	}

	void ClusterMember::StopApplication (void)
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

	void ClusterMember::Setup (Ipv4Address address, uint16_t port, DataRate dr, bool toSend, bool broadcastaddr, uint32_t nodeid, string slaveString,NodeContainer nodesC, Ipv4InterfaceContainer interf,Ptr<ClusterManager> clusMgr,int pkt_type,int topic, int round_start, int round_end)
	{
		peerAddress = address;
		clusterMgr = clusMgr;
		nodeID = nodeid;
		peerPort = port;
		dataRate = dr;
		sending = toSend;
		broadcast = broadcastaddr;
		pac_type = pkt_type;
		slaveStr = slaveString;
		nodes = nodesC;
		interfaces = interf;
		roundStart = round_start;
		roundEnd = round_end;
		topic_of_interest = topic;
		//ClusterManager::sampleTest();
	}

	int readPacketTag(Ptr<Packet> packet)
	{
			MyTag recTag;
			packet->PeekPacketTag(recTag);
			int tagVal =int(recTag.GetPacketType());
			std::ostringstream s;
			s<<tagVal;
			std::string ss(s.str());
			int pkt_Type = atoi(ss.c_str());
			return pkt_Type;
	}
	
	int readTopicTagFromPacket(Ptr<Packet> packet)
	{
			MyTag recTag;
			packet->PeekPacketTag(recTag);
			int tagVal =int(recTag.GetTopic());
			std::ostringstream s;
			s<<tagVal;
			std::string ss(s.str());
			int topic = atoi(ss.c_str());
			return topic;
	}

	vector<std::string> getTokens(string str)
	{
		vector<std::string> tokens;
		std::string delimiter = ",";
		size_t pos = 0;
		std::string token;
		while ((pos = str.find(delimiter)) != std::string::npos) {
			token = str.substr(0, pos);
			tokens.push_back(token);
			str.erase(0, pos + delimiter.length());
		}
		tokens.push_back(str);
	
		return tokens;
	}

	void ClusterMember::HandleRead (Ptr<Socket> socket)
	{		
		NS_LOG_FUNCTION (this << socket);
		Ptr<Packet> packet;
		Address from;
		
		while (packet = socket->RecvFrom (from))
		{
			//std::cout << "Yes  1111------------------- Cluster Memeber from "<<from << std::endl;	
				
			//std::cout<< "At time " << Simulator::Now ().GetSeconds () << "s peer received " << packet->GetSize () << " bytes from " <<
			//		   InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
			//		   InetSocketAddress::ConvertFrom (from).GetPort ()<< " recving : "<<nodeID<<std::endl;
		
			uint8_t *buffer = new uint8_t[packet->GetSize()];
			packet->CopyData(buffer,packet->GetSize());

			int pkt_Type = readPacketTag(packet);
			recvpacket = packet;
			
			//std::cout<<"..........Member received packet type : "<< pkt_Type << std::endl;
			
			if(pkt_Type == 1)
			{
				
				///////////////////////////// check whether current node is master or not and only then handle this packet type
				
				//std::cout<<"********************* "<<clusterMgr->isMaster(GetNode())<<std::endl;
				//std::cout<<std::endl;
				if(clusterMgr->isMaster(GetNode()))
				{
					clusterMgr->numofMastersrecv++;
					if(clusterMgr->numofMastersrecv == clusterMgr->getNumberOfClusters())
						clusterMgr->setDone(true);
						
					uint16_t currentTopic = clusterMgr->getTopicFromNode(GetNode());
					//std::cout<<"Current topic for master node :"<<nodeID<<" is "<<currentTopic<<std::endl;
					//std::cout<<"..........Slave string for this master node : "<< slaveStr << std::endl;
					vector<std::string> slaves = getTokens(slaveStr);
					//std::cout<<"Slave size : "<<slaves.size()<<std::endl;
					
					
					int packet_type = 2;
					//std::cout<<"11111"<<std::endl;
					
					if(clusterMgr->getNumberOfClusters() == 1)
					{
						std::cout<<"Num of cluster is 1"<<std::endl;
						Ptr<ClusterMember> masterApp = CreateObject<ClusterMember> ();
						masterApp->Setup (Ipv4Address::GetBroadcast (), 10, DataRate ("1Mbps"), true, true,nodeID,slaveStr, nodes,interfaces,clusterMgr,packet_type, currentTopic, roundStart, roundEnd );
						nodes.Get(nodeID)->AddApplication (masterApp);
						masterApp->SetStartTime (Seconds (roundStart ));
						masterApp->SetStopTime (Seconds (roundEnd));
						
						Ptr<ClusterMember> peers[slaves.size()];
						for(std::vector<int>::size_type i = 0; i != slaves.size(); i++) {
							
							std::string slave = slaves[i];
							int slaveID = atoi(slave.c_str());
							
							peers[i] = CreateObject<ClusterMember> ();
							int packet_type = 2;
							peers[i]->Setup (Ipv4Address::GetBroadcast (), peerPort, DataRate ("1Mbps"), false, false, slaveID, slaveStr, nodes,interfaces,clusterMgr,packet_type,currentTopic, roundStart, roundEnd);
							nodes.Get(slaveID)->AddApplication (peers[i]);
							peers[i]->SetStartTime (Seconds (roundStart ));
							peers[i]->SetStopTime (Seconds (roundEnd));

						}
					
	
					}
					//std::cout<<"Sending broadcast for Current topic : "<< currentTopic<<std::endl;
		
					else
					{
					
						Ptr<ClusterMember> masterApp[slaves.size()];			
						Ptr<ClusterMember> peers[slaves.size()];
						for(std::vector<int>::size_type i = 0; i != slaves.size(); i++) {
							
							std::string slave = slaves[i];
							int slaveID = atoi(slave.c_str());
							
							masterApp[i] = CreateObject<ClusterMember> ();
							masterApp[i]->Setup (interfaces.GetAddress (slaveID), peerPort, DataRate ("1Mbps"), true, false,nodeID,slaveStr, nodes,interfaces,clusterMgr,packet_type,currentTopic,roundStart, roundEnd );
							nodes.Get(nodeID)->AddApplication (masterApp[i]);
						
							masterApp[i]->SetStartTime (Seconds (roundStart ));
							masterApp[i]->SetStopTime (Seconds (roundEnd));
							
							peers[i] = CreateObject<ClusterMember> ();
							int packet_type = 2;
							peers[i]->Setup (interfaces.GetAddress (nodeID), peerPort, DataRate ("1Mbps"), false, false, slaveID, slaveStr, nodes,interfaces,clusterMgr,packet_type,currentTopic, roundStart, roundEnd);
							nodes.Get(slaveID)->AddApplication (peers[i]);
							peers[i]->SetStartTime (Seconds (roundStart ));
							peers[i]->SetStopTime (Seconds (roundEnd));
							
						//	std::cout<<"..........Slave  : "<< slaveID << std::endl;
						 
						}
					
					}
					
					
					
				}
				
			}
			else if(pkt_Type == 2){
				
				int top = readTopicTagFromPacket(packet);
				//std::cout<<"Top :"<<top<<std::endl;
				Ptr<Node> recvnode = GetNode();
				
				//std::cout<<"*******111  "<<nodeID<<std::endl;
				int interested_topic = clusterMgr->getTopicFromNode(recvnode);
				//std::cout<<"*******222   "<<interested_topic<<std::endl;
				
				if(interested_topic == top)
				{
					std::cout<<"..........Slave recev node  : "<< nodeID << " topic : "<<top<< std::endl;
					//std::cout<<"..........Slave recev node  : "<< slaveStr << std::endl;
				}
				
			}
		}

	}

	Ptr<Packet> ClusterMember::getPacket(){
		return recvpacket;
	}

	void ClusterMember::ScheduleTransmit (Time dt)
	{
		NS_LOG_FUNCTION (this << dt);
		sendEvent = Simulator::Schedule (dt, &ClusterMember::Send, this);
	}

	void ClusterMember::Send (void)
	{
		NS_LOG_FUNCTION (this);

		NS_ASSERT (sendEvent.IsExpired ());
		
		Ptr<Packet> p;
		p = Create<Packet> (dataSize);
		
		if(pac_type == 2)
		{
			MyTag sendTag;
			sendTag.SetPacketType(pac_type);
			
			sendTag.SetTopic(topic_of_interest);
			p->AddPacketTag(sendTag);
    
		}

		// May want to add a trace sink
		m_socket->Send (p);

		NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s peer sent " << dataSize << " bytes to " <<
					   Ipv4Address::ConvertFrom (peerAddress) << " port " << peerPort);

	}

}

