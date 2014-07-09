// Copyright (c) 2013 Arista Networks, Inc.  All rights reserved.
// Arista Networks, Inc. Confidential and Proprietary.

#ifndef EOS_INTF_H
#define EOS_INTF_H

#include <string>
#include <stddef.h>

#include <eos/base.h>
#include <eos/base_handler.h>
#include <eos/base_mgr.h>
#include <eos/iterator.h>

/**
 * Base interface types.
 *
 * A base interface contains non-media or interface type specific information.
 * Also included in this module is the interface ID type intf_id_t, used
 * to uniquely identify any interface in the system of any type, as well as
 * common enumerates.
 */

namespace eos {

/// The operational status of an interface
enum oper_status_t {
   INTF_OPER_NULL,
   INTF_OPER_UP,
   INTF_OPER_DOWN,
};

/// The interface's type
enum intf_type_t {
   INTF_TYPE_NULL,
   INTF_TYPE_OTHER,
   INTF_TYPE_ETH,
   INTF_TYPE_VLAN,
   INTF_TYPE_MANAGEMENT,
   INTF_TYPE_LOOPBACK,
   INTF_TYPE_LAG,
   INTF_TYPE_NULL0,
   INTF_TYPE_CPU,
};

/// Unique identifier for an interface.
class EOS_SDK_PUBLIC intf_id_t {
 public:
   /// Default constructor
   intf_id_t();
   // Copy constructor and assignment operator implicitly declared.

   /// Constructor based on an interface name, i.e. 'Ethernet3/1', or 'Management1'
   explicit intf_id_t(char const * name);
   /// Constructor based on an interface name, i.e. 'Ethernet3/1', or 'Management1'
   explicit intf_id_t(std::string const & name);

   /// Returns true if the interface is Null0
   bool is_null0() const;
   /// Returns the interface's type
   intf_type_t intf_type() const;

   /// Returns the interface name as a string, e.g., 'Ethernet3/1'
   std::string to_string() const;

   // Only the "default interface" provided by the default constructor
   // evaluates to false.
   bool operator!() const;
   bool operator==(intf_id_t const & other) const;
   bool operator!=(intf_id_t const & other) const;
   bool operator<(intf_id_t const & other) const;

 private:
   friend struct IntfIdHelper;
   uint64_t intfId_;
 public:  // Not part of the public API.
   /// Constructor based on internal id representation
   explicit intf_id_t(uint64_t) EOS_SDK_INTERNAL;
};

/**
 * Interface counter class
 * 
 * All of these attributes have the same meanings as the corresponding
 * objects in the Interface MIB (RFC 2863, "IF-MIB").
 */
class EOS_SDK_PUBLIC intf_counters_t {
 public:
   intf_counters_t();
   intf_counters_t(uint64_t out_ucast_pkts,
                   uint64_t out_multicast_pkts,
                   uint64_t out_broadcast_pkts,
                   uint64_t in_ucast_pkts,
                   uint64_t in_multicast_pkts,
                   uint64_t in_broadcast_pkts,
                   uint64_t out_octets,
                   uint64_t in_octets,
                   uint64_t out_discards,
                   uint64_t out_errors,
                   uint64_t in_discards,
                   uint64_t in_errors,
                   seconds_t sample_time);

   /** IF-MIB ifOutUcastPkts
    *
    * Note that IF-MIB specifies that ifOutUcastPkts
    * should include packets that were dropped due to excessive
    * collisions, as if they were successfully transmitted.  We count
    * these as out_errors
    */
   uint64_t out_ucast_pkts() const;
   /// IF-MIB ifOutMulticastPkts counter
   uint64_t out_multicast_pkts() const;
   /// IF-MIB ifOutBroadcastPkts counter
   uint64_t out_broadcast_pkts() const;

   /// IF-MIB ifInUcastPkts counter
   uint64_t in_ucast_pkts() const;
   /// IF-MIB ifInMulticastPkts counter
   uint64_t in_multicast_pkts() const;
   /// IF-MIB ifInBroadcastPkts counter
   uint64_t in_broadcast_pkts() const;

   /** IF-MIB ifOutOctets counter
    *
    * Note that for Ethernet interfaces, the octet counters include the MAC header
    * and FCS (but not the preamble or SFD).  This is different to the IEEE 802.3
    * counters (which do not include MAC header and FCS).  See RFC 3635.
    */
   uint64_t out_octets() const;
   /** IF-MIB ifInOctets counter
    *
    * Note that for Ethernet interfaces, the octet counters include the MAC header
    * and FCS (but not the preamble or SFD).  This is different to the IEEE 802.3
    * counters (which do not include MAC header and FCS).  See RFC 3635.
    */
   uint64_t in_octets() const;
   
   /// IF-MIB ifOutDiscards counter
   uint64_t out_discards() const;
   /// IF-MIB ifOutErrors counter
   uint64_t out_errors() const;
   /// IF-MIB ifInDiscards counter
   uint64_t in_discards() const;
   /** IF-MIB ifInErrors counter
    *
    * The IF-MIB specifies that CRC errors should not get counted at all!, and
    * that inErrors should include IP header checksum errors.  We do not do this.
    * We count CRC errors as inErrors, and IP header checksum errors as good
    *  packets at this level (in_ucast_pkts);
    */
   uint64_t in_errors() const;

