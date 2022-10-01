
/**
 * @file sim_scrpt_oo.cc
 * @author Sthabile Lushaba (sthabile.nature@gmail.com)
 * @brief This script aims to simulate a generic mobile peer-to-peer network
 * where nodes are connected via WI-FI, in an ad-hoc manner. The basic structure
 * follows the example found in examples\routig\manet-routing-compare.cc
 * Some of the functions were taken from that example script.
 * @version 0.2.1 
 * Used this version to collect the first batch of results
 * @date 2022-09-28
 * 
 * @copyright Copyright (c) 2022
 * 
 * THINK ABOUT KEEPING THE DATA RATES SAME AS THE EXAMPLE FILE
 * Might need to wait a bit longer after the last packet before stopping the simulation
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

NS_LOG_COMPONENT_DEFINE ("AodvRoutingSimulation");

class AodvRoutingSimulation
{
    public:
        /**
         * @brief Constructor 
         */
        AodvRoutingSimulation ();

        /**
         * @brief Set up all the necessary simulation parameters.
         */

        void SetUp();

        /**
         * @brief Starts the simulation
         * 
         */
        void Run ();

    private:
        /**
         * @brief Function handling received packets.
         * Configures the sink node to receive UDP packets.
         * @param addr 
         * @param node 
         * @return Ptr<Socket> 
         */
        Ptr<Socket> SetupPacketReceive (Ipv4Address addr, Ptr<Node> node);

        /**
         * @brief Helper function for collecting network data 
         * 
         * @param socket 
         */
        void ReceivePacket (Ptr<Socket> socket);

        /**
         * @brief This function checks the network performance 
         * per second. Each metric is collected and stored in 
         * an external dataset file.
         */
        void CheckThroughput ();

        /**
         * @brief Function for initialising datasets
         * used for gnuplots
         */
        void setUpDataSets();

        /**
         * @brief Function for generating gnuplots
         */
        void generateCustomGnuplots();
//============================================================================================
//                 PRIVATE VARIABLES
//============================================================================================
        uint32_t port;                   // Receiving port number.
        uint32_t bytesTotal;             // Total received bytes. (per session)
        uint32_t networkBytesTotal;      // Overall bytes recieved (entire network simulation)
        uint32_t packetsReceived;        // Total received packets.
        uint32_t networkPacketsReceived; // Overall received packets.(entire network simulation)
        uint32_t packetExchangeStartTime;


        int numberOfNodes;
        int simulationTime;
        int networkSetUpTime;
        int transmissionRange;
        int nodeSpeed;
        int pauseTime;
        int transmission_power;
        string transmissionRate;
        string packetSize;
        string protocolName;
        string CSVfileName;
        string scenario;
        string dataRate;
        bool setMalicious;

        Gnuplot gnuplot;
        Gnuplot2dDataset dataSet;

        Gnuplot gnuplot1;
        Gnuplot2dDataset dataSet1;

        Gnuplot gnuplot2;
        Gnuplot2dDataset dataSet2;
};

AodvRoutingSimulation::AodvRoutingSimulation ()
{
}

static inline std::string
PrintReceivedPacket (Ptr<Socket> socket, Ptr<Packet> packet, Address senderAddress, uint32_t bytesTotal)
{
  std::ostringstream oss;

  oss << Simulator::Now ().GetSeconds () << " " << socket->GetNode ()->GetId ();

  if (InetSocketAddress::IsMatchingType (senderAddress))
    {
      InetSocketAddress addr = InetSocketAddress::ConvertFrom (senderAddress);
      oss << " received one packet from " << addr.GetIpv4 ()<<"\n";
      oss << "Total bytes : "<< bytesTotal;
    }
  else
    {
      oss << " received one packet!";
      oss << packet->ToString();
    }
  return oss.str ();
}

void
AodvRoutingSimulation::ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address senderAddress;
  while ((packet = socket->RecvFrom (senderAddress)))
  {
      bytesTotal += packet->GetSize ();
      networkBytesTotal += packet->GetSize();
      packetsReceived += 1;
      networkPacketsReceived +=1;
      NS_LOG_UNCOND (PrintReceivedPacket (socket, packet, senderAddress, bytesTotal));
  }
}

