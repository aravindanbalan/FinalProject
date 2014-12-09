
#include "FinalTry.h"

using namespace ns3;
static void PerformStep2(int masterNode);
void choose_master();
bool Configure (int argc, char *argv[])
{
	enableFlowMonitor = true;
	total_lost_packets = 0;
    CommandLine cmd;
    cmd.AddValue ("nodeNum", "Number of nodes.", nodeNum);
    cmd.AddValue ("duration", "Simulation time in sec.", duration);
    cmd.AddValue ("rounds", "Number of rounds to perform", numRounds);
    cmd.AddValue ("topics", "Number of topics of interest", numofTopics);
    cmd.AddValue ("masters", "Number of masters in each cluster", numMasters);
    cmd.AddValue ("range", "Max Range to consider packet losses", maxRange);
    cmd.AddValue ("distanceCheck", "Check distance while choosing master", check);
    cmd.AddValue ("traceFile", "NS3 mobility trace.", traceFile);
    cmd.AddValue ("threshold", "Distance Threshold while choosing masters", distanceThreshold);
    cmd.AddValue ("logFile", "Log file", logFile);
    cmd.AddValue ("selfish", "Enable/Disable selfishness of masters", isSelfish);
    cmd.AddValue ("prob", "Probability of a node being selfish", selfishprob);
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
    if (verbose) std::cout << "Creating " << nodeNum << " nodes.\n";
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
    std::string phyMode ("OfdmRate6Mbps");

    // disable fragmentation for frames below 2200 bytes
    Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
    // turn off RTS/CTS for frames below 2200 bytes
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
    // Fix non-unicast data rate to be the same as that of unicast
    Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

    
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    //channel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");//,
                               //"MaxRange", DoubleValue(100.0)); //XXX
    
    channel.AddPropagationLoss("ns3::RangePropagationLossModel",
                               "MaxRange", DoubleValue(maxRange));
   

    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    phy.SetChannel (channel.Create ());

    WifiHelper wifi = WifiHelper::Default ();
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode",StringValue (phyMode),
                                  "ControlMode",StringValue (phyMode),
                                  //"MaxSlrc",UintegerValue (2),
                                  "FragmentationThreshold", UintegerValue (2000));
    
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
		int selfish = 0;
		
		if(isSelfish)
		{
			selfish = randomBitGeneratorWithProb(selfishprob);	
			if (verbose) cout<<"Node : "<< nodeind <<" selfish : "<< selfish<<endl;
			if(selfish) self++;	
		}
			
		clusterMgr->join_Cluster(nodes.Get(nodeind), nodeind, random_topic_id, selfish);
	}	
	
	number_slaves = nodeMobileNodes - numofTopics;
	
	round_duration = duration/numRounds;
	number_masters = numofTopics * numMasters;
	
	cout<<"Starting Round "<<endl;
	

	
}

void ConnectionSucceededCallback (Ptr<Socket> socket)
{
 //cout<<"*****************************connection success"<<endl;
}
void ConnectionFailedCallback (Ptr<Socket> socket)
{
 //cout<<"*****************************connection failure"<<endl;
}

void DataSentCallback (Ptr<Socket> socket, uint32_t sent)
{
	//cout<<"*****************************Data Sent"<<endl;
}

void ConnectionRequestCallback (bool val,Ptr<Socket> socket,const Address &addr)
{
	//cout<<"*****************************new connection rquest"<<endl;
}

void ConnectionCreatedCallBack (Ptr<Socket> socket,const Address &addr)
{
	//cout<<"*****************************Connection created"<<endl;
}

static void SendToMasterOfEachCluster(Ptr<Socket> socket, int src)
{
    Ptr<Packet> sendPacket = Create<Packet> (size);

	//cout<<"src : "<<src<<endl;
    MyTag sendTag;
    sendTag.SetSourceAddress(src);
	
    sendPacket->AddPacketTag(sendTag);
    socket->Send(sendPacket);
    socket->Close();	
}

bool checkIfAllMastersSent(){
	
	map<int,bool>::iterator p;
	bool done = true;
	for(p = clusterSentMap.begin(); p != clusterSentMap.end(); p++) {
		done = done & p->second;
	}
	return done;
}

void resetClusterSentMap()
{
	for(int clusterID = 0 ; clusterID < numofTopics ; clusterID++)
	{
		setClusterSentMap(clusterID,false);
	}
}

