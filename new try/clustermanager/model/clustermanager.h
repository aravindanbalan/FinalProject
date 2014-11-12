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

namespace ns3 {

/* ... */

class Socket;

class ClusterManager : public Application
{
	public:
		ClusterManager ();
		virtual ~ClusterManager ();
		void Setup (Ipv4Address address, uint16_t port, DataRate dr, bool toSend);

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
		
		DataRate dataRate;
		EventId sendEvent;
};

}

#endif /* CLUSTERMANAGER_H */

