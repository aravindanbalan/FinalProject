
#include "FinalTry.h"

using namespace ns3;
static void PerformStep2(int masterNode, string slaveString);
bool Configure (int argc, char *argv[])
{
	enableFlowMonitor = true;
	total_lost_packets = 0;
    CommandLine cmd;
    cmd.AddValue ("nodeNum", "Number of nodes.", nodeNum);
    cmd.AddValue ("duration", "Simulation time in sec.", duration);
    cmd.AddValue ("rounds", "Number of rounds to perform", numRounds);
    cmd.AddValue ("topics", "Number of topics of interest", numofTopics);
    cmd.AddValue ("traceFile", "NS3 mobility trace.", traceFile);
    cmd.AddValue ("logFile", "Log file", logFile);
    cmd.AddValue ("verbose", "Tell application to log if true", verbose);

	nodeMobileNodes = nodeNum - 1;
	currentRound = 0;
    if (verbose)
    {
        LogComponentEnable ("ClusterManager", LOG_LEVEL_INFO);
    }

    cmd.Parse (argc, argv);
    return true;
}

void CreateNodes ()
{
    Ns2MobilityHelper mob = Ns2MobilityHelper (traceFile);
    myos.open (logFile.c_str ());
    std::cout << "Creating " << nodeNum << " nodes.\n";
    nodes.Create (nodeNum);
    for(uint32_t nodeind = 0; nodeind < nodeNum; nodeind++)
    {
        clusterMgr->putNodeInMap(nodes.Get(nodeind),nodeind);
    }
	mob.Install ();
    Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange", MakeBoundCallback (&CourseChange, &myos));
}

void CreateDevices ()
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

void InstallInternetStack ()
{
    stack.Install (nodes);
    address.SetBase ("1.0.0.0", "255.0.0.0");

    interfaces = address.Assign (devices);
 
}

 void form_Initial_Cluster()
{
	for(uint32_t nodeind = 0; nodeind < nodeMobileNodes; nodeind++)
    {
		int random_topic_id = randomNumberGenerator(numofTopics); 
			
		clusterMgr->join_Cluster(nodes.Get(nodeind), nodeind, random_topic_id);
		//cout<<"In cluster number : "<< clusterMgr->getClusterIDFromNode(nodes.Get(nodeind)) << endl;
	}	
	int numClusters = clusterMgr->getNumberOfClusters();
	cout<<"Number of clusters : "<<  numClusters << endl;
	number_slaves = nodeMobileNodes - numofTopics;
	
	round_duration = duration/numRounds;
	number_masters = numofTopics;
	
	/*
	for(int i =0 ;i < numofTopics;i++)
	{		
		cluster_sizes[i] = clusterMgr->getClusterFromClusterID(i)->getNumNodes();
	}
	* 
	* */
	
}


static void SendToMasterOfEachCluster(Ptr<Socket> socket, int src)
{
    Ptr<Packet> sendPacket = Create<Packet> (size);

	//cout<<"Sending packet from....."<<src<<endl;
    MyTag sendTag;
    sendTag.SetSourceAddress(src);
	
    sendPacket->AddPacketTag(sendTag);
    socket->Send(sendPacket);
    socket->Close();	
}

