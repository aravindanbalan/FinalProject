#include <stdlib.h>

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
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
#include "ns3/netanim-module.h"
#include "MyTag.h"

#include "clustermanager.h"

using namespace ns3;
using namespace std;

uint32_t size = 1024;
 std::string logFile;
 std::ofstream myos;
 std::string traceFile;
 InternetStackHelper stack;
 Ipv4AddressHelper address;

 double round_start;
 double round_end;
 uint32_t nodeNum;
 uint32_t nodeMobileNodes;
 int packet_type;
 int numRounds;
 int numofTopics;
 double duration;
 bool pcap;
 bool enableFlowMonitor;
 bool verbose;
 ClusterManager* clusterMgr;
 vector<int> masterIDs;
 vector<string> slaveListStrings;

 NodeContainer nodes;
 NodeContainer wifiApNode;
 NetDeviceContainer devices;
 Ipv4InterfaceContainer interfaces;
 Ipv4InterfaceContainer interfaceBS;
 NetDeviceContainer apDevices;
 uint32_t total_lost_packets;
 double round_duration;
 
 static int currentRound = 0;
 static int number_slaves = 0;
 int number_masters = 0;
 
static void
CourseChange (std::ostream *myos, std::string foo, Ptr<const MobilityModel> mobility)
{
  Ptr<Node> node = mobility->GetObject<Node> ();
  Vector pos = mobility->GetPosition (); // Get position
  Vector vel = mobility->GetVelocity (); // Get velocity
  
  std::cout.precision(5);
  *myos << Simulator::Now () << "; NODE: " << node->GetId() << "; POS: x=" << pos.x << ", y=" << pos.y
	<< ", z=" << pos.z << "; VEL: x=" << vel.x << ", y=" << vel.y
	<< ", z=" << vel.z << std::endl;
}


int readSourceAddressPacketTag(Ptr<Packet> packet)
{
	MyTag recTag;
	packet->PeekPacketTag(recTag);
	int tagVal =int(recTag.GetSourceAddress());
	std::ostringstream s;
	s<<tagVal;
	std::string ss(s.str());
	int source_addr = atoi(ss.c_str());
	return source_addr;
}
	
int readTopicFromPacket(Ptr<Packet> packet)
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