   // Time when the counters were updated
   seconds_t sample_time() const;

   bool operator==(intf_counters_t const & other) const;
   bool operator!=(intf_counters_t const & other) const;
   
 private:
   uint64_t out_ucast_pkts_;
   uint64_t out_multicast_pkts_;
   uint64_t out_broadcast_pkts_;
   uint64_t in_ucast_pkts_;
   uint64_t in_multicast_pkts_;
   uint64_t in_broadcast_pkts_;
   uint64_t out_octets_;
   uint64_t in_octets_;
   uint64_t out_discards_;
   uint64_t out_errors_;
   uint64_t in_discards_;
   uint64_t in_errors_;
   seconds_t sample_time_;
};

/// Interface traffic rates class
class EOS_SDK_PUBLIC intf_traffic_rates_t {
 public:
   intf_traffic_rates_t();
   intf_traffic_rates_t(double out_pkts_rate,
                        double in_pkts_rate,
                        double out_bits_rate,
                        double in_bits_rate,
                        seconds_t sample_time);
   
   /// Output packets per second
   double out_pkts_rate() const;
   /// Input packets per second
   double in_pkts_rate() const;
   /// Output bits per second
   double out_bits_rate() const;
   /// Input bits per second
   double in_bits_rate() const;
   // Time when the rates were updated
   seconds_t sample_time() const;

   bool operator==(intf_traffic_rates_t const & other) const;
   bool operator!=(intf_traffic_rates_t const & other) const;

 private:
   double out_pkts_rate_;
   double in_pkts_rate_;
   double out_bits_rate_;
   double in_bits_rate_;
   seconds_t sample_time_;
};

class intf_mgr;

/// This class receives changes to base interface attributes.
class EOS_SDK_PUBLIC intf_handler : public base_handler<intf_mgr, intf_handler> {
 public:
   explicit intf_handler(intf_mgr *);
   intf_mgr * get_intf_mgr() const;

   /**
    * Registers this class to receive change updates on the interface.
    *
    * Expects a boolean signifying whether notifications should be
    * propagated to this instance or not.
    */
   void watch_all_intfs(bool);

  /**
    * Registers this class to receive change updates on the given interface.
    *
    * Expects the id of the corresponding interface and a boolean signifying whether
    * notifications should be propagated to this instance or not.
    */
   void watch_intf(intf_id_t, bool);

   /// Handler called when a new interface is created.
   virtual void on_intf_create(intf_id_t);
   /// Handler called when an interface has been removed.
   virtual void on_intf_delete(intf_id_t);
   /// Handler called when the operational status of an interface changes.
   virtual void on_oper_status(intf_id_t, oper_status_t);
   /// Handler called after an interface has been configured to be enabled.
   virtual void on_admin_enabled(intf_id_t, bool);
};

class intf_iter_impl;

/// An interface iterator
class EOS_SDK_PUBLIC intf_iter_t : public iter_base<intf_id_t,
                                                    intf_iter_impl> {
 private:
   friend class intf_iter_impl;
   explicit intf_iter_t(intf_iter_impl * const) EOS_SDK_PRIVATE;
};

/**
 * The interface manager.
 * This class inspects and configures base interface attributes.
 */
class EOS_SDK_PUBLIC intf_mgr : public base_mgr<intf_handler, intf_id_t> {
 public:
   virtual ~intf_mgr();

   /// Iterates over all interfaces currently available in the system.
   virtual intf_iter_t intf_iter() const = 0;

   /// Returns true if the intf_id_t has a corresponding status.
   virtual bool exists(intf_id_t) const = 0;

   // Attribute accessors
   /// Returns true if the given interface is configured to be enabled
   virtual bool admin_enabled(intf_id_t) const = 0;
   /// Configures the enabled status of the interface
   virtual void admin_enabled_is(intf_id_t, bool) = 0;
   /**
    * Configure the description of the given interface.
    * Creates a copy of the passed in string description.
    */
   virtual void description_is(intf_id_t, char const *) = 0;
   /// Inspects the curren operational status of the given interface.
   virtual oper_status_t oper_status(intf_id_t) const = 0;

 protected:
   intf_mgr() EOS_SDK_PRIVATE;
   friend class intf_handler;

 private:
   EOS_SDK_DISALLOW_COPY_CTOR(intf_mgr);
};


/**
 * The interface counter manager.
 * This class inspects base interface counters and statistics.
 */
class EOS_SDK_PUBLIC intf_counter_mgr {
 public:
   virtual ~intf_counter_mgr();
   
   /// Get the current counters of the given interface.
   virtual intf_counters_t counters(intf_id_t) const = 0;
   /// Get the current traffic rates of the given interface.
   virtual intf_traffic_rates_t traffic_rates(intf_id_t) const = 0;
   
 protected:
   intf_counter_mgr() EOS_SDK_PRIVATE;
   
 private:
   EOS_SDK_DISALLOW_COPY_CTOR(intf_counter_mgr);
};

}

#include <eos/inline/intf.h>

#endif // EOS_INTF_H
