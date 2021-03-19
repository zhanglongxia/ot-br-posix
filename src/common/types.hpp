/*
 *    Copyright (c) 2017, The OpenThread Authors.
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *    POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file includes definition for data types used by Thread border agent.
 */

#ifndef OTBR_COMMON_TYPES_HPP_
#define OTBR_COMMON_TYPES_HPP_

#include "openthread-br/config.h"

#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

#include "common/toolchain.hpp"

#ifndef IN6ADDR_ANY
/**
 * Any IPv6 address literal.
 *
 */
#define IN6ADDR_ANY "::"
#endif

#define OTBR_IP6_ADDRESS_SIZE 16
#define OTBR_IP6_PREFIX_SIZE 8
#define OTBR_MASTER_KEY_SIZE 16
#define OTBR_PSKC_SIZE 16

/**
 * Forward declaration for otIp6Prefix to avoid including <openthread/ip6.h>
 *
 */
struct otIp6Prefix;

/**
 * Forward declaration for otExtAddress to avoid including <openthread/platform/radio.h>
 *
 */
struct otExtAddress;

/**
 * Forward declaration for otExtendedPanId to avoid including <openthread/dataset.h>
 *
 */
struct otExtendedPanId;

/**
 * This enumeration represents error codes used throughout OpenThread Border Router.
 */
enum otbrError
{
    OTBR_ERROR_NONE = 0, ///< No error.

    OTBR_ERROR_ERRNO           = -1,  ///< Error defined by errno.
    OTBR_ERROR_DBUS            = -2,  ///< DBus error.
    OTBR_ERROR_MDNS            = -3,  ///< MDNS error.
    OTBR_ERROR_OPENTHREAD      = -4,  ///< OpenThread error.
    OTBR_ERROR_REST            = -5,  ///< Rest Server error.
    OTBR_ERROR_SMCROUTE        = -6,  ///< SMCRoute error.
    OTBR_ERROR_NOT_FOUND       = -7,  ///< Not found.
    OTBR_ERROR_PARSE           = -8,  ///< Parse error.
    OTBR_ERROR_NOT_IMPLEMENTED = -9,  ///< Not implemented error.
    OTBR_ERROR_INVALID_ARGS    = -10, ///< Invalid arguments error.
    OTBR_ERROR_DUPLICATED      = -11, ///< Duplicated operation, resource or name.
};

namespace otbr {

enum
{
    kSizePSKc        = 16,         ///< Size of PSKc.
    kSizeNetworkName = 16,         ///< Max size of Network Name.
    kSizeExtPanId    = 8,          ///< Size of Extended PAN ID.
    kSizeEui64       = 8,          ///< Size of Eui64.
    kSizeExtAddr     = kSizeEui64, ///< Size of Extended Address.
};

static constexpr char kSolicitedMulticastAddressPrefix[]   = "ff02::01:ff00:0";
static constexpr char kLinkLocalAllNodesMulticastAddress[] = "ff02::01";

/**
 * This method converts the hex array to the string in hex format.
 *
 * @param[in]  aHex     A pointer to the hex array.
 * @param[in]  aLength  The length of the hex array.
 *
 * @returns The string in hex format.
 *
 */
std::string HexToString(const uint8_t *aHex, uint16_t aLength);

/**
 * This method converts the extended pan ID in uint64_t to otExtendedPanId.
 *
 * @param[in] aExtPanId  The extended pan ID in uint64_t.
 *
 * @returns The otExtendedPanId value.
 *
 */
otExtendedPanId Uint64ToOtExtendedPanId(uint64_t aExtPanId);

/**
 * This class implements the Ipv6 address functionality.
 *
 */
OTBR_TOOL_PACKED_BEGIN
class Ip6Address
{
public:
    /**
     * Default constructor.
     *
     */
    Ip6Address(void)
    {
        m64[0] = 0;
        m64[1] = 0;
    }

    /**
     * Constructor with an 16-bit Thread locator.
     *
     * @param[in]   aLocator    16-bit Thread locator, RLOC or ALOC.
     *
     */
    Ip6Address(uint16_t aLocator)
    {
        m64[0] = 0;
        m32[2] = 0;
        m16[6] = 0;
        m8[14] = aLocator >> 8;
        m8[15] = aLocator & 0xff;
    }

    /**
     * Constructor with an Ip6 address.
     *
     * @param[in]   aAddress    The Ip6 address.
     *
     */
    Ip6Address(const uint8_t (&aAddress)[16]);

