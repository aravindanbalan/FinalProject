/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "clustermember.h"
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

namespace ns3 {

/* ... */
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
		
		m_socket->SetRecvCallback (MakeCallback (&ClusterMember::HandleRead, this));
		*/
		
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

	void ClusterMember::Setup (Ipv4Address address, uint16_t port, DataRate dr, bool toSend, bool broadcastaddr, uint32_t nodeid)
	{
		peerAddress = address;
		nodeID = nodeid;
		peerPort = port;
		dataRate = dr;
		sending = toSend;
		broadcast = broadcastaddr;
	}

	void ClusterMember::HandleRead (Ptr<Socket> socket)
	{
		
		
		NS_LOG_FUNCTION (this << socket);
		Ptr<Packet> packet;
		Address from;

		

		while (packet = socket->RecvFrom (from))
		{
		std::cout << "Yes  1111------------------- Cluster Memeber from "<<from << std::endl;	
			
		std::cout<< "At time " << Simulator::Now ().GetSeconds () << "s peer received " << packet->GetSize () << " bytes from " <<
				   InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
				   InetSocketAddress::ConvertFrom (from).GetPort ()<< " recving : "<<nodeID<<std::endl;
	
			
			packet->RemoveAllPacketTags ();
			packet->RemoveAllByteTags ();

		/*
			//NS_LOG_LOGIC ("Echoing packet");
			//socket->SendTo (packet, 0, from);      
			if (InetSocketAddress::IsMatchingType (from))
			{
				NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
							   InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
							   InetSocketAddress::ConvertFrom (from).GetPort ());
			}
		*/
		}

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

		// May want to add a trace sink
		m_socket->Send (p);

		NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s peer sent " << dataSize << " bytes to " <<
					   Ipv4Address::ConvertFrom (peerAddress) << " port " << peerPort);

	}

}

