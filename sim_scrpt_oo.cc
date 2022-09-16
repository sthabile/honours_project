
/**
 * @file sim_scrpt_oo.cc
 * @author Sthabile Lushaba (sthabile.nature@gmail.com)
 * @brief This script aims to simulate a generic mobile peer-to-peer network
 * where nodes are connected via WI-FI, in an ad-hoc manner. The basic structure
 * follows the example found in examples\routig\manet-routing-compare.cc
 * Some of the functions were taken from that example script.
 * @version 0.1.2 
 * Used this version to collect the first batch of results
 * @date 2022-09-08
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

NS_LOG_COMPONENT_DEFINE ("PURE AODV ROUTING");

class PureAodvRouting
{
    public:
        /**
         * Constructor
         */
        PureAodvRouting ();
        /**
         * Set up all the necessary parameters.
         */
        void SetUp();
        /**
         * Run the experiment.
         */
        void Run ();
    private:
        Ptr<Socket> SetupPacketReceive (Ipv4Address addr, Ptr<Node> node);

        void ReceivePacket (Ptr<Socket> socket);

        void CheckThroughput ();

        uint32_t port;            //!< Receiving port number.
        uint32_t bytesTotal;      //!< Total received bytes.
        uint32_t packetsReceived; //!< Total received packets.

        int numberOfNodes;
        int simulationTime;
        int networkSetUpTime;
        int transmissionRange;
        int nodeSpeed;
        int pauseTime;
        string transmissionRate;
        string packetSize;
        string protocolName;
        string CSVfileName;
        string dataRate;
};

/**
 * Basic Constructor
 */
PureAodvRouting::PureAodvRouting ()
{
}

static void
ThroughputMonitor(FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> monitor, Gnuplot2dDataset DataSet, Gnuplot2dDataset DataSet1, Gnuplot2dDataset DataSet2)
{
  double Throughput = 0;
  double packetloss = 0;
  double delay = 0;

  std::cout<< "========================= COLLECTING NETWORK PERFORMANCE WITH FLOWMONITOR 10 ========================\n";

  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier ());

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if(t.sourceAddress==Ipv4Address("10.1.1.1") && t.destinationAddress==Ipv4Address("10.1.1.10"))  //MUST BE FOR THE SPECIFIC NODES
      {
        std::cout << "Flow ID:    " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        std::cout << "Transmitted Bytes:   " << i->second.txBytes << "\n";        
        std::cout << "Recieved Bytes:   " << i->second.rxBytes << "\n";
        std::cout << "Transmitted Packets:   " << i->second.rxPackets << "\n";
        std::cout << "Recieved Packets:   " << i->second.rxPackets << "\n";
        std::cout << "Recieved Packets:   " << i->second.rxPackets << "\n";
        std::cout <<"Duration   : "<<(i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds())<<std::endl;
        std::cout <<"Last Received Packet : "<< i->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<std::endl;
        std::cout <<"Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1000000<< " Mbps\n";
        std::cout <<"Average delay: "<< (i->second.delaySum.GetSeconds()/i->second.rxPackets)<<std::endl;
        std::cout <<"Packet drop: "<<i->second.lostPackets<<"\n";
        std::cout<<"---------------------------------------------------------------------------"<<std::endl;
        
        Throughput = (i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds())) /1000000; //Convert to Mbps
        packetloss = (i->second.lostPackets);  //Packets lost during this flow
        delay = (i->second.delaySum.GetDouble()/i->second.rxPackets); //Average delay
        //update gnuplot file data
        DataSet.Add((double)Simulator::Now().GetSeconds(), (double) Throughput);
        DataSet1.Add((double)Simulator::Now().GetSeconds(), (double) packetloss);
        DataSet2.Add((double)Simulator::Now().GetSeconds(), (double) delay);
      }
    }

    Simulator::Schedule(Seconds(1), &ThroughputMonitor,fmhelper,monitor,DataSet,DataSet1,DataSet2);
}

static inline std::string
PrintReceivedPacket (Ptr<Socket> socket, Ptr<Packet> packet, Address senderAddress)
{
  std::ostringstream oss;

  oss << Simulator::Now ().GetSeconds () << " " << socket->GetNode ()->GetId ();

  if (InetSocketAddress::IsMatchingType (senderAddress))
    {
      InetSocketAddress addr = InetSocketAddress::ConvertFrom (senderAddress);
      oss << " received one packet from " << addr.GetIpv4 ();
      oss << packet->ToString();
    }
  else
    {
      oss << " received one packet!";
      oss << packet->ToString();
    }
  return oss.str ();
}

void
PureAodvRouting::ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address senderAddress;
  while ((packet = socket->RecvFrom (senderAddress)))
    {
      bytesTotal += packet->GetSize ();
      packetsReceived += 1;
      NS_LOG_UNCOND (PrintReceivedPacket (socket, packet, senderAddress));
    }
}

void
PureAodvRouting::CheckThroughput ()
{
  double kbs = (bytesTotal * 8.0) / 1000;  //killobits
  bytesTotal = 0;

  std::ofstream out (CSVfileName.c_str (), std::ios::app);

  out << (Simulator::Now ()).GetSeconds () << ","
      << kbs << std::endl;

  out.close ();
  packetsReceived = 0;
  Simulator::Schedule (Seconds (1.0), &PureAodvRouting::CheckThroughput, this);
}

Ptr<Socket>
PureAodvRouting::SetupPacketReceive (Ipv4Address addr, Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback (&PureAodvRouting::ReceivePacket, this));

  return sink;
}

