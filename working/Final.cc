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
    bool enableFlowMonitor;
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
    uint32_t total_lost_packets;

private:
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
	enableFlowMonitor = true;
	total_lost_packets = 0;
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
	
	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor;
	if (enableFlowMonitor)
    {
      
		monitor = flowmon.InstallAll(); 
    }

    Simulator::Run ();
    
    myos.close (); // close log file
	if (enableFlowMonitor)
    {
		monitor->CheckForLostPackets ();
		
		Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
      std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
      std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      std::cout << "  Tx Packets: " << i->second.txPackets << std::endl; 
         std::cout << "  Rx Packets: " << i->second.rxPackets << std::endl; 
         std::cout << "  Lost Packets: " << i->second.lostPackets << std::endl; 
         total_lost_packets += i->second.lostPackets;
      
      //std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (duration - clientStart) / 1024 / 1024  << " Mbps\n";
    }

  monitor->SerializeToXmlFile ("vanet_flowmon.xml", true, true);

		
    }
    
    std::cout<<"Total number of lost packets : "<<total_lost_packets<<std::endl;
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
		int round_duration = duration/numRounds;
		int round_start = 1;
		int round_end = round_duration;
		
		int round = 1;
		//for(round = 1; round <= numRounds ; round++ )
		{
			cout<<"Round duration : "<<round_duration<<endl;
			cout<<"Round start : "<<round_start<<endl;
			cout<<"Round end : "<<round_end<<endl;
			if(round > 1)
			{
				round_start += round_duration;
				round_end += round_duration;
			}
		
			performRound(round,round_duration, round_start, round_end);
		}
    
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

void Vanet::Callback (){
	
	cout<<"Inside callback";
}

void Vanet::performRound(int round, int round_duration, int round_start, int round_end)
{
	FormClusters(round, round_duration, round_start, round_end);
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


void Vanet::FormClusters (int round, int round_duration, int round_start, int round_end)
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
				//	baseStationApp->leave_Cluster(nodes.Get(nodeInd), nodeInd);
				}
					
				baseStationApp->join_Cluster(nodes.Get(nodeInd), nodeInd, random_topic_id);
				
		}	
	}
	
	ChooseMaster(baseStationApp, round);
	
	cout << "Total number of clusters formed : " << baseStationApp->getNumberOfClusters() << endl;
	
	if(numApps < 1)
		nodes.Get (50)->AddApplication (baseStationApp);
		
		
	SendToMasterOfEachCluster(round, round_duration , round_start, round_end); 	
}

void Vanet::SendToMasterOfEachCluster (int round , int round_duration, int round_start, int round_end)
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
	baseStationApp->SetStartTime (Seconds (round_start ));
	baseStationApp->SetStopTime (Seconds (round_end));

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
		masters[i]->Setup (Ipv4Address::GetBroadcast (), 9, DataRate ("1Mbps"), false, false, masterIDs[i], slaveListStrings[i], nodes,interfaces, baseStationApp, 0,0, round_start, round_end);
		nodes.Get (masterIDs[i])->AddApplication (masters[i]);
		masters[i]->SetStartTime (Seconds (round_start));
		masters[i]->SetStopTime (Seconds (round_end));		
	}
	
	
	/*
	// new code receiver for all bradcast in port 10
	Ptr<ClusterMember> allNodesBroadcastReceiverApp[nodeMobileNodes];
	for(uint32_t i = 0 ; i< nodeMobileNodes; i++) {
		
		//int masterID = baseStationApp->getMasterNodeIDFromSlaveID(nodes.Get(i));
		//cout<<"Master Node for Node "<< i << " is "<<masterID<<endl;
		//int currentTopic = baseStationApp->getTopicFromNode(nodes.Get (i));
	
			allNodesBroadcastReceiverApp[i] = CreateObject<ClusterMember> ();
			allNodesBroadcastReceiverApp[i]->Setup (Ipv4Address::GetBroadcast (), 10, DataRate ("1Mbps"), false, false, i, "", nodes,interfaces, baseStationApp, 0,0, round_start, round_end);
			nodes.Get (i)->AddApplication (allNodesBroadcastReceiverApp[i]);
			allNodesBroadcastReceiverApp[i]->SetStartTime (Seconds (0.));
			allNodesBroadcastReceiverApp[i]->SetStopTime (Seconds (300.0));	
			
	}
	*/
	
	
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