    /**
     * This method overloads `<` operator and compares if the Ip6 address is smaller than the other address.
     *
     * @param[in] aOther  The other Ip6 address to compare with.
     *
     * @returns  Whether the Ip6 address is smaller than the other address.
     *
     */
    bool operator<(const Ip6Address &aOther) const { return memcmp(this, &aOther, sizeof(Ip6Address)) < 0; }

    /**
     * This method overloads `==` operator and compares if the Ip6 address is equal to the other address.
     *
     * @param[in] aOther  The other Ip6 address to compare with.
     *
     * @returns  Whether the Ip6 address is equal to the other address.
     *
     */
    bool operator==(const Ip6Address &aOther) const { return m64[0] == aOther.m64[0] && m64[1] == aOther.m64[1]; }

    /**
     * Retrieve the 16-bit Thread locator.
     *
     * @returns RLOC16 or ALOC16.
     *
     */
    uint16_t ToLocator(void) const { return static_cast<uint16_t>(m8[14] << 8 | m8[15]); }

    /**
     * This method returns the solicited node multicast address.
     *
     * @returns The solicited node multicast address.
     *
     */
    Ip6Address ToSolicitedNodeMulticastAddress(void) const;

    /**
     * This method returns the string representation for the Ip6 address.
     *
     * @returns The string representation of the Ip6 address.
     *
     */
    std::string ToString(void) const;

    /**
     * This method returns if the Ip6 address is a multicast address.
     *
     * @returns  Whether the Ip6 address is a multicast address.
     *
     */
    bool IsMulticast(void) const { return m8[0] == 0xff; }

    /**
     * This function returns the wellknown Link Local All Nodes Multicast Address (ff02::1).
     *
     * @returns The Link Local All Nodes Multicast Address.
     *
     */
    static const Ip6Address &GetLinkLocalAllNodesMulticastAddress(void)
    {
        static Ip6Address sLinkLocalAllNodesMulticastAddress = FromString(kLinkLocalAllNodesMulticastAddress);

        return sLinkLocalAllNodesMulticastAddress;
    }

    /**
     * This function returns the wellknown Solicited Node Multicast Address Prefix (ff02::01:ff00:0).
     *
     * @returns The Solicited Node Multicast Address Prefix.
     *
     */
    static const Ip6Address &GetSolicitedMulticastAddressPrefix(void)
    {
        static Ip6Address sSolicitedMulticastAddressPrefix = FromString(kSolicitedMulticastAddressPrefix);

        return sSolicitedMulticastAddressPrefix;
    }

    /**
     * This function converts Ip6 addresses from text to `Ip6Address`.
     *
     * @param[in]   aStr    The Ip6 address text.
     * @param[out]  aAddr   A reference to `Ip6Address` to output the Ip6 address.
     *
     * @retval OTBR_ERROR_NONE          If the Ip6 address was successfully converted.
     * @retval OTBR_ERROR_INVALID_ARGS  If @p `aStr` is not a valid string representing of Ip6 address.
     *
     */
    static otbrError FromString(const char *aStr, Ip6Address &aAddr);

    /**
     * This method copies the Ip6 address to a `sockaddr_in6` structure.
     *
     * @param[out] aSockAddr  The `sockaddr_in6` structure to copy the Ip6 adress to.
     *
     */
    void CopyTo(struct sockaddr_in6 &aSockAddr) const;

    /**
     * This method copies the Ip6 address to a `in6_addr` structure.
     *
     * @param[out] aIn6Addr  The `in6_addr` structure to copy the Ip6 adress to.
     *
     */
    void CopyTo(struct in6_addr &aIn6Addr) const;

    union
    {
        uint8_t  m8[16];
        uint16_t m16[8];
        uint32_t m32[4];
        uint64_t m64[2];
    };

private:
    static Ip6Address FromString(const char *aStr);

} OTBR_TOOL_PACKED_END;

/**
 * This class represents a Ipv6 prefix.
 *
 */
OTBR_TOOL_PACKED_BEGIN
class Ip6Prefix
{
public:
    /**
     * Default constructor.
     *
     */
    Ip6Prefix(void) { Clear(); }

    /**
     * Constructor with an Ip6 prefix.
     *
     * @param[in]   aAddress    The Ip6 prefix.
     *
     */
    Ip6Prefix(const otIp6Prefix &aPrefix) { Set(aPrefix); }

