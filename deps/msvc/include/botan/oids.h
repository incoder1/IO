/*
* OID Registry
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_OIDS_H_
#define BOTAN_OIDS_H_

#include <botan/asn1_oid.h>

namespace Botan {

namespace OIDS {

/**
* Resolve an OID
* @param oid the OID to look up
* @return name associated with this OID
*/
BOTAN_PUBLIC_API(2,0) std::string lookup(const OID& oid);

/**
* Find the OID to a name. The lookup will be performed in the
* general OID section of the configuration.
* @param name the name to resolve
* @return OID associated with the specified name
*/
BOTAN_PUBLIC_API(2,0) OID lookup(const std::string& name);

inline std::string oid2str(const OID& oid)
   {
   return lookup(oid);
   }

inline OID str2oid(const std::string& name)
   {
   return lookup(name);
   }

/**
* See if an OID exists in the internal table.
* @param oid the oid to check for
* @return true if the oid is registered
*/
inline bool have_oid(const std::string& oid)
   {
   return (lookup(oid).empty() == false);
   }

}

}

#endif
