/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Author: Sthabile Lushab
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