    /**
     * This method sets the Ip6 prefix to an `otIp6Prefix` value.
     *
     * @param[in] aPrefix  The `otIp6Prefix` value to set the Ip6 prefix.
     *
     */
    void Set(const otIp6Prefix &aPrefix);

    /**
     * This method returns the string representation for the Ip6 prefix.
     *
     * @returns The string representation of the Ip6 prefix.
     *
     */
    std::string ToString(void) const;

    /**
     * This method clears the Ip6 prefix to be unspecified.
     *
     */
    void Clear(void) { memset(reinterpret_cast<void *>(this), 0, sizeof(*this)); }

    /**
     * This method returns if the Ip6 prefix is valid.
     *
     * @returns  If the Ip6 prefix is valid.
     *
     */
    bool IsValid(void) const { return mLength > 0 && mLength <= 128; }

    Ip6Address mPrefix; ///< The IPv6 prefix.
    uint8_t    mLength; ///< The IPv6 prefix length (in bits).
} OTBR_TOOL_PACKED_END;

/**
 * This class implements the Ipv6 network prefix functionality.
 *
 */
OTBR_TOOL_PACKED_BEGIN
class Ip6NetworkPrefix
{
public:
    /**
     * Default constructor.
     *
     */
    Ip6NetworkPrefix(void) { m64[0] = 0; }

    /**
     * Constructor with an 8-bytes prefix.
     *
     * @param[in] aPrefix  A pointer to the buffer containing the 8-bytes prefix.
     *
     */
    Ip6NetworkPrefix(const uint8_t *aPrefix) { memcpy(m8, aPrefix, sizeof(*this)); }

    /**
     * This method returns the string representation for the Ip6 network prefix.
     *
     * @returns The string representation of the Ip6 network prefix.
     *
     */
    std::string ToString(void) const;

    union
    {
        uint8_t  m8[8];
        uint16_t m16[4];
        uint32_t m32[2];
        uint64_t m64[1];
    };
} OTBR_TOOL_PACKED_END;

/**
 * This class represents an ethernet MAC address.
 */
OTBR_TOOL_PACKED_BEGIN
class MacAddress
{
public:
    /**
     * Default constructor.
     *
     */
    MacAddress(void)
    {
        m16[0] = 0;
        m16[1] = 0;
        m16[2] = 0;
    }

    /**
     * This method returns the string representation for the MAC address.
     *
     * @returns The string representation of the MAC address.
     *
     */
    std::string ToString(void) const { return HexToString(m8, sizeof(m8)); }

    union
    {
        uint8_t  m8[6];
        uint16_t m16[3];
    };
} OTBR_TOOL_PACKED_END;

/**
 * This class represents an extended MAC address.
 */
OTBR_TOOL_PACKED_BEGIN
class ExtAddress
{
public:
    /**
     * Default constructor.
     *
     */
    ExtAddress(void) { memset(m8, 0, sizeof(m8)); }

    /**
     * Constructor with an extended MAC address.
     *
     * @param[in]  aExtAddress  The extended MAC address.
     *
     */
    ExtAddress(const otExtAddress &aExtAddress) { memcpy(reinterpret_cast<void *>(this), &aExtAddress, sizeof(*this)); }

    /**
     * This method returns the string representation for the Extended address.
     *
     * @returns The string representation of the Extended address.
     *
     */
    std::string ToString(void) const { return HexToString(m8, 8); };

    uint8_t m8[8];
} OTBR_TOOL_PACKED_END;

/**
 * This class represents an extended Pan Id.
 */
OTBR_TOOL_PACKED_BEGIN
class ExtPanId
{
public:
    /**
     * Default constructor.
     *
     */
    ExtPanId(void) { memset(m8, 0, sizeof(m8)); }

    /**
     * Constructor with an extended Pan Id.
     *
     * @param[in] aExtPanId  The extended Pan Id.
     *
     */
    ExtPanId(const otExtendedPanId &aExtPanId) { memcpy(reinterpret_cast<void *>(this), &aExtPanId, sizeof(*this)); }

    /**
     * This method returns the string representation for the extended Pan Id.
     *
     * @returns The string representation of the extended Pan Id.
     *
     */
    std::string ToString(void) const { return HexToString(m8, sizeof(m8)); };

    uint8_t m8[8];
} OTBR_TOOL_PACKED_END;
} // namespace otbr

#endif // OTBR_COMMON_TYPES_HPP_
