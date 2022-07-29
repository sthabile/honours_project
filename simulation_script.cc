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
   int simulationTime = 200; //200s
   int simulationArea; //100 m x 100m
   int transmissionRange = 50;//50 m
   int node_speed = 1; // 1 m/s
   int pause_time = 0;
   //Traffic type -> CBR (Constant Bit Rte)
   //Routing protocol -> AODV
   string transmissio_rate = "DsssRate11Mbps";
   int packetRate;  //11 Mbps
   int packetSize;  // 1 kb
   //MAC -> IEEE 802.11


//==================================== Setting up grid with mobile nodes =======================================
   NodeContainer node_container;
   node_container.Create(numberOfNodes);

   MobilityHelper node_mobility;

   //THis section specifes the mobility model for the nodes.
   // Firts, the grid or simulation area is created through the posision allocator object
   // Then the posistion allocator is passed to the mobility model.
   // The nodes posisitions are allocated randomly within the grid.
   ObjectFactory pos;
   pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
   pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
   pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));

   Ptr<PositionAllocator> posAlloc = pos.Create ()->GetObject<PositionAllocator> ();

   std::stringstream ssSpeed;
   ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << node_speed<< "]";
   std::stringstream ssPause;
   ssPause << "ns3::ConstantRandomVariable[Constant=" << pause_time << "]";
   node_mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Speed", StringValue (ssSpeed.str()),  //Speicifies how fast the nodes move
                                  "Pause", StringValue (ssPause.str()),  //specifies whether nodes pause before changing direcion
                                  "PositionAllocator", PointerValue (posAlloc));

   node_mobility.Install(node_container);

//==================================== Installing devices and setting up the WiFi connection =======================================
/*
   This section sets up the connection type between the nodes.
   It specifies the physical layer and the data link layer
*/
   NetDeviceContainer network_devices;

   // The following code creates a physical layer channel and
   // specifies the propagation delay as well as the propagation
   // loss models.
   YansWifiPhyHelper phys_layer;
   YansWifiChannelHelper wifiChannel;
   wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
   wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
   phys_layer.SetChannel(wifiChannel.Create ());

   //Specifing the Mac Layer
   WifiMacHelper mac_layer;
   mac_layer.SetType ("ns3::AdhocWifiMac");

   //Specifying the connetion type.  Could be wifi = bluetooth e.t.c
   WifiHelper wifi;  
   wifi.SetStandard (WIFI_STANDARD_80211b);
   wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (transmissio_rate),
                                "ControlMode",StringValue (transmissio_rate));

  network_devices = wifi.Install (phys_layer, mac_layer, node_container); 


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
   phys_layer.EnableAsciiAll (ascii.CreateFileStream ("aodv_sim.tr"));
   phys_layer.EnablePcapAll ("aodv_sim", true);

   // Ptr<FlowMonitor> flowmon;
   // FlowMonitorHelper flowmonHelper;s
   // flowmon = flowmonHelper.InstallAll ();

   Simulator::Run ();
   Simulator::Stop (Seconds (simulationTime));
   Simulator::Destroy ();

   return 0;
}
