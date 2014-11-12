// ./waf --run "scratch/mobTop5 --nodeNum=50 --traceFile=scratch/sc2.tcl"


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

class Topology
{
public:
    Topology ();
    bool Configure (int argc, char *argv[]);
    void Run ();
    std::string logFile;
    std::ofstream myos;
    std::string traceFile;
    void PrintNames ();

private:
    uint32_t nodeNum;
    double duration;
    bool pcap;
    bool verbose;

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
};

Topology::Topology (): nodeNum (50), duration (300.0), pcap (false),
    verbose (true)
{
}

bool Topology::Configure (int argc, char *argv[])
{
    CommandLine cmd;

    cmd.AddValue ("nodeNum", "Number of nodes.", nodeNum);
    cmd.AddValue ("duration", "Simulation time in sec.", duration);
    cmd.AddValue ("traceFile", "NS3 mobility trace.", traceFile);
    cmd.AddValue ("logFile", "Log file", logFile);
    cmd.AddValue ("verbose", "Tell application to log if true", verbose);

    if (verbose)
    {
        LogComponentEnable ("ClusterManager", LOG_LEVEL_INFO);
    }

    cmd.Parse (argc, argv);
    return true;
}

void Topology::Run ()
{
    CreateNodes ();
    CreateDevices ();
    ConfigureBaseStations ();
    InstallInternetStack ();
    InstallApplications ();

    std::cout << "Starting simulation for " << duration << " s.\n";

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Simulator::Stop (Seconds (duration));

//	AnimationInterface::SetNodeColor (nodes.Get(0), 0, 255, 0);
//	AnimationInterface::SetNodeColor (nodes.Get(8), 0, 0, 255);
	AnimationInterface anim ("vanet_animation.xml");
	anim.EnablePacketMetadata (true);
    Simulator::Run ();
    
    myos.close (); // close log file
    Simulator::Destroy ();
}

// XXX: I am thinking that we can create a sepreate set of nodes for each community
//      and set up a different mobility trace for each community
void Topology::CreateNodes ()
{
    Ns2MobilityHelper mob = Ns2MobilityHelper (traceFile);
    myos.open (logFile.c_str ());
    std::cout << "Creating " << nodeNum << " nodes.\n";
    nodes.Create (nodeNum);
	mob.Install ();
    Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
		   MakeBoundCallback (&CourseChange, &myos));
}

void Topology::CreateDevices ()
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

void Topology::ConfigureBaseStations ()
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

void Topology::InstallInternetStack ()
{
    InternetStackHelper stack;
    // add all stationary base station nodes to this container
	nodes.Add (wifiApNode);
	
    stack.Install (nodes);
    //stack.Install (wifiApNode.Get (0));
    Ipv4AddressHelper address;
    address.SetBase ("1.0.0.0", "255.0.0.0");
    devices.Add(apDevices);
    interfaces = address.Assign (devices);
    //interfaceBS = address.Assign (apDevices);
    //interfaces.Add(interfaceBS);
}

void Topology::InstallApplications ()
{
	/*
    Ptr<ClusterMember> app[nodeNum];
    std::cout<<"Base station address : "<<interfaceBS.GetAddress (0)<<std::endl;
    for (uint32_t i =0; i < nodeNum; i++)
    {   
        app[i] = CreateObject<ClusterMember> ();
        // false - receiver nodes
        
        app[i]->Setup (interfaceBS.GetAddress (0), 9, DataRate ("1Mbps"), false);
        nodes.Get (i)->AddApplication (app[i]);
        //app[i]->SetStartTime (Seconds (0.5 + 0.0001*i));
        app[i]->SetStartTime (Seconds (0.5));
        app[i]->SetStopTime (Seconds (300.));
    }
    
    Ptr<ClusterManager> BSApp;
    BSApp = CreateObject<ClusterManager> ();
    BSApp->Setup (interfaces.GetAddress (25), 9, DataRate ("1Mbps"), true);
    std::cout<<"BS sending to address : "<<interfaces.GetAddress (25)<<std::endl;
    wifiApNode.Get (0)->AddApplication (BSApp);
    BSApp->SetStartTime (Seconds (0.7));
    BSApp->SetStopTime (Seconds (10.));
    * 
    * */
    
    std::cout<<"BS address"<<interfaces.GetAddress (50)<<std::endl;
    std::cout<<"recv node address"<<interfaces.GetAddress (1)<<std::endl;
    
    
    
    // Below code works as 0 -49 are mobile nodes and mobile nodes are able to communicate with each other.
    
    Ptr<ClusterManager> peerSend = CreateObject<ClusterManager> ();
	peerSend->Setup (interfaces.GetAddress (0), 9, DataRate ("1Mbps"), true);
	nodes.Get (49)->AddApplication (peerSend);
	peerSend->SetStartTime (Seconds (1.));
	peerSend->SetStopTime (Seconds (10.));

	Ptr<ClusterMember> peerRecv = CreateObject<ClusterMember> ();
	peerRecv->Setup (interfaces.GetAddress (49), 9, DataRate ("1Mbps"), false);
	nodes.Get (0)->AddApplication (peerRecv);
	peerRecv->SetStartTime (Seconds (1.));
	peerRecv->SetStopTime (Seconds (10.));
	
	/* **********************************************************************
	// Below code doesnt works as 0 is not reachable from 50(NS) are mobile nodes and mobile nodes are able to communicate with each other.
    
    Ptr<ClusterManager> peerSend = CreateObject<ClusterManager> ();
	peerSend->Setup (interfaces.GetAddress (0), 9, DataRate ("1Mbps"), true);
	nodes.Get (49)->AddApplication (peerSend);
	peerSend->SetStartTime (Seconds (1.));
	peerSend->SetStopTime (Seconds (10.));

	Ptr<ClusterMember> peerRecv = CreateObject<ClusterMember> ();
	peerRecv->Setup (interfaces.GetAddress (49), 9, DataRate ("1Mbps"), false);
	nodes.Get (0)->AddApplication (peerRecv);
	peerRecv->SetStartTime (Seconds (1.));
	peerRecv->SetStopTime (Seconds (10.));
	
	
	*/ 
}

void Topology::PrintNames ()
{
    for (uint32_t i=0; i< nodeNum; i++)
        std::cout << Names::FindName (nodes.Get(i)) << std::endl;
}

int main (int argc, char *argv[])
{
    Topology test;
    if (! test.Configure (argc, argv))
        NS_FATAL_ERROR ("Configuration failed. Aborted.");

    test.Run ();

    return 0;
}
