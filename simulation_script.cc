
/**
 * @file simulation_script.cc
 * @author Sthabile Lushaba (sthabile.nature@gmail.com)
 * @brief This script aims to simulate a generic mobile peer-to-peer network
 * where nodes are connected via WI-FI, in an ad-hoc manner. The basic structure
 * follows the example found in examples\routig\manet-routing-compare.cc
 * Some of the functions were taken from that example script.
 * @version 1.0.0 
 * @date 2022-10-24
 * 
 * @copyright Copyright (c) 2022
 * 
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
        void CheckPerformance ();

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
        uint32_t bytesPerSec;             // Total received bytes. (per session)
        uint32_t networkBytesTotal;      // Overall bytes recieved (entire network simulation)
        uint32_t packetsReceivedPerSec;  // Total received packets.
        uint32_t networkPacketsReceived; // Overall received packets.(entire network simulation)
        double  packetExchangeStartTime;
        uint32_t packetlossPerSecond;
        uint32_t delayPerSecond;

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
        bool enableDetection;

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

/**
 * @brief See based on the PrintReceivedPacket() from manet-routing-compare
 */

static inline std::string
PrintReceivedPacket (Ptr<Socket> socket, Ptr<Packet> packet, Address senderAddress, uint32_t bytesPerSec)
{
  std::ostringstream oss;

  oss << Simulator::Now ().GetSeconds () << " " << socket->GetNode ()->GetId ();

  if (InetSocketAddress::IsMatchingType (senderAddress))
    {
      InetSocketAddress addr = InetSocketAddress::ConvertFrom (senderAddress);
      oss << " received one packet from " << addr.GetIpv4 ()<<"\n";
      oss << "Total bytes : "<< bytesPerSec;
    }
  else
    {
      oss << " received one packet!";
      oss << packet->ToString();
    }
  return oss.str ();
}

/**
 * @brief socket->RecvFrom() returns a single packet from the socket
 * and also retreive the sender address.
 * Based on the ReceivePacket from manet-routing-compare
 */
void
AodvRoutingSimulation::ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address senderAddress;
  packet = socket->RecvFrom (senderAddress);
  
  bytesPerSec += packet->GetSize ();
  networkBytesTotal += packet->GetSize();
  packetsReceivedPerSec += 1;
  networkPacketsReceived +=1;

  if(networkPacketsReceived == 1) //first packet
  {
    SeqTsSizeHeader hd;
    packet -> PeekHeader(hd);
    packetExchangeStartTime = (double)hd.GetTs().GetSeconds();
  }
  NS_LOG_UNCOND (PrintReceivedPacket (socket, packet, senderAddress, bytesPerSec)); 

}

void
AodvRoutingSimulation::CheckPerformance ()
{
  u_int32_t tempBytes = bytesPerSec;

  double kiloBits = (tempBytes * 8.0) / 1000;  //Multiply by 8 to convert to bits, then devide by 1000 to convert to kilobits
  bytesPerSec = 0;

  std::ofstream out ((scenario+ CSVfileName).c_str (), std::ios::app);

  int32_t current_time = (int32_t)(Simulator::Now ()).GetSeconds ();

  out << current_time<< ","
      << packetsReceivedPerSec << ","
      << kiloBits/10 << std::endl;  //killobits per second
  out.close ();

  dataSet.Add(current_time, (double) (kiloBits));

  packetsReceivedPerSec = 0;
  Simulator::Schedule (Seconds(10.0), &AodvRoutingSimulation::CheckPerformance, this);
}

/**
 * @brief See based on the SetupPacketReceive() from manet-routing-compare
 */
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
    scenario = "20-blackhole_performance over time";
    // scenario = "blackhole";
    numberOfNodes= 20;
    simulationTime = 200;
    networkSetUpTime = 50;
    transmissionRange = 20;
    nodeSpeed = 25; 
    pauseTime = 0;
    transmission_power = 20; 
    transmissionRate = "DsssRate11Mbps";
    packetSize = "64";  
    protocolName = "AODV_PURE";
    CSVfileName = "_sim.csv";
    dataRate = "2048bps";  
    setMalicious = false;
    enableDetection = false;  

    port =  9;  
    bytesPerSec = 0;     
    networkBytesTotal = 0;
    packetsReceivedPerSec = 0 ; 
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
    dataSet.SetStyle (Gnuplot2dDataset::LINES_POINTS);

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

  NodeContainer allNodeContainer;
  NodeContainer maliciousNodes;
  NodeContainer nonMaliciousNodes;

  allNodeContainer.Create(numberOfNodes);

  if(setMalicious){
    for(int i =0; i< numberOfNodes; i++) 
    {
      if(i != 8 && i != numberOfNodes/2 && i != numberOfNodes/4) 
      {   
        // NS_LOG_UNCOND("Adding a non malicious node");
        nonMaliciousNodes.Add(allNodeContainer.Get(i));
      }
      else
      {
        // NS_LOG_UNCOND("Adding a malicious node");
        maliciousNodes.Add(allNodeContainer.Get(i));
      }
    }
  }

