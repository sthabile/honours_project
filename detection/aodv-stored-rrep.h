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

#ifndef StoredRrep_H
#define StoredRrep_H

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
#include "ns3/object.h"
#include "ns3/pointer.h"

namespace ns3 {
namespace aodv {

  /**
  * \ingroup aodv
  * \brief Collected RREP packet
  */
  class StoredRrep
  {
    public:

      Ptr<Packet> getPacket();
      Ipv4Address getReceiver();
      Ipv4Address getSender();
      Ptr<NetDevice> getDev();
      Ipv4InterfaceAddress getInterfaceAddress();
      uint32_t getDstSeqNo();
      Ipv4Address getOrigin();
      Ipv4Address getDst();


      void setReceiver(Ipv4Address r);

      void setSender(Ipv4Address s);

      void setPacket(Ptr<Packet> p);

      void setDevice(Ptr<NetDevice> d);

      void setinterfaceAddress(Ipv4InterfaceAddress intfAddrss);

      void setDstSeqNo(uint32_t dsn);

      void setOrigin(Ipv4Address origin);

      void setDst(Ipv4Address dstn);

    private:
      Ptr<Packet> savedPacket ;
      Ipv4Address receiver;
      Ipv4Address sender;
      Ptr<NetDevice> dev;
      Ipv4InterfaceAddress interfaceAddress;
      uint32_t dstSeqNo;
      Ipv4Address rrepOrigin;
      Ipv4Address rrepDst;
  };
}  // namespace aodv
}  // namespace ns3

#endif /* StoredRrep_H */
