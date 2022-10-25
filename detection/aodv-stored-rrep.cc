/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Based on
 *      NS-2 AODV model developed by the CMU/MONARCH group and optimized and
 *      tuned by Samir Das and Mahesh Marina, University of Cincinnati;
 *
 *      AODV-UU implementation by Erik Nordström of Uppsala University
 *      http://core.it.uu.se/core/index.php/AODV-UU
 *
 * Authors: Elena Buchatskaia <borovkovaes@iitp.ru>
 *          Pavel Boyko <boyko@iitp.ru>
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

