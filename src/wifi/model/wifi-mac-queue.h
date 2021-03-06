/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005, 2009 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
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
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Mirko Banchi <mk.banchi@gmail.com>
 *          Stefano Avallone <stavallo@unina.it>
 */

#ifndef WIFI_MAC_QUEUE_H
#define WIFI_MAC_QUEUE_H

#include "ns3/queue.h"
#include "wifi-mac-header.h"

namespace ns3 {
class QosBlockedDestinations;

/**
 * \ingroup wifi
 *
 * WifiMacQueueItem stores (const) packets along with their Wifi MAC headers
 * and the time when they were enqueued.
 */
class WifiMacQueueItem : public SimpleRefCount<WifiMacQueueItem>
{
public:
  /**
   * \brief Create a Wifi MAC queue item containing a packet and a Wifi MAC header.
   * \param p the const packet included in the created item.
   * \param header the Wifi Mac header included in the created item.
   */
  WifiMacQueueItem (Ptr<const Packet> p, const WifiMacHeader & header);

  virtual ~WifiMacQueueItem ();

  /**
   * \brief Get the packet stored in this item
   * \return the packet stored in this item.
   */
  Ptr<const Packet> GetPacket (void) const;

  /**
   * \brief Get the header stored in this item
   * \return the header stored in this item.
   */
  const WifiMacHeader & GetHeader (void) const;

  /**
   * \brief Return the requested address present in the header
   * \param type the type of the address to return
   * \return the address
   */
  Mac48Address GetAddress (enum WifiMacHeader::AddressType type) const;

  /**
   * \brief Get the timestamp included in this item
   * \return the timestamp included in this item.
   */
  Time GetTimeStamp (void) const;

  /**
   * \brief Return the size of the packet included in this item
   *
   * \return the size of the packet included in this item.
   */
  uint32_t GetSize (void) const;

private:
  /**
   * \brief Default constructor
   *
   * Defined and unimplemented to avoid misuse
   */
  WifiMacQueueItem ();
  /**
   * \brief Copy constructor
   *
   * Defined and unimplemented to avoid misuse
   */
  WifiMacQueueItem (const WifiMacQueueItem &);
  /**
   * \brief Assignment operator
   *
   * Defined and unimplemented to avoid misuse
   * \returns
   */
  WifiMacQueueItem &operator = (const WifiMacQueueItem &);

  Ptr<const Packet> m_packet;  //!< The packet contained in this queue item
  WifiMacHeader m_header;      //!< Wifi MAC header associated with the packet
  Time m_tstamp;               //!< timestamp when the packet arrived at the queue
};


/**
 * \ingroup wifi
 *
 * This queue implements the timeout procedure described in
 * (Section 9.19.2.6 "Retransmit procedures" paragraph 6; IEEE 802.11-2012).
 *
 * When a packet is received by the MAC, to be sent to the PHY,
 * it is queued in the internal queue after being tagged by the
 * current time.
 *
 * When a packet is dequeued, the queue checks its timestamp
 * to verify whether or not it should be dropped. If
 * dot11EDCATableMSDULifetime has elapsed, it is dropped.
 * Otherwise, it is returned to the caller.
 *
 * WifiMacQueue could have been declared as a subclass of Queue<WifiMacQueueItem>,
 * but this would have caused python examples using wifi to crash at runtime
 * with the following error message: "Trying to allocate twice the same uid:
 * ns3::Queue<WifiMacQueueItem>". Such an error wasn't fully debugged but it
 * may have been due to static initialization order issues.
 *
 * To avoid such an error, the template class WifiQueue<Item> is introduced
 * as a subclass of Queue<Item>. WifiMacQueue is then declared as a
 * specialization of WifiQueue<Item> with Item equal to WifiMacQueueItem.
 * The methods of the WifiQueue class are left unimplemented for Item other
 * than WifiMacQueueItem.
 *
 * Modules that need to forward declare WifiMacQueue have to include the
 * following:
 *
 * \code
 *   class WifiMacQueueItem;
 *   template <typename Item> class WifiQueue;
 *   typedef WifiQueue<WifiMacQueueItem> WifiMacQueue;
 * \endcode
 *
 * in their header file.
 */
template <typename Item>
class WifiQueue : public Queue<Item>
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  WifiQueue ();
  ~WifiQueue ();

  /// drop policy
  enum DropPolicy
  {
    DROP_NEWEST,
    DROP_OLDEST
  };

  /**
   * Set the maximum delay before the packet is discarded.
   *
   * \param delay the maximum delay
   */
  void SetMaxDelay (Time delay);
  /**
   * Return the maximum delay before the packet is discarded.
   *
   * \return the maximum delay
   */
  Time GetMaxDelay (void) const;