//============================================================================================
//                 NODE POSITIONS AND MOBILITY SPECIFICATIONS
//============================================================================================

  NS_LOG_LOGIC("Specifying node mobility");
  MobilityHelper nodeMobility;
  ObjectFactory positionAllocatorFactory;
  positionAllocatorFactory.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  positionAllocatorFactory.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));
  positionAllocatorFactory.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));

  Ptr<PositionAllocator> posAlloc = positionAllocatorFactory.Create ()->GetObject<PositionAllocator> ();
// 
  std::stringstream selectedSpeed;
  selectedSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed<< "]";
  std::stringstream selectedPauseTime;
  selectedPauseTime << "ns3::ConstantRandomVariable[Constant=" << pauseTime << "]";
  nodeMobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Speed", StringValue (selectedSpeed.str()),
                                  "Pause", StringValue (selectedPauseTime.str()),
                                  "PositionAllocator", PointerValue (posAlloc));

  nodeMobility.Install(allNodeContainer);

//============================================================================================
//                 INSTALLING DEVICES ON EACH NODE AND 
//                 SETTING UP COMMUNICATION MEDIUM
//============================================================================================
  NS_LOG_LOGIC("Signal strength specifications");
  NetDeviceContainer networkDevices;

  YansWifiPhyHelper physLayer;
  physLayer.Set("TxPowerStart", DoubleValue(transmission_power));
  physLayer.Set("TxPowerEnd", DoubleValue(transmission_power));
  physLayer.Set("TxPowerLevels", UintegerValue(1));

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  physLayer.SetChannel(wifiChannel.Create ());

  WifiMacHelper macLayer;
  macLayer.SetType ("ns3::AdhocWifiMac");

  WifiHelper wifi;  
  wifi.SetStandard (WIFI_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode",StringValue (transmissionRate),
                                  "ControlMode",StringValue (transmissionRate));

  networkDevices = wifi.Install (physLayer, macLayer, allNodeContainer); 

//============================================================================================
//                 ADDING THE INTERNET STACK
//============================================================================================
  NS_LOG_LOGIC("Specifying the routing protocol");
  AodvHelper nonMaliciousAodvProtocol;
  AodvHelper maliciousAodvProtocol;

  maliciousAodvProtocol.Set("IsMalicious",BooleanValue(true));  //Turn on malicious behaviour for the rest of the nodes
  if(enableDetection){
    nonMaliciousAodvProtocol.Set("enableBlackholeAttackDetection", BooleanValue(true));      
  }

  Ipv4InterfaceContainer nodeInterfaces;
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
    stack.Install(allNodeContainer);
  }

  Ipv4AddressHelper nodeAddresses;
  nodeAddresses.SetBase("10.1.1.0", "255.255.255.0");
  nodeInterfaces = nodeAddresses.Assign (networkDevices);

  Packet::EnablePrinting();

  Ptr<OutputStreamWrapper> routingOutputStream = Create<OutputStreamWrapper> ("aodv.routes", std::ios::out);
  nonMaliciousAodvProtocol.PrintRoutingTableAllAt (Seconds (8), routingOutputStream);

  if(setMalicious){
    maliciousAodvProtocol.PrintRoutingTableAllAt (Seconds (8), routingOutputStream);
  }

//============================================================================================
//             SETTING UP THE SOURCE AND SINK NODES FOR DATA COMMUNICATION
//============================================================================================
/**
 * @brief 
 * Setting up the destination node to handle UDP packets
 * Installing an application at the 2 source and 2 sink nodes
 * For 2 connections
 */
  OnOffHelper onoffAppHelper ("ns3::UdpSocketFactory", Address());
  onoffAppHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  onoffAppHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
  onoffAppHelper.SetAttribute ("PacketSize", StringValue(packetSize));
  onoffAppHelper.SetAttribute ("DataRate", StringValue(dataRate));

  // Source-Sink pair 1s
  Ptr<Socket> firstSinkNode = SetupPacketReceive(nodeInterfaces.GetAddress(9),allNodeContainer.Get(9));
  AddressValue firstRemoteAddress (InetSocketAddress (nodeInterfaces.GetAddress(9), port)); 
  onoffAppHelper.SetAttribute("Remote", firstRemoteAddress);

  ApplicationContainer firstSourceApplication = onoffAppHelper.Install(allNodeContainer.Get(0)); 
  firstSourceApplication.Start(Seconds(networkSetUpTime)); 
  firstSourceApplication.Stop(Seconds(simulationTime));

  // Source-Sink pair 2
  Ptr<Socket>secondSinkNode = SetupPacketReceive(nodeInterfaces.GetAddress(numberOfNodes-2),allNodeContainer.Get(numberOfNodes-2));
  AddressValue secondRemoteAddress (InetSocketAddress (nodeInterfaces.GetAddress(numberOfNodes-2), port));
  onoffAppHelper.SetAttribute("Remote", secondRemoteAddress);

  ApplicationContainer secondSourceApplication = onoffAppHelper.Install(allNodeContainer.Get(3));
  secondSourceApplication.Start(Seconds(networkSetUpTime));
  secondSourceApplication.Stop(Seconds(simulationTime));

