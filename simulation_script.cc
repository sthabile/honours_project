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

#include "ns3/netanim-module.h"

using namespace ns3;
using namespace std;

/*
  *script simulates a network topology where nodes are randomly positioned.
  *Choosing node 2 as source to ensure enough space between the source and 
   the destination to demostrate the necesary routing concepts.
*/
NS_LOG_COMPONENT_DEFINE ("Simulating AODV_PURE");

/**
 * This function basically prints out the packet received from a particular socket.
 * */
void ReceivePacket(Ptr<Socket> socket)
{
   Ptr<Packet> packet;
   Address senderAddress;
   while(packet == socket -> RecvFrom(senderAddress))
   {
     cout<< "Hypothetically printing the contents of the packet";
   }
}
/**
 * This function essentially opens a socket and binds it to and address.
 * This allows the a device to comminicate with other devices on the application
 * level.
 */
Ptr<Socket> SetupPacketReceive(Ipv4Address addr, Ptr<Node> node, uint16_t port)
{
   NS_LOG_INFO("Setup event for packets received");

   TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
   Ptr<Socket> sink = Socket::CreateSocket(node,tid);
   InetSocketAddress local = InetSocketAddress(addr, port);
   sink-> Bind(local);
   // sink->SetRecvCallback(MakeCallback(ReceivePacket, sink));

   return sink;
}

int main(int argc, char **argv)
{
   cout<<"Simulation starting now";
   int numberOfNodes=5;
   int simulationTime = 100;
   int networkSetUpTime = simulationTime/2;
   int transmissionRange = 50; //50 m
   int node_speed = 50;
   int pause_time = 0;
   string transmissio_rate = "DsssRate11Mbps";
   int packetSize; 

//==================================== Setting up grid with mobile nodes =======================================
   NodeContainer node_container;
   node_container.Create(numberOfNodes);

   MobilityHelper node_mobility;

   //This section specifies the mobility model for the nodes.
   // First, the grid or simulation area is created through the posision allocator object
   // Then the posistion allocator is passed to the mobility model.
   // The nodes posisitions are allocated randomly within the grid.
   ObjectFactory pos;
   pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
   pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=500.0]"));
   pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=500.0]"));

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
   It specifies the physical layer and the mac link layer. The propagation delay and loss models are also specified
   Once the physical and mac layers are set, these are installed to each node and stored as network devices on the netDeviceContainer 
*/
   NetDeviceContainer network_devices;
   
   YansWifiPhyHelper phys_layer;
   phys_layer.Set("TxGain",DoubleValue(0));
   phys_layer.Set("RxGain",DoubleValue(0)); 
   phys_layer.Set("TxPowerStart", DoubleValue(10)); //Was 20
   phys_layer.Set("TxPowerEnd", DoubleValue(10));
   phys_layer.Set("TxPowerLevels", UintegerValue(1));

   YansWifiChannelHelper wifiChannel;
   wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
   wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
   phys_layer.SetChannel(wifiChannel.Create ());

   //Specifing the Mac Layer
   WifiMacHelper mac_layer;
   mac_layer.SetType ("ns3::AdhocWifiMac");

   //Specifying the connetion type. This could be wifi,bluetooth e.t.c
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
  stack.SetRoutingHelper(aodv_protocol);
  stack.Install(node_container);

  Ipv4AddressHelper addresses;
  addresses.SetBase("10.1.1.0", "255.255.255.0");
  interfaces = addresses.Assign (network_devices);

//   if (printRoutes)
//   {
//       Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodv.routes", std::ios::out);
//       aodv.PrintRoutingTableAllAt (Seconds (8), routingStream);
//   }

//==================================== Initiate a conversation between two nodes =======================================


   uint16_t port = 9;   // Discard port (RFC 863)
   OnOffHelper onoff ("ns3::UdpSocketFactory", Address());
   onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
   onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

// Setting up the destination node to handle UDP packets
   Ptr<Socket> dstn = SetupPacketReceive(interfaces.GetAddress(numberOfNodes-1),node_container.Get(numberOfNodes-1), port);
   AddressValue remoteAddress (InetSocketAddress (interfaces.GetAddress(numberOfNodes-1), port));
   onoff.SetAttribute("Remote", remoteAddress);

// Installing an OnOff application on the source node so it can send UDP packets 
   ApplicationContainer apps = onoff.Install (node_container.Get (0));
   apps.Start (Seconds(networkSetUpTime));
   apps.Stop (Seconds(simulationTime));

//==================================== Code for trace files =======================================

   // Should also be able to print the selected route.
   // Check RouteOutput() from the main file.
   AsciiTraceHelper ascii;
   phys_layer.EnableAsciiAll (ascii.CreateFileStream ("aodv_sim.tr"));
   phys_layer.EnablePcapAll ("aodv_sim", true);

   AnimationInterface anim ("pure_aodv_animation.xml"); // Mandatory
   for (int i = 0; i < numberOfNodes; i++)
   {
      if(i == 0)
      {
         string source_tag = "Source";
         // string temp_addrss = "";
         // interfaces.GetAddress(1).Print(temp_addrss);
         anim.UpdateNodeDescription (node_container.Get(0), source_tag); 
         anim.UpdateNodeColor (node_container.Get(0), 0,255, 0);  //Green
      }
      else if (i == (numberOfNodes - 1))
      {
         anim.UpdateNodeDescription (node_container.Get(numberOfNodes-1), "Destination"); 
         anim.UpdateNodeColor (node_container.Get(numberOfNodes - 1), 0,255, 0);  //Green
      }
      else
      {
         string node_tag = "N";
         anim.UpdateNodeDescription(node_container.Get(i), node_tag.append(to_string(i))); 
      }
   }

   anim.EnablePacketMetadata (); // Optional

   Simulator::Stop (Seconds(simulationTime));
   Simulator::Run ();
   Simulator::Destroy ();

   return 0;
}