  /**
   * Enqueue the given Wifi MAC queue item at the <i>end</i> of the queue.
   *
   * \param item the Wifi MAC queue item to be enqueued at the end
   * \return true if success, false if the packet has been dropped
   */
  bool Enqueue (Ptr<Item> item);
  /**
   * Enqueue the given Wifi MAC queue item at the <i>front</i> of the queue.
   *
   * \param item the Wifi MAC queue item to be enqueued at the front
   * \return true if success, false if the packet has been dropped
   */
  bool PushFront (Ptr<Item> item);
  /**
   * Dequeue the packet in the front of the queue.
   *
   * \return the packet
   */
  Ptr<Item> Dequeue (void);
  /**
   * Search and return, if present in the queue, the first packet having the
   * address indicated by <i>type</i> equal to <i>addr</i>, and tid
   * equal to <i>tid</i>. This method removes the packet from the queue.
   * It is typically used by ns3::EdcaTxopN in order to perform correct MSDU
   * aggregation (A-MSDU).
   *
   * \param tid the given TID
   * \param type the given address type
   * \param addr the given destination
   *
   * \return the packet
   */
  Ptr<Item> DequeueByTidAndAddress (uint8_t tid,
                                                WifiMacHeader::AddressType type,
                                                Mac48Address addr);
  /**
   * Return first available packet for transmission. A packet could be no available
   * if it is a QoS packet with a tid and an address1 fields equal to <i>tid</i> and <i>addr</i>
   * respectively that index a pending agreement in the BlockAckManager object.
   * So that packet must not be transmitted until reception of an ADDBA response frame from station
   * addressed by <i>addr</i>. This method removes the packet from queue.
   *
   * \param blockedPackets
   *
   * \return packet
   */
  Ptr<Item> DequeueFirstAvailable (const QosBlockedDestinations *blockedPackets);
  /**
   * Peek the packet in the front of the queue. The packet is not removed.
   *
   * \return the packet
   */
  Ptr<const Item> Peek (void) const;
  /**
   * Search and return, if present in the queue, the first packet having the
   * address indicated by <i>type</i> equal to <i>addr</i>, and tid
   * equal to <i>tid</i>. This method does not remove the packet from the queue.
   * It is typically used by ns3::EdcaTxopN in order to perform correct MSDU
   * aggregation (A-MSDU).
   *
   * \param tid the given TID
   * \param type the given address type
   * \param addr the given destination
   *
   * \return packet
   */
  Ptr<const Item> PeekByTidAndAddress (uint8_t tid,
                                                   WifiMacHeader::AddressType type,
                                                   Mac48Address addr);
  /**
   * Return first available packet for transmission. The packet is not removed from queue.
   *
   * \param hdr the header of the dequeued packet
   * \param tStamp
   * \param blockedPackets
   *
   * \return packet
   */
  Ptr<const Item> PeekFirstAvailable (const QosBlockedDestinations *blockedPackets);
  /**
   * Remove the packet in the front of the queue.
   *
   * \return the packet
   */
  Ptr<Item> Remove (void);
  /**
   * If exists, removes <i>packet</i> from queue and returns true. Otherwise it
   * takes no effects and return false. Deletion of the packet is
   * performed in linear time (O(n)).
   *
   * \param packet the packet to be removed
   *
   * \return true if the packet was removed, false otherwise
   */
  bool Remove (Ptr<const Packet> packet);
  /**
   * Return the number of QoS packets having tid equal to <i>tid</i> and address
   * specified by <i>type</i> equal to <i>addr</i>.
   *
   * \param tid the given TID
   * \param type the given address type
   * \param addr the given destination
   *
   * \return the number of QoS packets
   */
  uint32_t GetNPacketsByTidAndAddress (uint8_t tid,
                                       WifiMacHeader::AddressType type,
                                       Mac48Address addr);
  /**
   * This method must be used (instead of the IsEmpty method of the base class)
   * to check whether there are packets with unexpired time to live in the queue
   *
   * \return true if there are packets with unexpired time to live
   */
  bool HasPackets (void);

private:
  /**
   * Remove the item pointed to by the iterator <i>it</i> if it has been in the
   * queue for too long. If the item is removed, the iterator is updated to
   * point to the item that followed the erased one.
   *
   * \param it an iterator pointing to the item
   * \return true if the item is removed, false otherwise
   */
  bool TtlExceeded (typename Queue<Item>::ConstIterator &it);

  Time m_maxDelay;                          //!< Time to live for packets in the queue
  DropPolicy m_dropPolicy;                  //!< Drop behavior of queue
};

/// Declare WifiMacQueue as a specialization of template class WifiQueue
typedef WifiQueue<WifiMacQueueItem> WifiMacQueue;

} //namespace ns3

#endif /* WIFI_MAC_QUEUE_H */
