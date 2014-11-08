
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

string traceFile;
string logFile;
bool verbose = false;
bool tracing = true;
TypeId tid;
Ipv4InterfaceContainer i;
NodeContainer vehicles;
NodeContainer wifiApNode ;
int    nodeNum;
double duration;
int numRounds = 4;
int numofTopics = 7;
ClusterManager *clusterMgr = ClusterManager::getInstance();	
WifiHelper wifi;
string phyMode ("OfdmRate54Mbps");
static int currentRound = 0;


int randomNumberGenerator(int numTopics)
{
		//srand (time(NULL));
		int v1 = rand() % numTopics; 
		return v1;
}

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
