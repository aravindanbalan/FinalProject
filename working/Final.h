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
#include "ns3/clustermanager.h"
#include "ns3/clustermember.h"
#include "ns3/cluster.h"

//XXX For anumation
#include "ns3/netanim-module.h"

using namespace std;

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Nhap9");

// Applying the mobility model
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


    bool Configure (int argc, char *argv[]);
    void Run ();
    std::string logFile;
    std::ofstream myos;
    std::string traceFile;
    void PrintNames ();

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
    Ptr<ClusterManager> baseStationApp[30];
    vector<int> masterIDs;
    vector<string> slaveListStrings;

    NodeContainer nodes;
    NodeContainer wifiApNode;
    NetDeviceContainer devices;
    Ipv4InterfaceContainer interfaces;
    Ipv4InterfaceContainer interfaceBS;
    NetDeviceContainer apDevices;
    uint32_t total_lost_packets;
    int round_duration;

    void CreateNodes ();
    void Callback ();
    void CreateDevices ();
    void InstallInternetStack ();
    void InstallApplications ();
    void ConfigureBaseStations ();
    void performRound(int round,int round_duration, int round_start, int round_end);
    void FormClusters(int round,int round_duration, int round_start, int round_end);
    void SendToMasterOfEachCluster(int round,int round_duration, int round_start, int round_end);
    void ChooseMaster(Ptr<ClusterManager> baseStationApp, int round);


int randomNumberGenerator(int numTopics)
{
		int v1 = rand() % numTopics; 
		return v1;
}