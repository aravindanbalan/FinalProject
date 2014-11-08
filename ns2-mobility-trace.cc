
#include "ApplicationUtil.h"

NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhocGrid");

//////////////////////// Send and Recieve methods /////////////////////////////////////////

static void SendTopicRequest (Ptr<Socket> socket, int SourceNodeIndex)
{
	uint32_t size = 1024;
    Ptr<Packet> sendPacket = Create<Packet> (size);

    MyTag sendTag;
    sendTag.SetSimpleValue(SourceNodeIndex);
    sendPacket->AddPacketTag(sendTag);

    socket->Send(sendPacket);
    socket->Close();
}

static void ReceiveTopicRequest (Ptr<Socket> socket)
{
 
	//Ptr<Node> recvnode = socket->GetNode();
    //int recNodeIndex = ApplicationUtil::getInstance()->getNodeFromMap(recvnode);

    Ptr<Packet> recPacket = socket->Recv();

    uint8_t *buffer = new uint8_t[recPacket->GetSize()];
    recPacket->CopyData(buffer,recPacket->GetSize());

    MyTag recTag;
    recPacket->PeekPacketTag(recTag);
    int tagVal =int(recTag.GetSimpleValue());
    std::ostringstream s;
    s<<tagVal;
    std::string ss(s.str());
    int srcNodeIndex = atoi(ss.c_str());
    
	std::cout<<"Base station received from : "<< srcNodeIndex <<endl;
   
    //publicKeyCounter--;

	/*
    if(publicKeyCounter == 0)
    {
        Simulator::ScheduleNow (&SimulatorLoop, socket,tid,c,i);
    }
    */
}

//////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////// simulator methods //////////////////////////////////////////

//step1 - each node requests for the random topic assigned for current round
// sends a request packet

void requestForTopic(){
	
	cout<<"request..."<<endl;	

	// send request packet for each of the nodes in the system
	for(int nodeind = 0; nodeind < nodeNum; nodeind++)
    {
		Ptr<Node> sourceNode = vehicles.Get (nodeind);
		Ptr<Node> recvNode = clusterMgr->getClusterMgrNode();
		int baseStationIndex = 0;
		
		Ptr<Socket> recvNodeSink = Socket::CreateSocket (recvNode, tid);
		InetSocketAddress localSocket = InetSocketAddress (Ipv4Address::GetAny (),9803);
		recvNodeSink->Bind (localSocket);
		recvNodeSink->SetRecvCallback (MakeCallback (&ReceiveTopicRequest));

		InetSocketAddress remoteSocket = InetSocketAddress (iap.GetAddress (baseStationIndex, 0), 9803);
		Ptr<Socket> sourceNodeSocket = Socket::CreateSocket (sourceNode, tid);
		sourceNodeSocket->Connect (remoteSocket);
		Simulator::ScheduleNow (&SendTopicRequest, sourceNodeSocket,nodeind);
}
	
}

static void Vanet(){
		
	cout << "Inside vanet.... for round "<< currentRound << endl;
	
	if(currentRound == numRounds - 1)
	{
		//stop simulation
	}
	
	// erase all maps for fresh round - do it in the end before calling Vanet() for the next round
	// or for each node get the previous cluster details and leave that cluster and join the new cluster
	//clusterMgr->eraseAllMaps();
	
	for(int nodeind = 0; nodeind < nodeNum; nodeind++)
    {
		int random_topic_id = randomNumberGenerator(numofTopics); 
		//cout<< random_topic_id << endl;
		//put all nodes in some cluster based on the topic of interest
		cout<<"In cluster number................ : "<< clusterMgr->getClusterIDFromNode(vehicles.Get(nodeind)) << endl;
		
		if(clusterMgr->getClusterIDFromNode(vehicles.Get(nodeind)) != -999){
			clusterMgr->leave_Cluster(vehicles.Get(nodeind), nodeind);
		}
			
		clusterMgr->join_Cluster(vehicles.Get(nodeind), nodeind, random_topic_id);
		cout<<"In cluster number : "<< clusterMgr->getClusterIDFromNode(vehicles.Get(nodeind)) << endl;
		cout << "Is Master : " << clusterMgr->isMaster(vehicles.Get(nodeind)) << endl;
		cout<< endl;
	}	

	cout << "Total number of clusters formed : " << clusterMgr->getNumberOfClusters() << endl;

	requestForTopic();

}

//////////////////////////////////////////////////////////////////////////////


// Example to use ns2 traces file in ns3
int main (int argc, char *argv[])
{	
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

	vehicles.Create (nodeNum);
	wifiApNode.Create (1);
	
	// set the node container for the clusterManager
	
	clusterMgr->setNodeContainer(wifiApNode);
	
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
	NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, vehicles);
	
	// base station
	NetDeviceContainer apDevices = wifi.Install (wifiPhy, wifiMac, wifiApNode);


	// Create Ns2MobilityHelper with the specified trace log file as parameter
	Ns2MobilityHelper ns2mobility = Ns2MobilityHelper (traceFile);
	ns2mobility.Install (); // configure movements for each node, while reading trace file
	
	
	// set mobility model for base station AP
	
	MobilityHelper mobility;

	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (10.0),
                                 "MinY", DoubleValue (10.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (2.0),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));
	
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (wifiApNode);

	Ipv4StaticRoutingHelper staticRouting;
	Ipv4ListRoutingHelper list;
	list.Add (staticRouting, 0);

	InternetStackHelper internet;
	internet.SetRoutingHelper (list); // has effect on the next Install ()
	internet.Install (vehicles);
	internet.Install (wifiApNode);

	Ipv4AddressHelper ipv4;

	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	i = ipv4.Assign (devices);
	iap = ipv4.Assign (apDevices);

	tid = TypeId::LookupByName ("ns3::UdpSocketFactory");


	// Configure callback for logging
	//Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
				   //MakeBoundCallback (&CourseChange, &os));


	Simulator::Stop (Seconds (duration));
	AnimationInterface anim ("animation.xml");

	/////////////////////////////////////////////////////
	
	Vanet();
	
	//////////////////////////////////////

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
