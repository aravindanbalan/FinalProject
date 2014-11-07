
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/netanim-module.h"
#include "ClusterManager.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/aodv-helper.h"

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhocGrid");

// Prints actual position and velocity when a course change event occurs
/*
static void
CourseChange (std::ostream *os, std::string foo, Ptr<const MobilityModel> mobility)
{
  Vector pos = mobility->GetPosition (); // Get position
  Vector vel = mobility->GetVelocity (); // Get velocity
  Ptr<Node> node = mobility->GetObject<Node> ();

  // Prints position and velocities
  *os << Simulator::Now () << " POS: x=" << pos.x << ", y=" << pos.y
      << ", z=" << pos.z << "; VEL:" << vel.x << ", y=" << vel.y
      << ", z=" << vel.z << " Node : " << node->GetId () << std::endl;
}
*/
int randomNumberGenerator(int numTopics)
{
		//srand (time(NULL));
		int v1 = rand() % numTopics; 
		return v1;
}
// Example to use ns2 traces file in ns3
int main (int argc, char *argv[])
{
	 //NS_LOG_UNCOND("Inside Main");
	std::string traceFile;
	std::string logFile;

	bool verbose = false;
	bool tracing = true;
	TypeId tid;
	Ipv4InterfaceContainer i;
	NodeContainer c;
	int    nodeNum;
	double duration;
	//int numRounds = 4;
	int numofTopics = 7;
	ClusterManager *clusterMgr = ClusterManager::getInstance();	
	
	
	
	std::string phyMode ("OfdmRate54Mbps");

	// Enable logging from the ns2 helper
	//LogComponentEnable ("Ns2MobilityHelper",LOG_LEVEL_DEBUG);

	// Parse command line attribute
	
	CommandLine cmd;
	cmd.AddValue ("traceFile", "Ns2 movement trace file", traceFile);
	cmd.AddValue ("nodeNum", "Number of nodes", nodeNum);
	cmd.AddValue ("duration", "Duration of Simulation", duration);
	cmd.AddValue ("logFile", "Log file", logFile);
	cmd.Parse (argc,argv);

	// Check command line arguments
	if (traceFile.empty () || nodeNum <= 0 || duration <= 0 || logFile.empty ())
	{
	  std::cout << "Usage of " << argv[0] << " :\n\n"
	  "./waf --run \"ns2-mobility-trace"
	  " --traceFile=src/mobility/examples/default.ns_movements"
	  " --nodeNum=2 --duration=100.0 --logFile=ns2-mob.log\" \n\n"
	  "NOTE: ns2-traces-file could be an absolute or relative path. You could use the file default.ns_movements\n"
	  "      included in the same directory of this example file.\n\n"
	  "NOTE 2: Number of nodes present in the trace file must match with the command line argument and must\n"
	  "        be a positive number. Note that you must know it before to be able to load it.\n\n"
	  "NOTE 3: Duration must be a positive number. Note that you must know it before to be able to load it.\n\n";

	  return 0;
	}


	// disable fragmentation for frames below 2200 bytes
	Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
	// turn off RTS/CTS for frames below 2200 bytes
	Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
	// Fix non-unicast data rate to be the same as that of unicast
	Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
						StringValue (phyMode));

	c.Create (nodeNum);

	for(int nodeind = 0; nodeind < nodeNum; nodeind++)
    {
		int random_topic_id = randomNumberGenerator(numofTopics); 
		//cout<< random_topic_id << endl;
		//put all nodes in some cluster based on the topic of interest
		clusterMgr->join_Cluster(c.Get(nodeind), random_topic_id);
		
		cout<<"In cluster number : "<< clusterMgr->getClusterIDFromNode(c.Get(nodeind)) << endl;
		cout << "Is Master : " << clusterMgr->isMaster(c.Get(nodeind)) << endl;
		cout<< endl;
	}	

	cout << "Total number of clusters formed : " << clusterMgr->getNumberOfClusters() << endl;
	WifiHelper wifi;
	if (verbose)
	{
		wifi.EnableLogComponents ();  // Turn on all Wifi logging
	}

	YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
	// set it to zero; otherwise, gain will be added
	wifiPhy.Set ("RxGain", DoubleValue (0) );
	// ns-3 supports RadioTap and Prism tracing extensions for 802.11b
	wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
	wifiPhy.SetChannel (wifiChannel.Create ());

	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
	wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
								  "DataMode",StringValue (phyMode),
								  "ControlMode",StringValue (phyMode));

	// Set it to adhoc mode
	wifiMac.SetType ("ns3::AdhocWifiMac");
	NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);


	// Create Ns2MobilityHelper with the specified trace log file as parameter
	Ns2MobilityHelper mobility = Ns2MobilityHelper (traceFile);

	// open log file for output
	//std::ofstream os;
	//os.open (logFile.c_str ());

	// Create all nodes.

	mobility.Install (); // configure movements for each node, while reading trace file

	Ipv4StaticRoutingHelper staticRouting;
	Ipv4ListRoutingHelper list;
	list.Add (staticRouting, 0);

	InternetStackHelper internet;
	internet.SetRoutingHelper (list); // has effect on the next Install ()
	internet.Install (c);

	Ipv4AddressHelper ipv4;

	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	i = ipv4.Assign (devices);

	tid = TypeId::LookupByName ("ns3::UdpSocketFactory");


	// Configure callback for logging
	//Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
				   //MakeBoundCallback (&CourseChange, &os));


	Simulator::Stop (Seconds (duration));
	AnimationInterface anim ("animation.xml");

	if (tracing == true)
	{
		AsciiTraceHelper ascii;
		wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("wifi-simple-adhoc-grid.tr"));
		wifiPhy.EnablePcap ("wifi-simple-adhoc-grid", devices);
		// Trace routing tables
		Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("wifi-simple-adhoc-grid.routes", std::ios::out);


		// To do-- enable an IP-level trace that shows forwarding events only
	}
	
	Simulator::Run ();
	Simulator::Destroy ();

	//os.close (); // close log file
	return 0;
}
