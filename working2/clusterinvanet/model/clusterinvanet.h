/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef CLUSTERINVANET_H
#define CLUSTERINVANET_H

#include "ns3/application.h"
#include <stdlib.h> 
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/average.h"
#include "ns3/simulator.h"
#include "ns3/clustermanager.h"
#include "ns3/clustermember.h"
#include <map>

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/event-id.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/log.h"

#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/flow-monitor-module.h"

namespace ns3 {

class Vanet : public Application
{
	public:
	 static bool Configure (int argc, char *argv[]);
    static void Run ();
    static std::string logFile;
    static std::ofstream myos;
    static std::string traceFile;

	static double round_start;
	static double round_end;
    static uint32_t nodeNum;
    static uint32_t nodeMobileNodes;
    static int packet_type;
    static int numRounds;
	static int numofTopics;
    static double duration;
    static bool pcap;
    static bool enableFlowMonitor;
    static bool verbose;
    static Ptr<ClusterManager> baseStationApp[30];
    static vector<int> masterIDs;
    static vector<string> slaveListStrings;

    static NodeContainer nodes;
    static NodeContainer wifiApNode;
    static NetDeviceContainer devices;
    static Ipv4InterfaceContainer interfaces;
    static Ipv4InterfaceContainer interfaceBS;
    static NetDeviceContainer apDevices;
    static uint32_t total_lost_packets;
    static int round_duration;

    static void CreateNodes ();
    static void Callback ();
    static void CreateDevices ();
    static void InstallInternetStack ();
    static void InstallApplications ();
    static void ConfigureBaseStations ();
    static void performRound(int round,int round_duration, int round_start, int round_end);
    static void FormClusters(int round,int round_duration, int round_start, int round_end);
    static void SendToMasterOfEachCluster(int round,int round_duration, int round_start, int round_end);
    static void ChooseMaster(Ptr<ClusterManager> baseStationApp, int round);
	static void sampleRun(int round, int numRnds, double rnd_start, double rnd_end , double rnd_duration);
	static void setupEnv(int argc, char *argv[]);
	static void test();
};


}

#endif /* CLUSTERINVANET_H */

