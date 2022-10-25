/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *
 * Author: Sthabile Lushaba
 */

#include "aodv-packet.h"
#include "aodv-neighbor.h"
#include "aodv-dpd.h"
#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"

#include <iostream>
#include "ns3/header.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include <map>
#include "ns3/nstime.h"
#include "aodv-stored-rrep.h"
#include "ns3/pointer.h"

namespace ns3 {
namespace aodv {

    Ptr<Packet> StoredRrep::getPacket(){
      return savedPacket;
    }
    
    Ipv4Address StoredRrep::getReceiver(){
      return receiver;
    }
    Ipv4Address StoredRrep::getSender(){
      return sender;
    }

    Ptr<NetDevice> StoredRrep::getDev(){
      return dev;
    }

    Ipv4InterfaceAddress StoredRrep::getInterfaceAddress(){
      return interfaceAddress;
    }

    uint32_t StoredRrep::getDstSeqNo(){
      return dstSeqNo;
    }

    Ipv4Address StoredRrep::getOrigin()
    {
      return rrepOrigin;
    }
    Ipv4Address StoredRrep::getDst(){
      return rrepDst;
    }

    void StoredRrep::setReceiver(Ipv4Address r){
      receiver = r;
    }

    void StoredRrep::setSender(Ipv4Address s){
      sender = s;
    }

    void StoredRrep::setPacket(Ptr<Packet> p){
      savedPacket = p;
    }

    void StoredRrep::setDevice(Ptr<NetDevice> d){
      dev = d;
    }

    void StoredRrep::setinterfaceAddress(Ipv4InterfaceAddress intfAddrss){
      interfaceAddress = intfAddrss;
    }

    void StoredRrep::setDstSeqNo(uint32_t d){
      dstSeqNo = d;
    }

    void StoredRrep::setDst(Ipv4Address dstn){
      rrepDst = dstn;
    }

    void StoredRrep::setOrigin(Ipv4Address origin)
    {
      rrepOrigin = origin;
    }

  }
}