void
AodvRoutingSimulation::CheckThroughput ()
{
  u_int32_t tempBytes = bytesTotal;

  double kiloBits = (bytesTotal * 8.0) / 1000;  //Multiply by 8 to convert to bits, then devide by 1000 to convert to kilobits
  bytesTotal = 0;

  std::ofstream out ((scenario+ CSVfileName).c_str (), std::ios::app);

  int32_t current_time = (int32_t)(Simulator::Now ()).GetSeconds ();

  out << current_time<< ","
      << packetsReceived << ","
      << kiloBits << std::endl;
  out.close ();

  dataSet.Add(current_time, (double) (kiloBits));

  std::ostringstream oss;

  oss<< "Total Bytes at "<<current_time<<": "<< tempBytes;
  oss<< "Throughput at "<<current_time<<": "<< kiloBits;

  NS_LOG_UNCOND(oss.str());

  packetsReceived = 0;
  Simulator::Schedule (Seconds(1.0), &AodvRoutingSimulation::CheckThroughput, this);
}

Ptr<Socket>
AodvRoutingSimulation::SetupPacketReceive (Ipv4Address addr, Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback (&AodvRoutingSimulation::ReceivePacket, this));

  return sink;
}

void
AodvRoutingSimulation::SetUp(){
    NS_LOG_DEBUG("Initialising all the necessary parameters");
    scenario = "pure_aodv";
    // scenario = "blackhole";
    numberOfNodes=10;
    simulationTime = 200;
    networkSetUpTime = 50;
    transmissionRange = 50;
    nodeSpeed = 20; 
    pauseTime = 0;
    transmission_power = 20; 
    transmissionRate = "DsssRate11Mbps";
    packetSize = "64";  
    protocolName = "AODV_PURE";
    CSVfileName = "_sim.csv";
    dataRate = "2048bps";  
    setMalicious = true;  

    port =  9;  
    bytesTotal = 0;     
    networkBytesTotal = 0;
    packetsReceived = 0 ; 
    networkPacketsReceived = 0;
    packetExchangeStartTime = 0;

    setUpDataSets();
}


void
AodvRoutingSimulation::setUpDataSets(){
    //For Throughput
    gnuplot.SetOutputFilename("throughput.png");
    gnuplot.SetTitle ("throughput.plt");
    gnuplot.SetTerminal("png");
    gnuplot.SetLegend("Simulation time (Seconds)", "Throughput(Kbps)");
    
    dataSet.SetTitle ("Throughput");
    dataSet.SetStyle (Gnuplot2dDataset::LINES);

    //For Packetloss
    gnuplot1.SetOutputFilename("packetloss.png");
    gnuplot1.SetTitle ("packetloss.plt");
    gnuplot1.SetTerminal("png");
    gnuplot1.SetLegend("Simulation time in seconds", "Number of packet loss");

    Gnuplot2dDataset dataset1;
    dataSet1.SetTitle ("Packetloss");
    dataSet1.SetStyle (Gnuplot2dDataset::LINES_POINTS);

    //For End-to-End delay
    gnuplot2.SetOutputFilename("delay.png");
    gnuplot2.SetTitle ("delay.plt");
    gnuplot2.SetTerminal("png");
    gnuplot2.SetLegend("Simulation time in seconds", "Average End-to-End delay for each flow(ns)");
    
    dataSet2.SetTitle ("End-to-End Delay");
    dataSet2.SetStyle (Gnuplot2dDataset::LINES_POINTS);
}

void
AodvRoutingSimulation::generateCustomGnuplots()
{
    gnuplot.AddDataset(dataSet);
    std::ofstream plotFile ("throughput.plt");
    gnuplot.GenerateOutput (plotFile);
    plotFile.close ();

    gnuplot1.AddDataset(dataSet1);
    std::ofstream plotFile1 ("packetloss.plt");
    gnuplot1.GenerateOutput (plotFile1);
    plotFile1.close ();

    gnuplot2.AddDataset(dataSet2);
    std::ofstream plotFile2 ("delay.plt");
    gnuplot2.GenerateOutput (plotFile2);
    plotFile2.close ();
}

