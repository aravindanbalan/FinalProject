/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef CLUSTERMEMBER_H
#define CLUSTERMEMBER_H

#include "ns3/application.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/average.h"
#include "ns3/simulator.h"
#include "ns3/clustermanager.h"
#include <map>

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/event-id.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/log.h"

namespace ns3 {

/* ... */

class Socket;
class ClusterManager;

class ClusterMember : public Application
{
	public:
		ClusterMember ();
		virtual ~ClusterMember ();
		void Setup (Ipv4Address address, uint16_t port, DataRate dr, bool toSend, bool broadcastaddr, uint32_t nodeid, std::string slaveString, NodeContainer nodesC, Ipv4InterfaceContainer interf, Ptr<ClusterManager> clusMgr, int pkt_type, int topic, int round_start, int round_end);
		Ptr<Packet> getPacket();

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
		uint32_t nodeID;
		Ptr<Packet> recvpacket;
		std::string slaveStr;
		NodeContainer nodes;
		Ipv4InterfaceContainer interfaces;
		int pac_type;
		int roundStart;
		int roundEnd;
		Ptr<ClusterManager> clusterMgr;
		int topic_of_interest;
		
		DataRate dataRate;
		EventId sendEvent;

};


}

#endif /* CLUSTERMEMBER_H */

