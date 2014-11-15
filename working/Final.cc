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
    int packet_type;
    int numRounds;
	int numofTopics;
    double duration;
    bool pcap;
    bool verbose;
    Ptr<ClusterManager> baseStationApp;
    vector<int> masterIDs;
    vector<string> slaveListStrings;

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
    void performRound(int round);
    void FormClusters(int round);
    void SendToMasterOfEachCluster(int round);
    void DistributePacketFromMasterToPeers();
    void ChooseMaster(Ptr<ClusterManager> baseStationApp, int round);
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

	// do this for many rounds
		performRound(1);
    
    
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

void Vanet::performRound(int round)
{
	FormClusters(round);
}
void Vanet::ChooseMaster(Ptr<ClusterManager> baseStationApp, int round)
{
	cout<<"Choosing master for round : "<<round<<endl;
	
	int numClusters = baseStationApp->getNumberOfClusters();
	for(int clusterID = 0 ; clusterID < numClusters ; clusterID++)
	{
		baseStationApp->choose_Master(clusterID);
		int masterNodeID = baseStationApp->getMasterNodeIDFromCluster(clusterID);
		cout<<"Master in cluster : "<<clusterID << " is : "<<masterNodeID<<endl;
	}
	
}


void Vanet::FormClusters (int round)
{
	
	int numApps = nodes.Get (50)->GetNApplications();
	if(numApps < 1)
		 baseStationApp = CreateObject<ClusterManager> ();
	
	else 
		 baseStationApp = nodes.Get (50)->GetObject<ClusterManager> ();
	
	if(round == 1)	// choose random topic only for first round, for other rounds choose a different the master
	{	 	 	
		for(uint32_t nodeInd = 0; nodeInd < nodeMobileNodes ; nodeInd ++)
		{

				int random_topic_id = randomNumberGenerator(numofTopics); 
				
				if(baseStationApp->getClusterIDFromNode(nodes.Get(nodeInd)) != -999){
					baseStationApp->leave_Cluster(nodes.Get(nodeInd), nodeInd);
				}
					
				baseStationApp->join_Cluster(nodes.Get(nodeInd), nodeInd, random_topic_id);
				
		}	
	}
	
	ChooseMaster(baseStationApp, round);
	
	cout << "Total number of clusters formed : " << baseStationApp->getNumberOfClusters() << endl;
	
	if(numApps < 1)
		nodes.Get (50)->AddApplication (baseStationApp);
		
		
	SendToMasterOfEachCluster(round); 	
}

void Vanet::SendToMasterOfEachCluster (int round)
{
	
	int numClusters = baseStationApp->getNumberOfClusters();
	cout << "Inside send master : " << numClusters << endl;
	
	
	for(int clusterID = 0 ; clusterID < numClusters ; clusterID++)
	{
		int masterNodeID = baseStationApp->getMasterNodeIDFromCluster(clusterID);
		string slaveString = baseStationApp->getSlaveNodeIDsFromCluster(clusterID);
		cout<<"Cluster ID : "<<clusterID << " , master node : "<<masterNodeID<<endl;
		cout<<"Cluster ID : "<<clusterID << " , Slave list : "<<slaveString<<endl;
		//form the vector to include in the packet header
		masterIDs.push_back(masterNodeID);
		slaveListStrings.push_back(slaveString);
	}
	int numMasters = numClusters;
	Ptr<ClusterMember> masters[numMasters];
	
	cout<<"Sending broadcast to all masters"<<endl;
	
	packet_type = 1;
	
	baseStationApp->Setup (Ipv4Address::GetBroadcast (), 9, DataRate ("1Mbps"), true, true, packet_type);
	baseStationApp->SetStartTime (Seconds (1. ));
	baseStationApp->SetStopTime (Seconds (30.));

	/*
	for(uint32_t i=0;i<nodeMobileNodes ; i++)
	{
		peerRecv[i] = CreateObject<ClusterMember> ();
		peerRecv[i]->Setup (Ipv4Address::GetBroadcast (), 9, DataRate ("1Mbps"), false, false,i);
		nodes.Get (i)->AddApplication (peerRecv[i]);
		peerRecv[i]->SetStartTime (Seconds (1.));
		peerRecv[i]->SetStopTime (Seconds (10.));		
	}
	*/	
	
	for(std::vector<int>::size_type i = 0; i != masterIDs.size(); i++) {
		masters[i] = CreateObject<ClusterMember> ();
		masters[i]->Setup (Ipv4Address::GetBroadcast (), 9, DataRate ("1Mbps"), false, false, masterIDs[i], slaveListStrings[i], nodes,interfaces, 0);
		nodes.Get (masterIDs[i])->AddApplication (masters[i]);
		masters[i]->SetStartTime (Seconds (1.));
		masters[i]->SetStopTime (Seconds (30.));		
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
