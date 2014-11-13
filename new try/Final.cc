// ./waf --run "scratch/mobTop5 --nodeNum=50 --traceFile=scratch/sc2.tcl"

#include <stdlib.h> 
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
//#include "ns3/social-network.h"
#include "ns3/clustermanager.h"
#include "ns3/clustermember.h"
#include "ns3/cluster.h"
#include "ns3/flow-monitor-module.h"

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

class Vanet
{
public:
    Vanet ();
    bool Configure (int argc, char *argv[]);
    void Run ();
    std::string logFile;
    std::ofstream myos;
    std::string traceFile;
    void PrintNames ();

private:
    uint32_t nodeNum;
    uint32_t nodeMobileNodes;
    int numRounds;
	int numofTopics;
    double duration;
    bool pcap;
    bool verbose;
    Ptr<ClusterManager> baseStationApp;
    Ptr<ClusterMember> peerRecv;
    vector<int> masterIDs;

    NodeContainer nodes;
    NodeContainer wifiApNode;
    NetDeviceContainer devices;
    Ipv4InterfaceContainer interfaces;
    Ipv4InterfaceContainer interfaceBS;
    NetDeviceContainer apDevices;

private:
    void CreateNodes ();
    void CreateDevices ();
    void InstallInternetStack ();
    void InstallApplications ();
    void ConfigureBaseStations ();
    void FormClusters();
    void SendToMasterOfEachCluster();
};

int randomNumberGenerator(int numTopics)
{
		int v1 = rand() % numTopics; 
		return v1;
}

Vanet::Vanet (): nodeNum (51), numRounds (5), numofTopics (7), duration (300.0), pcap (false),
    verbose (true)
{
	nodeMobileNodes = nodeNum - 1;
}

bool Vanet::Configure (int argc, char *argv[])
{
    CommandLine cmd;

    cmd.AddValue ("nodeNum", "Number of nodes.", nodeNum);
    cmd.AddValue ("duration", "Simulation time in sec.", duration);
    cmd.AddValue ("numRounds", "Number of rounds to perform", numRounds);
    cmd.AddValue ("numofTopics", "Number of topics of interest", numofTopics);
    cmd.AddValue ("traceFile", "NS3 mobility trace.", traceFile);
    cmd.AddValue ("logFile", "Log file", logFile);
    cmd.AddValue ("verbose", "Tell application to log if true", verbose);

	nodeMobileNodes = nodeNum - 1;
    if (verbose)
    {
        LogComponentEnable ("ClusterManager", LOG_LEVEL_INFO);
    }

    cmd.Parse (argc, argv);
    return true;
}

void Vanet::Run ()
{
    CreateNodes ();
    CreateDevices ();
    //ConfigureBaseStations ();
    InstallInternetStack ();
    InstallApplications ();

    std::cout << "Starting simulation for " << duration << " s.\n";

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Simulator::Stop (Seconds (duration));

	AnimationInterface::SetConstantPosition (nodes.Get (50), 7.6, 198.35);
	AnimationInterface anim ("vanet_animation.xml");
	anim.EnablePacketMetadata (true);
    Simulator::Run ();
    
    myos.close (); // close log file
    Simulator::Destroy ();
}

void Vanet::CreateNodes ()
{
    Ns2MobilityHelper mob = Ns2MobilityHelper (traceFile);
    myos.open (logFile.c_str ());
    std::cout << "Creating " << nodeNum << " nodes.\n";
    nodes.Create (nodeNum);
	mob.Install ();
    Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
		   MakeBoundCallback (&CourseChange, &myos));
}

void Vanet::CreateDevices ()
{
    std::string phyMode ("OfdmRate54Mbps");

    // disable fragmentation for frames below 2200 bytes
    Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
    // turn off RTS/CTS for frames below 2200 bytes
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
    // Fix non-unicast data rate to be the same as that of unicast
    Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

    
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    channel.AddPropagationLoss("ns3::RangePropagationLossModel",
                               "MaxRange", DoubleValue(5000.0)); //XXX
   

    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    phy.SetChannel (channel.Create ());

    WifiHelper wifi = WifiHelper::Default ();
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode",StringValue (phyMode),
                                  "ControlMode",StringValue (phyMode));
    
    NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();

    mac.SetType ("ns3::AdhocWifiMac");

    devices = wifi.Install (phy, mac, nodes);

    if (pcap)
    {
        phy.EnablePcapAll (std::string ("VanetSimulation"));
    }
    
}
/*
void Vanet::ConfigureBaseStations ()
{
   std::string phyMode ("OfdmRate54Mbps");

    // disable fragmentation for frames below 2200 bytes
    Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
    // turn off RTS/CTS for frames below 2200 bytes
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
    // Fix non-unicast data rate to be the same as that of unicast
    Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

    
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    channel.AddPropagationLoss("ns3::RangePropagationLossModel",
                               "MaxRange", DoubleValue(5000.0)); //XXX
   

    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    phy.SetChannel (channel.Create ());

    WifiHelper wifi = WifiHelper::Default ();
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode",StringValue (phyMode),
                                  "ControlMode",StringValue (phyMode));
    
    NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();

    mac.SetType ("ns3::AdhocWifiMac");

	wifiApNode.Create (1);
	
	MobilityHelper	mobility;	
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();	
	positionAlloc->Add(Vector(5.0,190.0,0.0));	
	mobility.SetPositionAllocator(positionAlloc);	
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");	
	mobility.Install (wifiApNode); 

	apDevices = wifi.Install (phy, mac, wifiApNode); 
}
*/
void Vanet::InstallInternetStack ()
{
    InternetStackHelper stack;
	
    stack.Install (nodes);
    Ipv4AddressHelper address;
    address.SetBase ("1.0.0.0", "255.0.0.0");

    interfaces = address.Assign (devices);
 
}

