//
//  CURTCNetConnection.h
//  Cornell University Game Library (CUGL)
//
//  This module provides the configuratiion for a Web RTC implementation. The configuration
//  is designed to be compatible with libdatachannels:
//
//      https://github.com/paullouisageneau/libdatachannel
//
//  Because configurations are intended to be on the stack, we do not provide explicit 
//  shared pointer support for this class.
//
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 1/6/23
//
#ifndef __CU_NETCODE_CONFIG_H__
#define __CU_NETCODE_CONFIG_H__
#include <cugl/net/CUInetAddress.h>
#include <cugl/net/CUICEAddress.h>
#include <unordered_map>
#include <vector>

namespace cugl {

/** Forward reference to JsonValues */
class JsonValue;

    /**
     * The CUGL networking classes.
     *
     * This internal namespace is for optional networking package. Currently CUGL
     * supports ad-hoc game lobbies using web-sockets. The sockets must connect
     * connect to a CUGL game lobby server.
     */
    namespace net {

/**
 * This class represents the configuration for our underlying Netcode. 
 * 
 * Each {@link NetcodeConnection} has a configuration that cannot be changed once 
 * connection is established. This configuration controls such things as the initial
 * lobby server (what the game connects to find other players), the ICE servers
 * (used for NAT traversal), and communication settings like the MTU (maximum 
 * transmission unit). All all of these, only the lobby is required.  Provided that 
 * the lobby is on the same network as the players, the default values for all of
 * the other settings are sufficient.
 *
 * The lobby MUST be the address of a websocket running the CUGL game lobby. While
 * our netcode uses standard Web RTC signaling protocols, a generic signaling server
 * will not give us the room management that we need.
 *
 * When specifying ICE servers, the standard setup is to either specify a STUN and
 * a TURN server, or a STUN server only. Specifying no ICE servers mean that only
 * local connections are supported.
 *
 * This class is effectively a simple struct. All attributes are publicly available
 * and we do not use the standard CUGL shared pointer architecture.  Internet addresses
 * are designed to be use on the stack, though you can combine them with share pointers
 * if you wish.
 */
class NetcodeConfig {
public:
    /** Whether the lobby requires an SSL connection */
    bool secure;
    
    /** The internet address for the lobby server */
    InetAddress lobby;
    
    /** The collection of STUN/TURN servers to use (default None) */
    std::vector<ICEAddress> iceServers;
    
    /** Whether to multiplex connections over a single UDP port (default false) */
    bool multiplex;
    
    /** The starting port to scan for connections (default 1024) */
    uint16_t portRangeBegin;
    
    /** The final port to scan for connections (default 65535) */
    uint16_t portRangeEnd;
    
    /** The maximum transmission unit (default 0 for automatic) */
    uint16_t mtu;
    
    /** The maximum message size (default 0 for automatic) */
    size_t maxMessage;
    
    /** The maximum number of players allowed (default 2) */
    uint16_t maxPlayers;
    
    /**
     * The API version number.
     *
     * Clients with mismatched versions will be prevented from connecting to
     * each other. Start this at 0 and increment it every time a backwards
     * incompatible API change happens.
     */
    uint8_t apiVersion;
    
#pragma mark Constructors
    /**
     * Creates a new configuration.
     *
     * All values will be defaults. The lobby server will be set to 'localhost"
     * at port 8000 (e.g. the Django port).
     */
    NetcodeConfig();

    /**
     * Creates a new configuration with the given lobby server
     *
     * All other values will be defaults. No ICE servers will be specified.
     */
    NetcodeConfig(const InetAddress& lobby);

    /**
     * Creates a new configuration with the given lobby and ICE server
     *
     * All other values will be defaults. No ICE servers will be specified.
     */
    NetcodeConfig(const InetAddress& lobby, const ICEAddress& iceServer);
    
    /**
     * Creates this configuration using a JSON entry.
     *
     * The JSON value should be an object with at least one key -- "lobby" --
     * which is the JSON for an {@link InetAddress}. All other keys are optional.
     * They include:
     *
     *      "secure":       A boolean indicating if the lobby uses SSL
     *      "ICE servers":  A list of {@link ICEAddress} JSONs
     *      "multiplex":    A boolean specifying whether to use UDP multiplexing
     *      "port range":   A list pair of the ports to scan
     *      "MTU":          An int representing the maximum transmission unit
     *      "max message":  An int respresenting the maximum transmission size
     *      "max players":  An int respresenting the maximum number of players
     *      "API version":  An int respresenting the API version
     *
     * @param prefs     The configuration settings
     */
    NetcodeConfig(const std::shared_ptr<JsonValue>& prefs);
    
    /**
     * Creates a copy of the configuration.
     *
     * This copy constructor is provided so that internet addresses may be
     * safely used on the stack, without the use of pointers.
     *
     * @param src    The original configuration to copy
     */
    NetcodeConfig(const NetcodeConfig& src) = default;
    
    /**
     * Creates a new configuration with the resources of the given one.
     *
     * This move constructor is provided so that internet addresses may be
     * used efficiently on the stack, without the use of pointers.
     *
     * @param src    The original configuration contributing resources
     */
    NetcodeConfig(NetcodeConfig&& src) = default;
    
    /**
     * Deletes this configuration, disposing all resources
     */
    ~NetcodeConfig();

#pragma mark Assignment
    /**
     * Assigns this configuration to be a copy of the given configuration.
     *
     * @param src   The configuration to copy
     *
     * @return a reference to this configuration for chaining purposes.
     */
    NetcodeConfig& operator=(const NetcodeConfig& src) = default;
    
    /**
     * Assigns this configuration to have the resources of the given configuration.
     *
     * @param src   The configuration to take resources from
     *
     * @return a reference to this configuration for chaining purposes.
     */
    NetcodeConfig& operator=(NetcodeConfig&& src) = default;
    
    /**
     * Assigns this configuration to be a copy of the given configuration.
     *
     * @param src   The configuration to copy
     *
     * @return a reference to this configuration for chaining purposes.
     */
    NetcodeConfig& set(const NetcodeConfig& src);

    /**
     * Assigns this configuration to be a copy of the given configuration.
     *
     * @param src   The configuration to copy
     *
     * @return a reference to this configuration for chaining purposes.
     */
    NetcodeConfig& set(const std::shared_ptr<NetcodeConfig>& src);

    /**
     * Assigns this configuration according to the given JSON object
     *
     * which is the JSON for an {@link InetAddress}. All other keys are optional.
     * They include:
     *
     *      "secure":       A boolean indicating if the lobby uses SSL
     *      "ICE servers":  A list of {@link ICEAddress} JSONs
     *      "multiplex":    A boolean specifying whether to use UDP multiplexing
     *      "port range":   A list pair of the ports to scan
     *      "MTU":          An int representing the maximum transmission unit
     *      "max message":  An int respresenting the maximum transmission size
     *      "max players":  An int respresenting the maximum number of players
     *      "API version":  An int respresenting the API version
     *
     * @param pref      The address settings
     *
     * @return a reference to this address for chaining purposes.
     */
    NetcodeConfig& set(const std::shared_ptr<JsonValue>& pref);

};

    }
}
#endif