//===========================================================================================
//                PRINTING PCAP FILES
//===========================================================================================
  AsciiTraceHelper asciHelper;
  physLayer.EnablePcapAll("aodv_node_pcap", true);

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
      if(i == 0 || i == 3)
      {
          string source_tag = "Source:N";
          anim.UpdateNodeDescription (allNodeContainer.Get(i), source_tag.append(to_string(i))); 
          anim.UpdateNodeColor (allNodeContainer.Get(i), 0,255, 0);  //Green  
      }
      else if (i == 9 || i == numberOfNodes-2)
      {
          string source_tag = "Destination:N";
          anim.UpdateNodeDescription (allNodeContainer.Get(i), source_tag.append(to_string(i))); 
          anim.UpdateNodeColor (allNodeContainer.Get(i), 0,255, 0);  //Green
      }
      else
      {
          if(setMalicious)
          {
            if (i == 8 || i==numberOfNodes/2 || i == numberOfNodes/4){
              anim.UpdateNodeDescription (allNodeContainer.Get(i), "Malicious"); 
              anim.UpdateNodeColor (allNodeContainer.Get(i), 0,0, 0);  //black
            }
            else
            {
              string node_tag = "N";
              anim.UpdateNodeDescription(allNodeContainer.Get(i), node_tag.append(to_string(i)));              
            }
          }
          else
          {
              string node_tag = "N";
              anim.UpdateNodeDescription(allNodeContainer.Get(i), node_tag.append(to_string(i))); 
          }

      }
  }
  anim.EnablePacketMetadata (); // Optional

//============================================================================================
//             SETTING UP FLOWMONITOR FOR DATA COLLECTION
//============================================================================================
  FlowMonitorHelper fmHelper;
  Ptr<FlowMonitor> flwMon = fmHelper.InstallAll();

  CheckPerformance();

//============================================================================================
//             SETTING UP SIMULATION START AND STOP SETTINGS
//============================================================================================
  Simulator::Stop(Seconds(simulationTime + 0.3));
  Simulator::Run();

//============================================================================================
//             FLOWMONITOR DATA COLLECTION
//============================================================================================
  flwMon -> CheckForLostPackets();
  flwMon -> SerializeToXmlFile(scenario +"flow.xml",true,false);

  std::cout<<"=============== Network Performance for "<< numberOfNodes<< " ==============="<<"\n";
  std::cout<<"Backhole attack simulated : "<< setMalicious<< "\n";
  std::cout<<"Detection mode : "<< enableDetection<< "\n";
  std::cout<<"Total received packets : "<< networkPacketsReceived<<"\n";
  std::cout<<"Total received bytes : "<< networkBytesTotal<<"\n";
  std::cout<<"Application start time : "<< networkSetUpTime<<"\n";
  std::cout<<"Final simulation time : "<< Simulator::Now().GetSeconds()<<"\n";
  std::cout<<"Avg Throughput : "<< (networkBytesTotal * 8 )/((double)(Simulator::Now().GetSeconds()) - networkSetUpTime)<<"kbps\n";

  double sentPackets = 0;
  double receiverPackets = 0;
  double lostPackets = 0;
  double delayAvg = 0;
  double throughputAvg = 0;

  std::map<FlowId, FlowMonitor::FlowStats> stats = flwMon->GetFlowStats ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmHelper.GetClassifier ());

  int32_t numOfFlows = 0;

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
  {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      
      sentPackets = sentPackets + (i -> second.txPackets);
      receiverPackets = receiverPackets + (i -> second.rxPackets);
      lostPackets = lostPackets + (i -> second.txPackets - i ->second.rxPackets);
      delayAvg = delayAvg + (i -> second.delaySum.GetSeconds());

      numOfFlows += 1;
  }

  std::cout<<"========== Flow Monitor ========="<<"\n";
  std::cout <<"Packet Delivery Ratio: "<< (receiverPackets * 100 )/sentPackets<<"%\n";
  std::cout <<"Average delay: "<< delayAvg<<" sec"<<std::endl;

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