void StartSimulation()
{
	cout<< currentRound <<" ";
	
	if(currentRound == numRounds)
	{
		   // currentRound = 0;
			//	Simulator::Schedule(Seconds(10.0),&StartSimulation);
	}
	else if(currentRound < numRounds)
	{
		currentRound++;
		//resetClusterSentMap();
		choose_master();
	}
}

void ReceiveMaster(Ptr<Socket> socket)
{
	Ptr<Packet> recPacket = socket->Recv();
	Ptr<Node> recvnode = socket->GetNode();
    int recNodeIndex = clusterMgr->getNodeFromMap(recvnode);

    uint8_t *buffer = new uint8_t[recPacket->GetSize()];
    recPacket->CopyData(buffer,recPacket->GetSize());

    //int srcNodeIndex = readSourceAddressPacketTag(recPacket);

	//std::cout<<"Master "<< recNodeIndex<<" received from : "<< srcNodeIndex <<endl;

	int clusterID = clusterMgr->getClusterIDFromNode(recvnode);
	
	// since previou round master sockets are also open and previous round masters are also receiving, thats why closed the socket
	//socket->Close();
	number_masters--;
	packets_recv_master++;

	if(isSelfish)
	{
		//check if the node is selfish , in that case dont distribute the packets to slaves
		if(!clusterMgr->getNodeFromSelfishMap(recvnode)){
			if (verbose) std::cout<<"Master "<< recNodeIndex<<" distributing the packets ...."<<endl;
			setClusterSentMap(clusterID,true);
			Simulator::ScheduleNow (&PerformStep2, recNodeIndex);
		}
		else{
			setClusterSentMap(clusterID,false);
			if (verbose) cout<<"Master Node : "<<recNodeIndex <<" is selfish and does not distribute to the slaves...."<<endl;
			num_selfish_masters++;
			// if all masters done call next round
			
			if(number_masters == 1)
			{
				number_masters = numofTopics * numMasters;
				//Simulator::Schedule (Seconds(round_duration/1000.0),&StartSimulation);
				Simulator::Schedule (Seconds(1.0),&StartSimulation);
			}
		
		}
	}
	else{
			if (verbose) std::cout<<"Master "<< recNodeIndex<<" distributing the packets ...."<<endl;
			//setClusterSentMap(clusterID,true);
			Simulator::ScheduleNow (&PerformStep2, recNodeIndex);
	}
	
}

	
// Broadcast to all masters
 void PerformStep1()
{
	
	int numClusters = clusterMgr->getNumberOfClusters();
	
	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	/*
	static const InetSocketAddress beaconBroadcast = InetSocketAddress(Ipv4Address::GetBroadcast(),9);

	Ptr<Node> sourceNode = nodes.Get (50);
	Ptr<Socket> beacon_source = Socket::CreateSocket(sourceNode, tid);
	beacon_source->Connect(beaconBroadcast);
	beacon_source->SetAllowBroadcast (true);
	*/
	for(int clusterID = 0 ; clusterID < numClusters ; clusterID++)
	{
	  vector<int> masterNodeIDs = clusterMgr->getMasterNodeIDsFromCluster(clusterID);
	  
	  // make only first master in each cluster to receive and to broadcast the packet
	  // because while creating beacon_source, multiple nodes cannot connect to same (broadcast address + port) socket.....
	  for(uint32_t i =0;i < masterNodeIDs.size();i++)
	  {
			//for(int topic = 0;topic < numofTopics;topic++)
			{
				Ptr<Socket> recvNodeSink = Socket::CreateSocket (nodes.Get (masterNodeIDs[i]), tid);
				InetSocketAddress localSocket = InetSocketAddress (Ipv4Address::GetAny (),9);
				recvNodeSink->Bind (localSocket);
				recvNodeSink->SetRecvCallback (MakeCallback (&ReceiveMaster));
				
				InetSocketAddress remoteSocket = InetSocketAddress (interfaces.GetAddress (masterNodeIDs[i], 0), 9);
				Ptr<Socket> sourceNodeSocket = Socket::CreateSocket (nodes.Get (50), tid);
				sourceNodeSocket->SetConnectCallback (
					MakeCallback (&ConnectionSucceededCallback),
					MakeCallback (&ConnectionFailedCallback) );
				sourceNodeSocket->SetDataSentCallback(MakeCallback (&DataSentCallback));
				sourceNodeSocket->Connect (remoteSocket);
				   
				Simulator::ScheduleNow (&SendToMasterOfEachCluster, sourceNodeSocket,50);
			}
	  }
	}
	
}

