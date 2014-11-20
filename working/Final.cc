// ./waf --run "scratch/mobTop5 --nodeNum=50 --traceFile=scratch/sc2.tcl"

#include "Final.h"

bool Configure (int argc, char *argv[])
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

static void sampleRun(int round, int numRnds, double rnd_start, double rnd_end , double rnd_duration)
{
	
	cout<<"Running sample run ...... for round : "<<round<<endl;

	cout<<"Round duration : "<<rnd_duration<<endl;
	cout<<"Round start : "<<rnd_start<<endl;
	cout<<"Round end : "<<rnd_end<<endl;
		
	performRound(round,rnd_duration, rnd_start, rnd_end);	
	
}

void Run ()
{
  
	
	// do this for many rounds
		round_duration = duration/numRounds;
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
    
}

void CreateNodes ()
{
    Ns2MobilityHelper mob = Ns2MobilityHelper (traceFile);
    myos.open (logFile.c_str ());
    std::cout << "Creating " << nodeNum << " nodes.\n";
    nodes.Create (nodeNum);
	mob.Install ();
    Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
		   MakeBoundCallback (&CourseChange, &myos));
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
    InternetStackHelper stack;
	
    stack.Install (nodes);
    Ipv4AddressHelper address;
    address.SetBase ("1.0.0.0", "255.0.0.0");

    interfaces = address.Assign (devices);
 
}

void ChooseMaster(Ptr<ClusterManager> bsApp, int round)
{
	cout<<"Choosing master for round : "<<round<<endl;
	
	int numClusters = bsApp->getNumberOfClusters();
	cout<<"round : "<< round<<" numClusters "<<numClusters<<endl;
	for(int clusterID = 0 ; clusterID < numClusters ; clusterID++)
	{
		//cout<<"round : ** "<< round<<endl;
		bsApp->choose_Master(clusterID);
		//cout<<"round : ------- "<< round<<endl;
		int masterNodeID = bsApp->getMasterNodeIDFromCluster(clusterID);
		cout<<"Master in cluster : "<<clusterID << " is : "<<masterNodeID<<endl;
	}
	
}

void performRound(int round, int round_duration, int round_start, int round_end)
{
	std::cout<<"Performing round : "<<round<<std::endl;
	FormClusters(round, round_duration, round_start, round_end);
}

void FormClusters (int round, int round_duration, int round_start, int round_end)
{
	
	ChooseMaster(baseStationApp[round], round);
	
	cout << "Total number of clusters formed : " << baseStationApp[round]->getNumberOfClusters() << endl;
	
	SendToMasterOfEachCluster(round, round_duration , round_start, round_end); 	
}

void SendToMasterOfEachCluster (int round , int round_duration, int round_start, int round_end)
{
	
	cout << "Round " << round<< " Start : "<<round_start<<" End : "<< round_end << " Duration : "<<round_duration << endl;
	int numClusters = baseStationApp[round]->getNumberOfClusters();
	cout << "Inside send master : " << numClusters << endl;
	
	
	for(int clusterID = 0 ; clusterID < numClusters ; clusterID++)
	{
		int masterNodeID = baseStationApp[round]->getMasterNodeIDFromCluster(clusterID);
		string slaveString = baseStationApp[round]->getSlaveNodeIDsFromCluster(clusterID);
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

		baseStationApp[round]->Setup (Ipv4Address::GetBroadcast (), 9, DataRate ("1Mbps"), true, true, packet_type);
		nodes.Get (50)->AddApplication (baseStationApp[round]);
	baseStationApp[round]->SetStartTime (Seconds (round_start ));
	baseStationApp[round]->SetStopTime (Seconds (round_end));

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
		masters[i]->Setup (Ipv4Address::GetBroadcast (), 9, DataRate ("1Mbps"), false, false, masterIDs[i], slaveListStrings[i], nodes,interfaces, baseStationApp[round], 0,0, round_start, round_end);
		nodes.Get (masterIDs[i])->AddApplication (masters[i]);
		masters[i]->SetStartTime (Seconds (round_start));
		masters[i]->SetStopTime (Seconds (round_end));		
	}
	
	
	if(numofTopics == 1)
	{
		// new code receiver for all bradcast in port 10
		Ptr<ClusterMember> allNodesBroadcastReceiverApp[nodeMobileNodes];
		for(uint32_t i = 0 ; i< nodeMobileNodes; i++) {
		
				allNodesBroadcastReceiverApp[i] = CreateObject<ClusterMember> ();
				allNodesBroadcastReceiverApp[i]->Setup (Ipv4Address::GetBroadcast (), 10, DataRate ("1Mbps"), false, false, i, "", nodes,interfaces, baseStationApp[round], 0,0, round_start, round_end);
				nodes.Get (i)->AddApplication (allNodesBroadcastReceiverApp[i]);
				allNodesBroadcastReceiverApp[i]->SetStartTime (Seconds (0.));
				allNodesBroadcastReceiverApp[i]->SetStopTime (Seconds (300.0));	
				
		}
		
	}
	
	masterIDs.erase (masterIDs.begin(),masterIDs.end());
	slaveListStrings.erase (slaveListStrings.begin(),slaveListStrings.end());	
			
		
	if(round <= numRounds && round_end < duration)
		Simulator::Schedule(Seconds(round_duration),&sampleRun, round+1, numRounds, round_start + round_duration, round_end + round_duration, round_duration);
	
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
        NS_FATAL_ERROR ("Configuration failed. Aborted.");
        
    CreateNodes ();
    CreateDevices ();
    InstallInternetStack ();

//////////////////////////////////////////////

   // Run ();
	
	
	int random_topic_id;	
	for(int round = 1 ; round <= numRounds ; round++ )	
	{	 
			baseStationApp[round] = CreateObject<ClusterManager> ();
			for(uint32_t nodeInd = 0; nodeInd < nodeMobileNodes ; nodeInd ++)
			{

					if(round == 1)	// choose random topic only for first round, for other rounds choose a different the master
					{
						random_topic_id = randomNumberGenerator(numofTopics); 
					}
					else
					{
						//get the topic assigned during round 1
						random_topic_id = baseStationApp[1]->getTopicFromNode(nodes.Get(nodeInd));
					}
						
					baseStationApp[round]->join_Cluster(nodes.Get(nodeInd), nodeInd, random_topic_id);
			}
	}
	
	round_duration = duration/numRounds;
	round_start = 1.0;
	round_end = round_duration;
		
	Simulator::ScheduleNow(&sampleRun, 1, numRounds, round_start, round_end, round_duration);
    
    ////////////////////////////////
    
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

    return 0;
}