void 
AodvRoutingSimulation::Run()
{
  NS_LOG_DEBUG("Starting the run() method");
  std::ostringstream oss;

//============================================================================================
//                 CREATING NODES
//============================================================================================

  NodeContainer nodeContainer;
  NodeContainer maliciousNodes;
  NodeContainer nonMaliciousNodes;

  nodeContainer.Create(numberOfNodes);

  if(setMalicious){
    oss<< "Setting up 3 malicious nodes";
    NS_LOG_UNCOND(oss.str());
    for(int i =0; i< numberOfNodes; i++) 
    {
      if(i != 8 && i != numberOfNodes/2 && i != numberOfNodes/4) 
      {   
        NS_LOG_UNCOND("Adding a non malicious node");
        nonMaliciousNodes.Add(nodeContainer.Get(i));
      }
      else
      {
        NS_LOG_UNCOND("Adding a malicious node");
        maliciousNodes.Add(nodeContainer.Get(i));
      }
    }
  }

//============================================================================================
//                 NODE POSITIONS AND MOBILITY SPECIFICATIONS
//============================================================================================

  NS_LOG_LOGIC("Specifying node mobility");
  MobilityHelper nodeMobility;
  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));

  Ptr<PositionAllocator> posAlloc = pos.Create ()->GetObject<PositionAllocator> ();

  std::stringstream ssSpeed;
  ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed<< "]";
  std::stringstream ssPause;
  ssPause << "ns3::ConstantRandomVariable[Constant=" << pauseTime << "]";
  nodeMobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Speed", StringValue (ssSpeed.str()),
                                  "Pause", StringValue (ssPause.str()),
                                  "PositionAllocator", PointerValue (posAlloc));

  nodeMobility.Install(nodeContainer);

//============================================================================================
//                 INSTALLING DEVICES ON EACH NODE AND 
//                 SETTING UP COMMUNICATION MEDIUM
//============================================================================================
  NS_LOG_LOGIC("Signal strength specifications");
  NetDeviceContainer network_devices;

  YansWifiPhyHelper phys_layer;
  phys_layer.Set("TxPowerStart", DoubleValue(transmission_power));
  phys_layer.Set("TxPowerEnd", DoubleValue(transmission_power));
  phys_layer.Set("TxPowerLevels", UintegerValue(1));

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  phys_layer.SetChannel(wifiChannel.Create ());

  WifiMacHelper mac_layer;
  mac_layer.SetType ("ns3::AdhocWifiMac");

  WifiHelper wifi;  
  wifi.SetStandard (WIFI_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode",StringValue (transmissionRate),
                                  "ControlMode",StringValue (transmissionRate));

  network_devices = wifi.Install (phys_layer, mac_layer, nodeContainer); 

//============================================================================================
//                 ADDING THE INTERNET STACK
//============================================================================================
  NS_LOG_LOGIC("Specifying the routing protocol");
  AodvHelper nonMaliciousAodvProtocol;
  AodvHelper maliciousAodvProtocol;

  maliciousAodvProtocol.Set("IsMalicious",BooleanValue(true));  //Turn on malicious behaviour for the rest of the nodes

  Ipv4InterfaceContainer interfaces;
  InternetStackHelper stack;

  if(setMalicious){
    stack.SetRoutingHelper(nonMaliciousAodvProtocol);
    stack.Install(nonMaliciousNodes);

    stack.SetRoutingHelper(maliciousAodvProtocol);
    stack.Install(maliciousNodes);
  }
  else
  {
    stack.SetRoutingHelper(nonMaliciousAodvProtocol);
    stack.Install(nodeContainer);
  }

  Ipv4AddressHelper addresses;
  addresses.SetBase("10.1.1.0", "255.255.255.0");
  interfaces = addresses.Assign (network_devices);

  Packet::EnablePrinting();