void choose_master()
{
	int numClusters = clusterMgr->getNumberOfClusters();
	//cout<<"round : "<< currentRound <<" numClusters "<<numClusters<<endl;
	for(int clusterID = 0 ; clusterID < numClusters ; clusterID++)
	{
		
		bool done = clusterMgr->choose_Master(clusterID, numMasters, nodes);
		
		if(!done) Simulator::Stop(Seconds(duration));
		
		vector<int> masterNodeIDs = clusterMgr->getMasterNodeIDsFromCluster(clusterID);
		//if (verbose) 
		{
			cout<<"Masters in cluster id : "<<clusterID<<endl;
			for(uint32_t i =0;i < masterNodeIDs.size();i++)
			{
				cout<<" "<<masterNodeIDs[i];
			}	
			cout<<endl;
		}		
	}
	Simulator::ScheduleNow (&PerformStep1);
}



void DistributeToAllNodes(Ptr<Socket> socket, int src, int topic)
{
    Ptr<Packet> sendPacket = Create<Packet> (size);
	//cout<<"Distributing packet for topic....."<<topic<<" from "<<src<<endl;
    MyTag sendTag;
    sendTag.SetSourceAddress(src);
	sendTag.SetTopic(topic);
    sendPacket->AddPacketTag(sendTag);
    socket->Send(sendPacket);
    socket->Close();	

	if(numMasters > 1)
	{
		if(number_masters == 1)
	   {
			number_masters = numofTopics * numMasters;
			//Simulator::Schedule (Seconds(round_duration/1000.0),&StartSimulation);
			Simulator::Schedule (Seconds(1.0),&StartSimulation);
		}
	}
	else
	{
		if(number_masters == 0)
	   {
			number_masters = numofTopics * numMasters;
			//Simulator::Schedule (Seconds(round_duration/1000.0),&StartSimulation);
			Simulator::Schedule (Seconds(1.0),&StartSimulation);
		}

	}
	
}

void ReceiveSlave(Ptr<Socket> socket)
{
	Ptr<Packet> recPacket = socket->Recv();
	packets_recv_slaves++;
	Ptr<Node> slaveNode = socket->GetNode();
    int recNodeIndex = clusterMgr->getNodeFromMap(slaveNode);

	if(clusterMgr->checkIfAlreadyChosenBefore(recNodeIndex) && currentRound > 0)
			packets_recv_decoded++;

	uint8_t *buffer = new uint8_t[recPacket->GetSize()];
    recPacket->CopyData(buffer,recPacket->GetSize());

    int srcNodeIndex = readSourceAddressPacketTag(recPacket);
    int topic = readTopicFromPacket(recPacket);
    int interested_topic = clusterMgr->getTopicFromNode(slaveNode);
    
     //socket->Close();
     
    // cout<<"Topic received : "<<topic<<endl;
    if(topic == interested_topic && !clusterMgr->isMaster(slaveNode)){
    
		if (verbose) std::cout<<"Slave "<< recNodeIndex<<" received from : "<< srcNodeIndex << " Topic : "<< topic<<endl;
		
	}
}

// Distribute to all slaves in the cluster
static void PerformStep2(int masterNode)
{
	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	
	/*
	static const InetSocketAddress beaconBroadcast = InetSocketAddress(Ipv4Address::GetBroadcast(),10);

	
	Ptr<Socket> beacon_source = Socket::CreateSocket(sourceNode, tid);
	beacon_source->Connect(beaconBroadcast);
	beacon_source->SetAllowBroadcast (true);
	* 
	* */
	Ptr<Node> sourceNode = nodes.Get (masterNode);
	int topic = clusterMgr->getTopicFromNode(sourceNode);
	string slaveString = clusterMgr->getSlaveStrForMasterFromMap(sourceNode);
	
	vector<std::string> slaves = getTokens(slaveString);
		
	 for(std::vector<int>::size_type i = 0; i != slaves.size(); i++) {
		 
		 std::string slave = slaves[i];
		int slaveID = atoi(slave.c_str());
		  
		Ptr<Socket> recvNodeSink = Socket::CreateSocket (nodes.Get (slaveID), tid);
		InetSocketAddress localSocket = InetSocketAddress (Ipv4Address::GetAny (),10);
		recvNodeSink->Bind (localSocket);
		recvNodeSink->SetRecvCallback (MakeCallback (&ReceiveSlave));	
		
		InetSocketAddress remoteSocket = InetSocketAddress (interfaces.GetAddress (slaveID, 0), 10);		
		Ptr<Socket> sourceNodeSocket = Socket::CreateSocket (nodes.Get (masterNode), tid);
		sourceNodeSocket->Connect (remoteSocket);
                
		//Simulator::Schedule (Seconds(topic /1000.0),&DistributeToAllNodes, sourceNodeSocket,masterNode,topic);
		Simulator::ScheduleNow (&DistributeToAllNodes, sourceNodeSocket,masterNode,topic);		
		}
	
}