void ReceiveMaster(Ptr<Socket> socket)
{
	Ptr<Packet> recPacket = socket->Recv();
	Ptr<Node> recvnode = socket->GetNode();
    int recNodeIndex = clusterMgr->getNodeFromMap(recvnode);

	//cout<<"Recieving packet....."<<endl;
    uint8_t *buffer = new uint8_t[recPacket->GetSize()];
    recPacket->CopyData(buffer,recPacket->GetSize());

    //int srcNodeIndex = readSourceAddressPacketTag(recPacket);
    
	//std::cout<<"Master "<< recNodeIndex<<" received from : "<< srcNodeIndex <<endl;
	
	int clusterID = clusterMgr->getClusterIDFromNode(nodes.Get(recNodeIndex));
	string slaveString = clusterMgr->getSlaveNodeIDsFromCluster(clusterID);
	
	socket->Close();
	number_masters--;
	Simulator::ScheduleNow (&PerformStep2, recNodeIndex, slaveString);
}



 void choose_master()
{
	int numClusters = clusterMgr->getNumberOfClusters();
	cout<<"round : "<< currentRound <<" numClusters "<<numClusters<<endl;
	for(int clusterID = 0 ; clusterID < numClusters ; clusterID++)
	{
		clusterMgr->choose_Master(clusterID);
		int masterNodeID = clusterMgr->getMasterNodeIDFromCluster(clusterID);
		cout<<"Master in cluster : "<<clusterID << " is : "<<masterNodeID<<endl;
	}
}

	
// Broadcast to all masters
 void PerformStep1()
{
	//cout<<"Performing Broadcast to all masters...."<<endl;
	
	int numClusters = clusterMgr->getNumberOfClusters();
	for(int clusterID = 0 ; clusterID < numClusters ; clusterID++)
	{
		int masterNodeID = clusterMgr->getMasterNodeIDFromCluster(clusterID);
		string slaveString = clusterMgr->getSlaveNodeIDsFromCluster(clusterID);
		//cout<<"Cluster ID : "<<clusterID << " , master node : "<<masterNodeID<<endl;
		//cout<<"Cluster ID : "<<clusterID << " , Slave list : "<<slaveString<<endl;
		//form the vector to include in the packet header
		masterIDs.push_back(masterNodeID);
		slaveListStrings.push_back(slaveString);
	}
	
	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	static const InetSocketAddress beaconBroadcast = InetSocketAddress(Ipv4Address::GetBroadcast(),9);

	Ptr<Node> sourceNode = nodes.Get (50);
	Ptr<Socket> beacon_source = Socket::CreateSocket(sourceNode, tid);
	beacon_source->Connect(beaconBroadcast);
	beacon_source->SetAllowBroadcast (true);
	
	for(int clusterID = 0 ; clusterID < numClusters ; clusterID++)
	{
	  int masterNodeID = clusterMgr->getMasterNodeIDFromCluster(clusterID);
	  Ptr<Socket> beacon_sink = Socket::CreateSocket(nodes.Get(masterNodeID),tid);
	  InetSocketAddress beacon_local = InetSocketAddress(Ipv4Address::GetAny(), 9);
	  beacon_sink->Bind(beacon_local);
	  beacon_sink->SetRecvCallback(MakeCallback(&ReceiveMaster));  
	}
	Simulator::ScheduleNow (&SendToMasterOfEachCluster, beacon_source,50);
}


void StartSimulation()
{
	cout<<"Current round ........ "<<currentRound<< " total : "<<numRounds<<endl;
	if(currentRound == numRounds)
	{
		
		Simulator::Stop ();
	}
	else
	{
		choose_master();
		PerformStep1();
	}
}



void DistributeToAllNodes(Ptr<Socket> socket, int src, int topic)
{
    Ptr<Packet> sendPacket = Create<Packet> (size);

	cout<<"Distributing packet for topic....."<<topic<<endl;
    MyTag sendTag;
    sendTag.SetSourceAddress(src);
	sendTag.SetTopic(topic);
    sendPacket->AddPacketTag(sendTag);
    socket->Send(sendPacket);
    socket->Close();	
    
    
    if(number_masters == 0)
    {
		currentRound++;
			//cout<<"Next round ........ "<< currentRound<< " "<< round_duration <<endl;
			number_masters = numofTopics;
			 cout<<"*******************number of masters sent"<<number_masters<<endl;
			
			Simulator::Schedule (Seconds(round_duration/1000.0),&StartSimulation);
			//Simulator::ScheduleNow(&StartSimulation);
	}
	
}

/*
bool checkIfAllDone(){
	
	for(int i =0 ;i<numofTopics ; i++)
	{
		if(cluster_sizes[i] != 1)
			return false;
	}
	
	return true;
}

void printsizes(){
	
	for(int i =0 ;i<numofTopics ; i++)
	{
		cout<<" "<<cluster_sizes[i];
	}
	cout<<endl;

}

void resetAll(){
	
	for(int i =0 ;i<numofTopics ; i++)
	{
		cluster_sizes[i] = clusterMgr->getClusterFromClusterID(i)->getNumNodes();
	}

}
* */