//============================================================================================
//             SETTING UP THE SOURCE AND SINK NODES FOR DATA COMMUCATION
//============================================================================================
/**
 * @brief 
 * Setting up the destination node to handle UDP packets
 * Installing an application at the 2 source and 2 sink nodes
 * For 2 connections
 */
  OnOffHelper onoff ("ns3::UdpSocketFactory", Address());
  onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
  onoff.SetAttribute ("PacketSize", StringValue(packetSize));
  onoff.SetAttribute ("DataRate", StringValue(dataRate));


  // Source-Sink pair 1
  Ptr<Socket> dstn1 = SetupPacketReceive(interfaces.GetAddress(9),nodeContainer.Get(9));
  AddressValue remoteAddress1 (InetSocketAddress (interfaces.GetAddress(9), port)); 
  onoff.SetAttribute("Remote", remoteAddress1);

  ApplicationContainer apps1 = onoff.Install(nodeContainer.Get(0)); 
  apps1.Start(Seconds(networkSetUpTime)); 
  apps1.Stop(Seconds(simulationTime));

  // Source-Sink pair 2
  Ptr<Socket> dstn2 = SetupPacketReceive(interfaces.GetAddress(numberOfNodes-2),nodeContainer.Get(numberOfNodes-2));
  AddressValue remoteAddress2 (InetSocketAddress (interfaces.GetAddress(numberOfNodes-2), port));
  onoff.SetAttribute("Remote", remoteAddress2);

  ApplicationContainer apps2 = onoff.Install(nodeContainer.Get(3));
  apps2.Start(Seconds(networkSetUpTime));
  apps2.Stop(Seconds(simulationTime));


//============================================================================================
//             VISUALISATION OF NODES WITH NETANIM
//============================================================================================
/**
 * @brief Using different colors to distinguish the nodes.
 * GREEN - Source and destination nodes
 * RED - Normal nodes
 * BLACK - Blackhole nodes
 */
  AnimationInterface anim (scenario +"_animation.xml");
  for (int i = 0; i < numberOfNodes; i++)
  {
      if(i == 0)
      {
          string source_tag = "Source";
          anim.UpdateNodeDescription (nodeContainer.Get(0), source_tag); 
          anim.UpdateNodeColor (nodeContainer.Get(0), 0,255, 0);  //Green  
      }
      else if (i == 9 )
      {
          anim.UpdateNodeDescription (nodeContainer.Get(9), "Destination"); 
          anim.UpdateNodeColor (nodeContainer.Get(9), 0,255, 0);  //Green
      }
      else if (setMalicious)
      {
        if (i == 8 || i==numberOfNodes/2 || i != numberOfNodes/4)
        {
          anim.UpdateNodeDescription (nodeContainer.Get(i), "Malicious"); 
          anim.UpdateNodeColor (nodeContainer.Get(i), 0,0, 0);  //black
        }
      }
      else
      {
          string node_tag = "N";
          anim.UpdateNodeDescription(nodeContainer.Get(i), node_tag.append(to_string(i))); 
      }
  }
  anim.EnablePacketMetadata (); // Optional

//============================================================================================
//             SETTING UP FLOWMONITOR FOR DATA COLLECTION
//============================================================================================
  FlowMonitorHelper fmHelper;
  Ptr<FlowMonitor> flwMon = fmHelper.InstallAll();

  CheckThroughput();

//============================================================================================
//             SETTING UP SIMULATION START AND STOP SETTINGS
//============================================================================================
  Simulator::Stop(Seconds(simulationTime + networkSetUpTime));
  Simulator::Run();



//============================================================================================
//             FLOWMONITOR DATA COLLECTION
//============================================================================================
  flwMon -> CheckForLostPackets();
  flwMon -> SerializeToXmlFile("pure_aodvflow.xml",true,false);

  std::cout<<"=============== Network Performance for "<< numberOfNodes<< " ==============="<<"\n";
  std::cout<<"Backhole attack simulated : "<< setMalicious<< "\n";
  std::cout<<"Total received packets : "<< networkPacketsReceived<<"\n";
  std::cout<<"Total received bytes : "<< networkBytesTotal<<"\n";
  std::cout<<"Avg Throughput : "<< networkBytesTotal * 8 /(double)Simulator::Now().GetSeconds() - networkSetUpTime<<"\n";
  std::cout<<"Final simulation time : "<< Simulator::Now().GetSeconds()<<"\n";

  generateCustomGnuplots();

  Simulator::Destroy ();
}

int main(int argc, char *argv[])
{
    AodvRoutingSimulation simulation;

    simulation.SetUp();

    simulation.Run();

    return 0;
}