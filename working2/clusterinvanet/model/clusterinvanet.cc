/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "clusterinvanet.h"

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("Vanet");

	NS_OBJECT_ENSURE_REGISTERED (Vanet);


 std::string Vanet::logFile;
 std::ofstream Vanet::myos;
	std::string Vanet::traceFile;
	 double Vanet::round_start;
	double Vanet::round_end;
    uint32_t Vanet::nodeNum;
    uint32_t Vanet::nodeMobileNodes;
    int Vanet::packet_type;
    int Vanet::numRounds;
	int Vanet::numofTopics;
    double Vanet::duration;
    bool Vanet::pcap;
    bool Vanet::enableFlowMonitor = true;
    bool Vanet::verbose;
    Ptr<ClusterManager> Vanet::baseStationApp[30];
    vector<int> Vanet::masterIDs;
    vector<string> Vanet::slaveListStrings;

    NodeContainer Vanet::nodes;
    NodeContainer Vanet::wifiApNode;
    NetDeviceContainer Vanet::devices;
    Ipv4InterfaceContainer Vanet::interfaces;
    Ipv4InterfaceContainer Vanet::interfaceBS;
    NetDeviceContainer Vanet::apDevices;
    uint32_t Vanet::total_lost_packets;
    int Vanet::round_duration;



int randomNumberGenerator(int numTopics)
{
		int v1 = rand() % numTopics; 
		return v1;
}
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


void Vanet::setupEnv(int argc, char *argv[]){
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
		
	//Simulator::ScheduleNow(&ClusterManager::sampleTest);
	Simulator::ScheduleNow(&sampleRun, 1, numRounds, round_start, round_end, round_duration);
    
}

void Vanet::test(){
	
	cout<<"########################################test"<<endl;
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

void Vanet::sampleRun(int round, int numRnds, double rnd_start, double rnd_end , double rnd_duration)
{
	
	cout<<"Running sample run ...... for round : "<<round<<endl;

	cout<<"Round duration : "<<rnd_duration<<endl;
	cout<<"Round start : "<<rnd_start<<endl;
	cout<<"Round end : "<<rnd_end<<endl;
		
	performRound(round,rnd_duration, rnd_start, rnd_end);	
	
}

void Vanet::Run ()
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

void Vanet::ChooseMaster(Ptr<ClusterManager> bsApp, int round)
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

void Vanet::performRound(int round, int round_duration, int round_start, int round_end)
{
	std::cout<<"Performing round : "<<round<<std::endl;
	FormClusters(round, round_duration, round_start, round_end);
}

void Vanet::FormClusters (int round, int round_duration, int round_start, int round_end)
{
	
	ChooseMaster(baseStationApp[round], round);
	
	cout << "Total number of clusters formed : " << baseStationApp[round]->getNumberOfClusters() << endl;
	
	SendToMasterOfEachCluster(round, round_duration , round_start, round_end); 	
}

void Vanet::SendToMasterOfEachCluster (int round , int round_duration, int round_start, int round_end)
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

cout<<"Simulator now for round : "<< round <<"   : "<<Simulator::Now() << "  : " <<round_start << " : "<< round_end <<endl;

cout<<"chosen master size "<<masterIDs.size()<<endl;
	for(std::vector<int>::size_type i = 0; i != masterIDs.size(); i++) {
		masters[i] = CreateObject<ClusterMember> ();
		cout<<"chosen master "<<masterIDs[i]<<endl;
		masters[i]->Setup (Ipv4Address::GetBroadcast (), 9, DataRate ("1Mbps"), false, false, masterIDs[i], slaveListStrings[i], nodes,interfaces, baseStationApp[round], 0,0, round_start, round_end, round, numRounds, round_duration);
		nodes.Get (masterIDs[i])->AddApplication (masters[i]);
		masters[i]->SetStartTime (Seconds (round_start));
		masters[i]->SetStopTime (Seconds (round_end));		
	}
	
	
	baseStationApp[round]->Setup (Ipv4Address::GetBroadcast (), 9, DataRate ("1Mbps"), true, true, packet_type);
	nodes.Get (50)->AddApplication (baseStationApp[round]);
	baseStationApp[round]->SetStartTime (Seconds (round_start ));
	baseStationApp[round]->SetStopTime (Seconds (round_end));


	if(numofTopics == 1)
	{
		// new code receiver for all bradcast in port 10
		Ptr<ClusterMember> allNodesBroadcastReceiverApp[nodeMobileNodes];
		for(uint32_t i = 0 ; i< nodeMobileNodes; i++) {
		
				allNodesBroadcastReceiverApp[i] = CreateObject<ClusterMember> ();
				allNodesBroadcastReceiverApp[i]->Setup (Ipv4Address::GetBroadcast (), 10, DataRate ("1Mbps"), false, false, i, "", nodes,interfaces, baseStationApp[round], 0,0, round_start, round_end, round, numRounds, round_duration);
				nodes.Get (i)->AddApplication (allNodesBroadcastReceiverApp[i]);
				allNodesBroadcastReceiverApp[i]->SetStartTime (Seconds (round_start));
				allNodesBroadcastReceiverApp[i]->SetStopTime (Seconds (round_end));	
				
		}
		
	}
	
}


}

