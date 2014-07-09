// Copyright (c) 2013 Arista Networks, Inc.  All rights reserved.
// Arista Networks, Inc. Confidential and Proprietary.

#ifndef EOS_IP_ROUTE_H
#define EOS_IP_ROUTE_H

#include <eos/intf.h>
#include <eos/ip.h>
#include <eos/iterator.h>
#include <eos/mpls.h>

namespace eos {

typedef uint32_t ip_route_tag_t;
typedef uint32_t ip_route_metric_t;
typedef uint8_t ip_route_preference_t;

/**
 * An IP route key, consisting of a prefix and preference.
 */
class EOS_SDK_PUBLIC ip_route_key_t {
 public:
   ip_route_key_t();
   explicit ip_route_key_t(ip_prefix_t const &);
   ip_route_key_t(ip_prefix_t const &, ip_route_preference_t);

   bool operator==(ip_route_key_t const & other) const;
   bool operator!=(ip_route_key_t const & other) const;

   ip_prefix_t prefix;                // IP v4/v6 network prefix
   ip_route_preference_t preference;  // 0..255 only, defaults to 1
   ip_route_metric_t metric;
};

/**
 * An IP v4/v6 static route.
 * "Via", or nexthops, for this route are stored separately and are
 * associated by route key.
 */
class EOS_SDK_PUBLIC ip_route_t {
 public:
   ip_route_t();
   /// Creates an IP static route for the route key
   explicit ip_route_t(ip_route_key_t const & route_key);

   ip_route_key_t key;  ///< The route's key

   ip_route_tag_t tag;  ///< A numbered tag, used for table segregation
   /**
    * Whether this route persists in system configuration.
    * If true, the route appears in 'show running-config', and will
    * be saved to startup-config if a 'copy running start' or
    * 'write memory' CLI command is issued.
    */
   bool persistent;
};

/**
 * A Via describing a particular set of nexthop information.
 *
 * A Via can describe either a nexthop IP address (hop), interface
 * (intf) or nexthop group name to either forward traffic to, or
 * drop traffic if the interface is Null0.
 */
class EOS_SDK_PUBLIC ip_route_via_t {
 public:
   ip_route_via_t();
   /// Creates a route via for a route key
   explicit ip_route_via_t(ip_route_key_t const & route_key);

   ip_route_key_t route_key; ///< Key for the route this via is attached to
   ip_addr_t hop;            ///< IP v4/v6 nexthop address
   /**
    * Use the named interface if not a default intf_id_t
    *
    * Using intf Null0 installs a 'drop' route for the given prefix and preference
    */
   intf_id_t intf;

   /**
    * Name of the next-hop group to use.
    * If this string is non-empty, the next-hop group of the given name will
    * be used, and both `hop' and `intf' must be left to their default value
    * otherwise we will panic().
    *
    * Note this is currently only supported for IPv4 routes.
    */
   std::string nexthop_group;

   /// Push an MPLS label
   mpls_label_t mpls_label;

   bool operator==(ip_route_via_t const & other) const;
   bool operator!=(ip_route_via_t const & other) const;
};


class ip_route_iter_impl;

/// An IP route iterator
class EOS_SDK_PUBLIC ip_route_iter_t : public iter_base<ip_route_t,
                                                        ip_route_iter_impl> {
 private:
   friend class ip_route_iter_impl;
   explicit ip_route_iter_t(ip_route_iter_impl * const) EOS_SDK_PRIVATE;
};


class ip_route_via_iter_impl;

/// A route via iterator
class EOS_SDK_PUBLIC ip_route_via_iter_t : public iter_base<ip_route_via_t,
                                                            ip_route_via_iter_impl> {
 private:
   friend class ip_route_via_iter_impl;
   explicit ip_route_via_iter_t(ip_route_via_iter_impl * const) EOS_SDK_PRIVATE;
};


/// The manager for IP static route configuration
class EOS_SDK_PUBLIC ip_route_mgr {
 public:
    virtual ~ip_route_mgr();

   /**
    * Set the manager to only interact with routes with the given tag.
    *
    * If this tag is set to a non-zero number, then methods on this
    * class will only affect or expose routes with the given tag.
    */
   virtual void tag_is(uint32_t) = 0;

   /// Returns the current tag, or 0 if no tag is set.
   virtual uint32_t tag() const = 0;

   /**
    * Resync provides a mechanism to set routes to a known state.
    *
    * To start a resync, call resync_init() and then use
    * ip_route_set() and ip_route_via_set() to populate the static
    * routing table with the known entries. After all entries have
    * been set, call resync_complete(), which will delete all existing
    * routes and vias that were not added or modified during resync.
    * If a tag is set, resync will only delete routes and vias
    * corresponding to the current tag.
    *
    * During resync, this manager will act like it is referencing a
    * temporary table that starts off empty. Thus, methods like exist,
    * del, and getters will act only on that temporary table,
    * regardless of the real values in Sysdb. The one exception is
    * iteration; they will traverse the table stored in Sysdb,
    * regardless of whether or not the manager is in resync mode.
    */
   virtual void resync_init() = 0;
   /// Completes any underway resync operation.
   virtual void resync_complete() = 0;

   /**
    * Iterates across all configured static routes.
    * If a tag is set, only return routes that have match the current tag.
    */
   virtual ip_route_iter_t ip_route_iter() const = 0;

   /**
    * Iterates across configured nexthops for a given route key,
    * i.e., emit all ip_route_via_t's for a given route key. If a tag
    * is set, only return vias on routes that match the current tag.
    */
   virtual ip_route_via_iter_t ip_route_via_iter(ip_route_key_t const &) const = 0;

   /**
    * Tests for existence of any routes matching the route key in the config.
    * If a tag is set and the route belongs to a different tag, this
    * function returns false.
    */
   virtual bool exists(ip_route_key_t const &) const = 0;
   /// Tests if the given via exists.
   virtual bool exists(ip_route_via_t const &) const = 0;

   // Route management functions

   /// Gets a static route, or panics if the route key does not exist.
   virtual ip_route_t ip_route(ip_route_key_t const &) = 0;
   /// Inserts or updates a static route into the switch configuration.
   virtual void ip_route_set(ip_route_t const &) = 0;
   /// Removes all ECMP vias matching the route key, and the route itself.
   virtual void ip_route_del(ip_route_key_t const &) = 0;

   /**
    * Adds a via to an ip_route_t.
    * Will call panic() if the corresponding route does not match the
    * currently configured tag.
    *
    * @throws eos::invalid_argument_error If the passed via has no IP
    * address, interface or nexthop group set.
    */
   virtual void ip_route_via_set(ip_route_via_t const &) = 0;
   /**
    * Removes a via from an ip_route_t.
    * When all vias are removed, the route still exists with no
    * nexthop information.
    */
   virtual void ip_route_via_del(ip_route_via_t const &) = 0;

 protected:
   ip_route_mgr() EOS_SDK_PRIVATE;
 private:
   EOS_SDK_DISALLOW_COPY_CTOR(ip_route_mgr);
};

}  // end namespace eos

#include <eos/inline/ip_route.h>

#endif // EOS_IP_ROUTE_H