void Vanet::InstallApplications ()
{
	
    std::cout<<"BS address"<<interfaces.GetAddress (50)<<std::endl;
    std::cout<<"recv node address"<<interfaces.GetAddress (1)<<std::endl;
    
    FormClusters();
    SendToMasterOfEachCluster();  
   /* 
    
    Ptr<ClusterManager> peerSend = CreateObject<ClusterManager> ();
	peerSend->Setup (interfaces.GetAddress (0), 9, DataRate ("1Mbps"), true);
	nodes.Get (50)->AddApplication (peerSend);
	peerSend->SetStartTime (Seconds (1.));
	peerSend->SetStopTime (Seconds (10.));

	Ptr<ClusterMember> peerRecv = CreateObject<ClusterMember> ();
	peerRecv->Setup (interfaces.GetAddress (50), 9, DataRate ("1Mbps"), false);
	nodes.Get (0)->AddApplication (peerRecv);
	peerRecv->SetStartTime (Seconds (1.));
	peerRecv->SetStopTime (Seconds (10.));
	*/
	
	
}

void Vanet::FormClusters ()
{
	
	int numApps = nodes.Get (50)->GetNApplications();
	if(numApps < 1)
		 baseStationApp = CreateObject<ClusterManager> ();
	
	else 
		 baseStationApp = nodes.Get (50)->GetObject<ClusterManager> ();
		
	for(uint32_t nodeInd = 0; nodeInd < nodeMobileNodes ; nodeInd ++)
	{
		int random_topic_id = randomNumberGenerator(numofTopics); 
		//cout<< random_topic_id << endl;
		//put all nodes in some cluster based on the topic of interest
		//cout<<"In cluster number................ : "<< baseStationApp->getClusterIDFromNode(nodes.Get(nodeInd)) << endl;
		
		if(baseStationApp->getClusterIDFromNode(nodes.Get(nodeInd)) != -999){
			baseStationApp->leave_Cluster(nodes.Get(nodeInd), nodeInd);
		}
			
		baseStationApp->join_Cluster(nodes.Get(nodeInd), nodeInd, random_topic_id);
		cout<<"In cluster number : "<< baseStationApp->getClusterIDFromNode(nodes.Get(nodeInd)) << endl;
		//cout << "Is Master : " << baseStationApp->isMaster(nodes.Get(nodeInd)) << endl;
		//cout<< endl;
	}
	
	cout << "Total number of clusters formed : " << baseStationApp->getNumberOfClusters() << endl;
	
	if(numApps < 1)
		nodes.Get (50)->AddApplication (baseStationApp);
}

void Vanet::SendToMasterOfEachCluster ()
{
	
	int numClusters = baseStationApp->getNumberOfClusters();
	cout << "Inside send master : " << numClusters << endl;
	peerRecv = CreateObject<ClusterMember> ();
	
	for(int clusterID = 0 ; clusterID < numClusters ; clusterID++)
	{
		int masterNodeID = baseStationApp->getMasterNodeIDFromCluster(clusterID);
		cout<<"Cluster ID : "<<clusterID << " , master node : "<<masterNodeID<<endl;
		
		/*
		baseStationApp->Setup (interfaces.GetAddress (masterNodeID), 9, DataRate ("1Mbps"), true);
		baseStationApp->SetStartTime (Seconds (1.0 + 0.0001*clusterID ));
		baseStationApp->SetStopTime (Seconds (10.));
	
		peerRecv->Setup (interfaces.GetAddress (50), 9, DataRate ("1Mbps"), false);
		masterNode->AddApplication (peerRecv);
		peerRecv->SetStartTime (Seconds (1.0 + 0.0001*clusterID));
		peerRecv->SetStopTime (Seconds (5.));
		*/
		
		//form the vector to include in the packet header
		masterIDs.push_back(masterNodeID);
	}
	
	baseStationApp->Setup (Ipv4Address::GetBroadcast (), 9, DataRate ("1Mbps"), true);
	baseStationApp->SetStartTime (Seconds (1. ));
	baseStationApp->SetStopTime (Seconds (10.));

	peerRecv->Setup (Ipv4Address::GetBroadcast (), 9, DataRate ("1Mbps"), false);
	
	for(uint32_t i=0;i<nodeMobileNodes ; i++)
	{
		nodes.Get (i)->AddApplication (peerRecv);
		peerRecv->SetStartTime (Seconds (1.));
		peerRecv->SetStopTime (Seconds (10.));
	}	
}

void Vanet::PrintNames ()
{
    for (uint32_t i=0; i< nodeNum; i++)
        std::cout << Names::FindName (nodes.Get(i)) << std::endl;
}

int main (int argc, char *argv[])
{
    Vanet test;
    if (! test.Configure (argc, argv))
        NS_FATAL_ERROR ("Configuration failed. Aborted.");

    test.Run ();

    return 0;
}
