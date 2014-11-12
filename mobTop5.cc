// ./waf --run "scratch/mobTop5 --nodeNum=50 --traceFile=scratch/sc2.tcl"


#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/social-network.h"

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
    NetDeviceContainer devices;
    Ipv4InterfaceContainer interfaces;

private:
    void CreateNodes ();
    void CreateDevices ();
    void InstallInternetStack ();
    void InstallApplications ();
};

Topology::Topology (): nodeNum (3), duration (100.0), pcap (false),
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
        LogComponentEnable ("SocialNetworkApplication", LOG_LEVEL_INFO);
    }

    cmd.Parse (argc, argv);
    return true;
}

void Topology::Run ()
{
    CreateNodes ();
    CreateDevices ();
    InstallInternetStack ();
    InstallApplications ();

    std::cout << "Starting simulation for " << duration << " s.\n";

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Simulator::Stop (Seconds (duration));

//	AnimationInterface::SetNodeColor (nodes.Get(0), 0, 255, 0);
//	AnimationInterface::SetNodeColor (nodes.Get(8), 0, 0, 255);
	AnimationInterface anim ("mobTop5_t.xml");
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

    // Name nodes
    for (uint32_t i = 0; i < nodeNum/2; i++)
    {
        std::ostringstream os1, os2;
        os1 << "Comm1-" << i;
        Names::Add (os1.str (), nodes.Get (i));
        os2 << "Comm2-" << i;
        Names::Add (os2.str (), nodes.Get (nodeNum/2 + i));
    }

    mob.Install ();
    Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
		   MakeBoundCallback (&CourseChange, &myos));
}

void Topology::CreateDevices ()
{
    std::string phyMode ("DsssRate1Mbps");

    // disable fragmentation for frames below 2200 bytes
    Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
    // turn off RTS/CTS for frames below 2200 bytes
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
    // Fix non-unicast data rate to be the same as that of unicast
    Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

    
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    channel.AddPropagationLoss("ns3::RangePropagationLossModel",
                               "MaxRange", DoubleValue(50.0)); //XXX
   

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
        phy.EnablePcapAll (std::string ("SocialTie"));
    }

    /*/ Take this out _____________________________________//
     MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue (0.0),
                                   "MinY", DoubleValue (0.0),
                                   "DeltaX", DoubleValue (5.0),
                                   "DeltaY", DoubleValue (10.0),
                                   "GridWidth", UintegerValue (3),
                                   "LayoutType", StringValue ("RowFirst"));

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (nodes);

    //_____________________________________________________*/
}

void Topology::InstallInternetStack ()
{
    InternetStackHelper stack;
    stack.Install (nodes);
    Ipv4AddressHelper address;
    address.SetBase ("1.0.0.0", "255.0.0.0");
    interfaces = address.Assign (devices);
}

void Topology::InstallApplications ()
{
    Ptr<SocialNetwork> app[nodeNum];
    for (uint32_t i =0; i < nodeNum; i++)
    {   
        app[i] = CreateObject<SocialNetwork> ();
        app[i]->Setup (9);
        nodes.Get (i)->AddApplication (app[i]);
        app[i]->SetStartTime (Seconds (0.5 + 0.0001*i));
        app[i]->SetStopTime (Seconds (500.));

        if (i < nodeNum/2)
             app[i]->m_communityId = 1;
        else
            app[i]-> m_communityId = 2;
    }

    app[0]->RequestContent(Ipv4Address("1.0.0.9"));
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