void
PureAodvRouting::SetUp(/*int argc, char *argv[]*/)
{

  // CommandLine cmd (__FILE__);
  // cmd.AddValue("numberOfNodes","THe number of nodes in the network", numberOfNodes);
  // cmd.Parse (argc, argv);
  numberOfNodes=10;
  simulationTime = 200;
  networkSetUpTime = simulationTime/2;
  transmissionRange = 50;
  nodeSpeed = 50;
  pauseTime = 0;
  transmissionRate = "DsssRate11Mbps";
  packetSize = "521";  //Following the work of Ndajah et.al
  protocolName = "AODV_PURE";
  CSVfileName = "pure_aodv_sim.csv";
  dataRate = "2048bps";  //Not
}


void 
PureAodvRouting::Run()
{
    NodeContainer nodeContainer;
    nodeContainer.Create(numberOfNodes);

// Position nodes on the simulation area
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

// Install devices and setup Wi-Fi connection
    NetDeviceContainer network_devices;
   
    YansWifiPhyHelper phys_layer;
    phys_layer.Set("TxGain",DoubleValue(0));
    phys_layer.Set("RxGain",DoubleValue(0)); 
    phys_layer.Set("TxPowerStart", DoubleValue(20));
    phys_layer.Set("TxPowerEnd", DoubleValue(20));
    phys_layer.Set("TxPowerLevels", UintegerValue(1));

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
    phys_layer.SetChannel(wifiChannel.Create ());

    WifiMacHelper mac_layer;
    mac_layer.SetType ("ns3::AdhocWifiMac");

    WifiHelper wifi;  
    wifi.SetStandard (WIFI_STANDARD_80211b);
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                    "DataMode",StringValue (transmissionRate),
                                    "ControlMode",StringValue (transmissionRate));

    network_devices = wifi.Install (phys_layer, mac_layer, nodeContainer); 


// Specify the internet stack
    AodvHelper aodv_protocol;
    Ipv4InterfaceContainer interfaces;

    InternetStackHelper stack;
    stack.SetRoutingHelper(aodv_protocol);
    stack.Install(nodeContainer);

    Ipv4AddressHelper addresses;
    addresses.SetBase("10.1.1.0", "255.255.255.0");
    interfaces = addresses.Assign (network_devices);

    Packet::EnablePrinting();

    OnOffHelper onoff ("ns3::UdpSocketFactory", Address());
    onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
    onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
    // onoff.SetAttribute ("PacketSize", StringValue(packe));
    // onoff.SetAttribute ("DataRate", StringValue(dataRate));

// Setting up the destination node to handle UDP packets
    Ptr<Socket> dstn = SetupPacketReceive(interfaces.GetAddress(9),nodeContainer.Get(9));
    AddressValue remoteAddress (InetSocketAddress (interfaces.GetAddress(9), port)); //Node 9 as the destination
    onoff.SetAttribute("Remote", remoteAddress);

// Installing an application at the source node
    ApplicationContainer apps = onoff.Install(nodeContainer.Get(0)); //Node 0 as source
    apps.Start(Seconds(1));
    apps.Stop(Seconds(simulationTime));

// Tracing using Anim
    AsciiTraceHelper ascii;
    //phys_layer.EnableAsciiAll (ascii.CreateFileStream ("pure_aodv_sim.tr"));
    //phys_layer.EnablePcapAll ("pure_aodv_sim", true);

    AnimationInterface anim ("pure_aodv_animation.xml"); // Mandatory
    for (int i = 0; i < numberOfNodes; i++)
    {
        if(i == 0)
        {
            string source_tag = "Source";
            anim.UpdateNodeDescription (nodeContainer.Get(0), source_tag); 
            anim.UpdateNodeColor (nodeContainer.Get(0), 0,255, 0);  //Green  
        }
        else if (i == (numberOfNodes - 1))
        {
            anim.UpdateNodeDescription (nodeContainer.Get(numberOfNodes-1), "Destination"); 
            anim.UpdateNodeColor (nodeContainer.Get(9), 0,255, 0);  //Green
        }
        else
        {
            string node_tag = "N";
            anim.UpdateNodeDescription(nodeContainer.Get(i), node_tag.append(to_string(i))); 
        }
    }

    anim.EnablePacketMetadata (); // Optional

// Tracing using Flowmon
    FlowMonitorHelper fmHelper;
    Ptr<FlowMonitor> flwMon = fmHelper.InstallAll();

    //Prep for plots
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
    // gnuplot1.AppendExtra("set xrange [199: 205]");
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

    CheckThroughput();

//Start the simulation
    Simulator::Stop(Seconds(simulationTime));
    Simulator::Run();

//More Flowmon tracing
    flwMon -> CheckForLostPackets();
    flwMon -> SerializeToXmlFile("aodvflow.xml",true,false);

    ThroughputMonitor(&fmHelper,flwMon,dataset,dataset1,dataset2);

// Ploting using gnuplots
    gnuplot.AddDataset(dataset);
    std::ofstream plotFile ("throughput.plt");
    gnuplot.GenerateOutput (plotFile);
    plotFile.close ();

    gnuplot1.AddDataset(dataset1);
    std::ofstream plotFile1 ("packetloss.plt");
    gnuplot1.GenerateOutput (plotFile1);
    plotFile1.close ();

    gnuplot2.AddDataset(dataset2);
    std::ofstream plotFile2 ("delay.plt");
    gnuplot2.GenerateOutput (plotFile2);
    plotFile2.close ();

    Simulator::Destroy ();
}

int main(int argc, char *argv[])
{
    PureAodvRouting simulation;

    simulation.SetUp();

    simulation.Run();

    return 0;
}