// ./waf --run "scratch/mobTop5 --nodeNum=50 --traceFile=scratch/sc2.tcl"

#include "Final.h"
#include "ns3/clustermanager.h"
#include "ns3/clustermember.h"
#include "ns3/cluster.h"
#include "ns3/clusterinvanet.h"

//class Vanet;
int main (int argc, char *argv[])
{
	
	Vanet::setupEnv(argc,argv);
    
     std::cout << "Starting simulation for " << Vanet::duration << " s.\n";

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Simulator::Stop (Seconds (Vanet::duration + 10));

	AnimationInterface::SetConstantPosition (Vanet::nodes.Get (50), 7.6, 198.35);
	AnimationInterface anim ("vanet_animation.xml");
	anim.EnablePacketMetadata (true);
	
	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor;
	if (Vanet::enableFlowMonitor)
    {
      
		monitor = flowmon.InstallAll(); 
    }

    Simulator::Run ();
    
    Vanet::myos.close (); // close log file
 
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
         Vanet::total_lost_packets += i->second.lostPackets;
      
      //std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (duration - clientStart) / 1024 / 1024  << " Mbps\n";


  monitor->SerializeToXmlFile ("vanet_flowmon.xml", true, true);

		
    }
    
    std::cout<<"Total number of lost packets : "<<Vanet::total_lost_packets<<std::endl;
    Simulator::Destroy ();

    return 0;
}
