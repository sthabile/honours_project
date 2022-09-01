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

#include "ns3/gnuplot.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"

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

void 
ThroughputMonitor(FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> monitor, Gnuplot2dDataset DataSet, Gnuplot2dDataset DataSet1, Gnuplot2dDataset DataSet2)
{
  double Throughput = 0;
  double packetloss = 0;
  double delay = 0;

  std::cout<< "========================= Inside ThroughputMonitor sss========================";

  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier ());
  std::cout<< "========================= Just before the loop ========================";

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      std::cout << "========================== Inside the For loop ========================";
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if(t.sourceAddress==Ipv4Address("10.1.1.1")&&t.destinationAddress==Ipv4Address("10.1.1.14"))  //MUST BE FOR THE SPECIFIC NODES
      {
        std::cout<< "========================== Inside the if statement ========================";
        std::cout << "Flow ID:    " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";        
        std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
        std::cout <<"Duration   : "<<(i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds())<<std::endl;
        std::cout <<"Last Received Packet : "<< i->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<std::endl;
        std::cout <<"Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
        std::cout <<"Average delay: "<< (i->second.delaySum.GetSeconds()/i->second.rxPackets)<<std::endl;
        std::cout <<"Packet drop: "<<i->second.lostPackets<<"\n";
        std::cout<<"---------------------------------------------------------------------------"<<std::endl;
        
        Throughput = (i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds())/1024/1024  );
        packetloss = (i->second.lostPackets);
        delay = (i->second.delaySum.GetDouble()/i->second.rxPackets);
        //update gnuplot file data
        DataSet.Add((double)Simulator::Now().GetSeconds(), (double) Throughput);
        DataSet1.Add((double)Simulator::Now().GetSeconds(), (double) packetloss);
        DataSet2.Add((double)Simulator::Now().GetSeconds(), (double) delay);
      }
    }
}

int main(int argc, char **argv)
{
   cout<<"Simulation starting now";
   int numberOfNodes=5;
   int simulationTime = 100;
   int networkSetUpTime = simulationTime/2;
   int transmissionRange = 50; //50 m
   int node_speed = 25;
   int pause_time = 0;
   string transmissio_rate = "DsssRate11Mbps";
   int packetSize; 
   bool printRoutes = true;

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
   pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));
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
   It specifies the physical layer and the mac link layer. The propagation delay and loss models are also specified
   Once the physical and mac layers are set, these are installed to each node and stored as network devices on the netDeviceContainer 
*/
   NetDeviceContainer network_devices;
   
   YansWifiPhyHelper phys_layer;
   phys_layer.Set("TxGain",DoubleValue(0));
   phys_layer.Set("RxGain",DoubleValue(0)); 
   phys_layer.Set("TxPowerStart", DoubleValue(20)); //Was 20
   phys_layer.Set("TxPowerEnd", DoubleValue(20));
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

//==================================== Initiate a conversation between two nodes =======================================


   uint16_t port = 9;   // Discard port (RFC 863)
   OnOffHelper onoff ("ns3::UdpSocketFactory", Address());
   onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
   onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

  // Setting up the destination node to handle UDP packets
  for (int i =0; i<numberOfNodes;i++)
  {
    Ptr<Socket> dstn = SetupPacketReceive(interfaces.GetAddress(i),node_container.Get(i), port);
    AddressValue remoteAddress (InetSocketAddress (interfaces.GetAddress(numberOfNodes-1), port));
    onoff.SetAttribute("Remote", remoteAddress);
  }

// Installing an OnOff application on the source node so it can send UDP packets 
  ApplicationContainer apps = onoff.Install (node_container.Get (0));
  apps.Start(Time(networkSetUpTime));
  apps.Stop(Time(simulationTime-3));  //(Clean-up time)Stoping the applications a few minutes before the simulation stops allows for queued packets to be processed. 

//==================================== Code for trace files & Network performance =======================================

   //print the routing table
  if (printRoutes)
  {
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodv.routes", std::ios::out);
      aodv_protocol.PrintRoutingTableAllAt (Seconds (networkSetUpTime), routingStream);
  }

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


  //Setup for figures showing network performance
  Gnuplot gnuplot ("throughput.png");
  gnuplot.SetTitle ("throughput.plt");
  gnuplot.SetTerminal("png");
  gnuplot.SetLegend("Simulation time in seconds", "Throughput");//set labels for each axis
  Gnuplot2dDataset dataset;
  dataset.SetTitle ("Throughput");
  dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

//used for create gnuplot file to show performance figure of Packet loss

  Gnuplot gnuplot1 ("packetloss.png");
  gnuplot1.SetTitle ("packetloss.plt");
  gnuplot1.SetTerminal("png");
  gnuplot1.SetLegend("Simulation time in seconds", "Number of packet loss");//set labels for each axis
  Gnuplot2dDataset dataset1;
  dataset1.SetTitle ("Packetloss");
  dataset1.SetStyle (Gnuplot2dDataset::LINES_POINTS);

//used for create gnuplot file to show performance figure of end-to-end delay
  Gnuplot gnuplot2 ("delay.png");
  gnuplot2.SetTitle ("delay.plt");
  gnuplot2.SetTerminal("png");
  gnuplot2.SetLegend("Simulation time in seconds", "Average End-to-End delay for each flow(ns)");//set labels for each axis
  Gnuplot2dDataset dataset2;
  dataset2.SetTitle ("End-to-End Delay");
  dataset2.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  //Flowmonitor Setup 
  FlowMonitorHelper fmHelper;
  Ptr<FlowMonitor> flwMon = fmHelper.InstallAll();


   //Start the simulation
  Simulator::Stop (Seconds(simulationTime));
  Simulator::Run ();

  flwMon -> SerializeToXmlFile("aodvflow.xml",true,true);

  flwMon -> CheckForLostPackets();
  ThroughputMonitor(&fmHelper,flwMon,dataset,dataset1,dataset2);


  //  add the Throughput dataset to the plot
    gnuplot.AddDataset(dataset);
  // Open the plot file.
    // string plt_name = "throughput.plt";
    std::ofstream plotFile ("throughput.plt");
  // Write the plot file.
    gnuplot.GenerateOutput (plotFile);
  // Close the plot file.
    plotFile.close ();

  //add the Packet loss dataset to the plot
    gnuplot1.AddDataset(dataset1);
  // Open the plot file.
    std::ofstream plotFile1 ("packetloss.plt");
  // Write the plot file.
    gnuplot1.GenerateOutput (plotFile1);
  // Close the plot file.
    plotFile1.close ();

  //add the end-to-end delay dataset to the plot
    gnuplot2.AddDataset(dataset2);
  // Open the plot file.
    std::ofstream plotFile2 ("delay.plt");
  // Write the plot file.
    gnuplot2.GenerateOutput (plotFile2);
  // Close the plot file.
    plotFile2.close ();


   Simulator::Destroy ();

   return 0;
}