int main (int argc, char *argv[])
{
	
	nodeNum = 51; 
	numRounds = 1; 
	numofTopics = 1;
	numMasters = 1; 
	duration = 300.0; 
	pcap = true;
    verbose = true;
    self=0;
    enableFlowMonitor = true;
    check = 0;
    isSelfish = 0;
    distanceThreshold = 0.0;
    maxRange = 1000.0;
    selfishprob = 0.5;
    packets_recv_master = 0;
    packets_recv_slaves = 0;
    packets_recv_decoded = 0;
    num_selfish_masters = 0;
    
    nodeMobileNodes = nodeNum - 1;
  
	if (!Configure (argc, argv))
        cout<<"Configuration failed. Aborted."<<endl;
   
    clusterMgr = ClusterManager::getInstance();

    CreateNodes ();
    CreateDevices ();
    InstallInternetStack ();
    
    clusterMgr->setClusterMgrNode(nodes.Get (50));
    clusterMgr->setMasterCheckParameters(check,distanceThreshold);
    
    std::cout << "Starting simulation for " << duration << " s.\n";
            
    currentRound = 0;
    form_Initial_Cluster();
    Simulator::Schedule(Seconds(10.0),&StartSimulation);
   
    
    Simulator::Stop (Seconds(duration));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    
   
    AnimationInterface::SetConstantPosition (nodes.Get (50), 7.6, 198.35);

	AnimationInterface anim ("vanet_animation.xml");
	anim.SetMobilityPollInterval (Seconds (1));
	anim.EnablePacketMetadata (true);
    

	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor;
      
	monitor = flowmon.InstallAll(); 
 
    Simulator::Run ();
    
    //displayStats();
    myos.close (); // close log file

		monitor->CheckForLostPackets ();
		
		Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  
  
  
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
     // Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      //std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
     // std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
     // std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
     // std::cout << "  Tx Packets: " << i->second.txPackets << std::endl; 
      //   std::cout << "  Rx Packets: " << i->second.rxPackets << std::endl; 
       //  std::cout << "  Lost Packets: " << i->second.lostPackets << std::endl; 
         total_lost_packets += i->second.lostPackets;
      
      //std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (duration - clientStart) / 1024 / 1024  << " Mbps\n";
    }

  monitor->SerializeToXmlFile ("vanet_flowmon.xml", true, true);

	cout<<endl;
	std::cout<<"Current round : "<<currentRound<<std::endl;
	cout<<"Total Sent packets : "<<stats.size()<<endl;  
	
    std::cout<<"Total number of lost packets : "<<total_lost_packets<<std::endl;
    std::cout<<"Total number of packets received by master : "<<packets_recv_master<<std::endl;
    std::cout<<"Sent Packets stage 2 : "<< (stats.size() - packets_recv_master)<<std::endl;
    std::cout<<"Total number of packets received by slave : "<<packets_recv_slaves<<std::endl;
    std::cout<<"Total number of packets decoded by slaves : "<<packets_recv_decoded<<std::endl;
    std::cout<<"Total number of selfish masters that didnt foward : "<<num_selfish_masters<<std::endl;
    std::cout<<"self : "<<self<<std::endl;
    
    double totalsize = stats.size();
    double loss_rate = total_lost_packets/ (totalsize - packets_recv_master);
    
    double slaves = nodeMobileNodes - (numofTopics * numMasters);
    double avg_pkt_recv_slave = packets_recv_slaves / slaves;
     double avg_pkt_decoded_slave = packets_recv_decoded / slaves;
     double decode_rate = avg_pkt_decoded_slave / avg_pkt_recv_slave;
    
    std::cout.precision(5);
    std::cout<<"Loss Rate : "<<loss_rate<<std::endl;
    std::cout<<"Decode Rate : "<<decode_rate<<std::endl;
    std::cout<<"Avg Pkts received by slaves : "<<avg_pkt_recv_slave<<std::endl;
    std::cout<<"Avg Pkts decoded by slaves : "<<avg_pkt_decoded_slave<<std::endl;
    
    Simulator::Destroy ();

    return 0;
}