void ReceiveSlave(Ptr<Socket> socket)
{
	Ptr<Packet> recPacket = socket->Recv();
	Ptr<Node> slaveNode = socket->GetNode();
    int recNodeIndex = clusterMgr->getNodeFromMap(slaveNode);

	//cout<<"Slave Recieving packet....."<<endl;	
	
	uint8_t *buffer = new uint8_t[recPacket->GetSize()];
    recPacket->CopyData(buffer,recPacket->GetSize());

    int srcNodeIndex = readSourceAddressPacketTag(recPacket);
    int topic = readTopicFromPacket(recPacket);
    
    int interested_topic = clusterMgr->getTopicFromNode(slaveNode);
    
    if(topic == interested_topic){
    
		std::cout<<"Slave "<< recNodeIndex<<" received from : "<< srcNodeIndex << " Topic : "<< topic<<endl;
		
		//cluster_sizes[topic]--;
		//printsizes();
		
		//number_slaves--;
		
		/*
		cout<<"number of slaves received"<<number_slaves<<endl;
		if(number_slaves == 0)
		{
			currentRound++;
			//cout<<"Next round ........ "<< currentRound<< " "<< round_duration <<endl;
			number_slaves = nodeMobileNodes - numofTopics;
			 
			Simulator::Schedule (Seconds(round_duration/1000.0),&StartSimulation);
			//Simulator::ScheduleNow(&StartSimulation);
		}
		* */
		
		/*
		if(checkIfAllDone())
		{
			cout<<"Calling next round ... "<<endl;
			currentRound++;
			resetAll();
		
			Simulator::Schedule (Seconds(round_duration/1000.0),&StartSimulation);
			//Simulator::ScheduleNow(&StartSimulation);
			
		}
		* */
	
		
	}
}


// Distribute to all slaves in the cluster
static void PerformStep2(int masterNode, string slaveString)
{
	//cout<<"Performing Broadcast to all peers...."<<endl;
	//cout<<"Recieved slave string from master "<< masterNode << " is "<<endl;
	//cout<<slaveString<<endl;
	
	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	static const InetSocketAddress beaconBroadcast = InetSocketAddress(Ipv4Address::GetBroadcast(),10);

	Ptr<Node> sourceNode = nodes.Get (masterNode);
	Ptr<Socket> beacon_source = Socket::CreateSocket(sourceNode, tid);
	beacon_source->Connect(beaconBroadcast);
	beacon_source->SetAllowBroadcast (true);
	
	int topic = clusterMgr->getTopicFromNode(sourceNode);
	
	
	
	vector<std::string> slaves = getTokens(slaveString);
	for(uint32_t i = 0; i < nodeMobileNodes; i++) {
		
	  Ptr<Socket> beacon_sink = Socket::CreateSocket(nodes.Get(i),tid);
	  InetSocketAddress beacon_local = InetSocketAddress(Ipv4Address::GetAny(), 10);
	  beacon_sink->Bind(beacon_local);
	  beacon_sink->SetRecvCallback(MakeCallback(&ReceiveSlave)); 				
							
	}
	
	Simulator::Schedule (Seconds(topic /1000.0),&DistributeToAllNodes, beacon_source,masterNode,topic);

	//Simulator::Schedule (Seconds((round_duration + topic)/100000.0),&DistributeToAllNodes, beacon_source,masterNode,topic);
}


int main (int argc, char *argv[])
{
	
	nodeNum = 51; 
	numRounds = 2; 
	numofTopics = 1; 
	duration = 300.0; 
	pcap = false;
    verbose = true;
    
    nodeMobileNodes = nodeNum - 1;
      
  
    
	if (!Configure (argc, argv))
        cout<<"Configuration failed. Aborted."<<endl;
   
    clusterMgr = ClusterManager::getInstance();

    CreateNodes ();
    CreateDevices ();
    InstallInternetStack ();
    
    clusterMgr->setClusterMgrNode(nodes.Get (50));
    
    
     std::cout << "Starting simulation for " << duration << " s.\n";

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    AnimationInterface::SetConstantPosition (nodes.Get (50), 7.6, 198.35);
	AnimationInterface anim ("vanet_animation.xml");
	anim.EnablePacketMetadata (true);
    
    form_Initial_Cluster();
    Simulator::ScheduleNow (&StartSimulation);
    
    
    Simulator::Run ();

	
	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor;
	if (enableFlowMonitor)
    {
      
		monitor = flowmon.InstallAll(); 
    }
   
    
    myos.close (); // close log file
 

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


  monitor->SerializeToXmlFile ("vanet_flowmon.xml", true, true);

		
    }
    
    std::cout<<"Total number of lost packets : "<<total_lost_packets<<std::endl;
    Simulator::Destroy ();

    return 0;
}
