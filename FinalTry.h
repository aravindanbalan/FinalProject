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
#include <cstdlib>
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

 int numMasters;
 double round_start;
 double round_end;
 uint32_t nodeNum;
 uint32_t nodeMobileNodes;
 int packet_type;
 int numRounds;
 int numofTopics;
 double duration;
 double maxRange;
 bool pcap;
 bool enableFlowMonitor;
 bool verbose;
 int check;
 double distanceThreshold;
 ClusterManager* clusterMgr;
 vector<int> masterIDs;
 vector<string> slaveListStrings;
 static double packets_recv_master;
 static int packets_recv_slaves;
 static int packets_recv_decoded;

int self = 0;

 NodeContainer nodes;
 NodeContainer wifiApNode;
 NetDeviceContainer devices;
 Ipv4InterfaceContainer interfaces;
 Ipv4InterfaceContainer interfaceBS;
 NetDeviceContainer apDevices;
 uint32_t total_lost_packets;
 double round_duration;
 int isSelfish;
 double selfishprob;
 static int num_selfish_masters;
 
 int currentRound = 0;
 static int number_slaves = 0;
 int number_masters = 0;
 int cluster_sizes[100] = {0};
 std::map<int, bool> clusterSentMap;
 
static void
CourseChange (std::ostream *myos, std::string foo, Ptr<const MobilityModel> mobility)
{
  Ptr<Node> node = mobility->GetObject<Node> ();
  Vector pos = mobility->GetPosition (); // Get position
  Vector vel = mobility->GetVelocity (); // Get velocity
  
  //constantly update the map for current location
  clusterMgr->setCurrentNodeLocation(node,pos);
  
  std::cout.precision(5);
  
  *myos << Simulator::Now () << "; NODE: " << node->GetId() << "; POS: x=" << pos.x << ", y=" << pos.y
	<< ", z=" << pos.z << "; VEL: x=" << vel.x << ", y=" << vel.y
	<< ", z=" << vel.z << std::endl;
}

int readRoundPacketTag(Ptr<Packet> packet)
{
	MyTag recTag;
	packet->PeekPacketTag(recTag);
	int tagVal =int(recTag.GetRound());
	std::ostringstream s;
	s<<tagVal;
	std::string ss(s.str());
	int round = atoi(ss.c_str());
	return round;
}

int readSourceAddressPacketTag(Ptr<Packet> packet)
{
	MyTag recTag;
	packet->PeekPacketTag(recTag);
	int tagVal =int(recTag.GetSourceAddress());
	//cout<<"Tag value : "<<tagVal<<endl;
	std::ostringstream s;
	s<<tagVal;
	std::string ss(s.str());
	int source_addr = atoi(ss.c_str());
	return source_addr;
}

int readSimpleValuePacketTag(Ptr<Packet> packet)
{
	MyTag recTag;
	packet->PeekPacketTag(recTag);
	int tagVal =int(recTag.GetSimpleValue());
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

int randomBitGeneratorWithProb(double p)
{
    double rndDouble = (double)rand() / RAND_MAX;
    return rndDouble < p;
}

void setClusterSentMap(int clusterID,bool value){
	
	map<int,bool>::iterator p;
		p = clusterSentMap.find(clusterID);
		if(p != clusterSentMap.end())
			clusterSentMap[clusterID] = value;
		else
			clusterSentMap.insert(pair<int,bool>(clusterID,value));
}
bool getClusterSentValueFromMap(int clusterID){
	
	map<int,bool>::iterator p;
		p = clusterSentMap.find(clusterID);
		return p->second;
}

