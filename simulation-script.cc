/*
This c++ script simulates a mobile p2p network where nodes can leave and join the newtork at anytime

The aim is to use pure AODV routing protocol and see how the network performs before introdicing
the blackhole attack
*/

#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"

#include "ns3/yans-wifi-phy.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-net-device.h"

#include "ns3/v4ping-helper.h"

using namespace ns3;
using namespace std;

/*
  script simulates a network topology where nodes are randomly positioned.
*/
NS_LOG_COMPONENT_DEFINE ("Simulating AODV_PURE");

int main(int argc, char **argv)
{
   cout<<"Simulation starting now";
   int numberOfNodes=5; //10
   int simulationTime; //200s
   int simulationArea; //100 m x 100m
   int transmissionRange; //50 m
   int speedOfNodes; // 1 m/s
   //Traffic type -> CBR (Constant Bit Rte)
   //Routing protocol -> AODV
   int packetRate;  //11 Mbps
   int packetSize;  // 1 kb
   //MAC -> IEEE 802.11


//==================================== Setting up grid with mobile nodes =======================================
   NodeContainer node_container;
   node_container.Create(numberOfNodes);

   MobilityHelper node_mobility;
   node_mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                 "X", StringValue ("100.0"),
                                 "Y", StringValue ("100.0"),
                                 "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=30]"));
   node_mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Mode", StringValue ("Time"),
                             "Time", StringValue ("2s"),
                             "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                             "Bounds", StringValue ("0|200|0|200"));

   node_mobility.Install(node_container);

//==================================== Installing devices and connecting the nodes via WiFi =======================================
/*
   This section sets up the connection type between the nodes.
   It specifies the physical layer and the data link layer
*/
   NetDeviceContainer network_devices;

   YansWifiPhyHelper wifi_phys_layer;   
   YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
   wifi_phys_layer.SetChannel(wifiChannel.Create());

   WifiMacHelper mac_layer;
   mac_layer.SetType("ns3::AdhocWifiMac");
   // mac_layer.SetChannelAttribute ("DataRate", DataRateValue (5000000));
   // mac_layer.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

   WifiHelper wifi;  
   wifi.SetStandard (WIFI_STANDARD_80211b);
   wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));

  network_devices = wifi.Install (wifi_phys_layer, mac_layer, node_container); 

//==================================== Setting up the internet stack =======================================

  AodvHelper aodv_protocol;
  Ipv4InterfaceContainer interfaces;

  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv_protocol);
  stack.Install (node_container);

  Ipv4AddressHelper addresses;
  addresses.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = addresses.Assign (network_devices);

//   if (printRoutes)
//   {
//       Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodv.routes", std::ios::out);
//       aodv.PrintRoutingTableAllAt (Seconds (8), routingStream);
//   }

//==================================== Installing applications on the devices =======================================


//==================================== Initiate a conversation between two nodes =======================================
   V4PingHelper ping (interfaces.GetAddress (4));
   ping.SetAttribute ("Verbose", BooleanValue (true));

   ApplicationContainer p = ping.Install (node_container.Get (0));
   p.Start (Seconds (0));
   p.Stop (Seconds (simulationTime) - Seconds (0.001));

   AsciiTraceHelper ascii;
   wifi_phys_layer.EnableAsciiAll (ascii.CreateFileStream ("aodv_sim.tr"));
   wifi_phys_layer.EnablePcapAll ("aodv_sim", true);

   // Ptr<FlowMonitor> flowmon;
   // FlowMonitorHelper flowmonHelper;s
   // flowmon = flowmonHelper.InstallAll ();

   Simulator::Run ();
   Simulator::Stop (Seconds (simulationTime));
   Simulator::Destroy ();
   
   return 0;
}
