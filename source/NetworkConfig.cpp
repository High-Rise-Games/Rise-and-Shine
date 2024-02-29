//
//  NetworkConfig.cpp
//  Ship
//
//  Created by Troy Moslemi on 2/29/24.
//

#include "NetworkConfig.h"
#include <cugl/cugl.h>
#include <iostream>
#include <sstream>

using namespace cugl;
using namespace cugl::net;
using namespace std;


/**
 * Converts a hexadecimal string to a decimal string
 *
 * This function assumes that the string is 4 hexadecimal characters
 * or less, and therefore it converts to a decimal string of five
 * characters or less (as is the case with the lobby server). We
 * pad the decimal string with leading 0s to bring it to 5 characters
 * exactly.
 *
 * @param hex the hexadecimal string to convert
 *
 * @return the decimal equivalent to hex
 */
static std::string hex2dec(const std::string hex) {
    Uint32 value = strtool::stou32(hex,0,16);
    std::string result = strtool::to_string(value);
    if (result.size() < 5) {
        size_t diff = 5-result.size();
        std::string alt(5,'0');
        for(size_t ii = 0; ii < result.size(); ii++) {
            alt[diff+ii] = result[ii];
        }
        result = alt;
    }
    return result;
}

/**
 * Converts a decimal string to a hexadecimal string
 *
 * This function assumes that the string is a decimal number less
 * than 65535, and therefore converts to a hexadecimal string of four
 * characters or less (as is the case with the lobby server). We
 * pad the hexadecimal string with leading 0s to bring it to four
 * characters exactly.
 *
 * @param dec the decimal string to convert
 *
 * @return the hexadecimal equivalent to dec
 */
static std::string dec2hex(const std::string dec) {
    Uint32 value = strtool::stou32(dec);
    if (value >= 655366) {
        value = 0;
    }
    return strtool::to_hexstring(value,4);
}


bool NetworkConfig::init(const std::shared_ptr<cugl::AssetManager>& assets, bool host) {
    // Initialize the scene to a locked width
    
    auto json = assets->get<JsonValue>("server");
    _config.set(json);
    
    if (host) {
        setHost();
    }
    
    setActive(false);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void NetworkConfig::dispose() {
    if (_active) {
        _network = nullptr;
        _active = false;
    }
}


/**
 * The method called to update the player's NetworkConfig.
 *
 * We need to update this method to constantly talk to the server
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void NetworkConfig::update(float timestep) {
    // We have written this for you this time
    if (_network) {
        _network->receive([this](const std::string source,
                                 const std::vector<std::byte>& data) {
            processData(source,data);
        });
        checkConnection();
        
        // include what logic needs to be done every time step below
        
    }
}

/**
 * Processes data sent over the network.
 *
 * Once connection is established, all data sent over the network consistes of
 * byte vectors. This function is a call back function to process that data.
 * Note that this function may be called *multiple times* per animation frame,
 * as the messages can come from several sources.
 *
 * @param source    The UUID of the sender
 * @param data      The data received
 */
void NetworkConfig::processData(const std::string source,
                            const std::vector<std::byte>& data) {
    
    
}


/**
 * Connects to the game server as specified in the assets file for a player with this Network
 * Config object.
 *
 * The {@link #init} method set the configuration data. This method simply uses
 * this to create a new {@Link NetworkConnection}. It also immediately calls
 * {@link #checkConnection} to determine the scene state.
 *
 * @return true if the connection was successful
 */
bool NetworkConfig::connect() {
    // IMPLEMENT ME
    
    _network = cugl::net::NetcodeConnection().alloc(_config);
    _network->open();
    
    if (checkConnection()) {
        return true;
    }
    
    return false;

}

/**
 * Checks that the network connection is still active for a player with this
 * NetworkConfig object.
 *
 * Even if you are not sending messages all that often, you need to be calling
 * this method regularly. This method is used to determine the current state
 * of the scene.
 *
 * @return true if the network connection is still active.
 */
bool NetworkConfig::checkConnection() {
        
   
    if (isHost()) {
        switch(_network->getState()) {
            case cugl::net::NetcodeConnection::State::CONNECTED:
                if (_status == WAIT) {
                    _status = IDLE;
                    _gameid->setText(hex2dec(_network->getRoom()));
                } else {
                    //                _status = START;
                    
                }
                return true;
            case cugl::net::NetcodeConnection::State::MISMATCHED:
                // code block
                _status = WAIT;
                return true;
            case cugl::net::NetcodeConnection::State::INVALID:
                // code block
                _status = WAIT;
                return true;
            case cugl::net::NetcodeConnection::State::FAILED:
                // code block
                _status = WAIT;
                return true;
            case cugl::net::NetcodeConnection::State::DENIED:
                // code block
                _status = WAIT;
                return true;
            case cugl::net::NetcodeConnection::State::DISCONNECTED:
                // code block
                _status = WAIT;
                return false;
            default:
                return false;
        }
        
        return false;
    } else {
        switch(_network->getState()) {
            case cugl::net::NetcodeConnection::State::CONNECTED:
                if (_status != START) {
                    _status = WAIT;
                }
               return true;
            case cugl::net::NetcodeConnection::State::MISMATCHED:
            // code block
                _status = WAIT;
                return true;
            case cugl::net::NetcodeConnection::State::NEGOTIATING:
            // code block
                _status = JOIN;
                return true;
            case cugl::net::NetcodeConnection::State::INVALID:
            // code block
                _status = IDLE;
                return true;
            case cugl::net::NetcodeConnection::State::FAILED:
            // code block
                _status = IDLE;
                return true;
            case cugl::net::NetcodeConnection::State::DENIED:
            // code block
                _status = IDLE;
                return true;
            case cugl::net::NetcodeConnection::State::DISCONNECTED:
            // code block
                _status = IDLE;
                return false;
          default:
            return true;
        }
        
        return true;
    }
    
}



/**
 * Starts the games (method only for host)
 *
 * This method is called once the requisite number of players have connected.
 * It locks down the room and sends a "start game" message to all other
 * players.
 */
void NetworkConfig::startGame() {
    
    
    if (isHost()) {
        _status = Status::START;
        std::vector<std::byte> byteVec;
        
        // Probaly need to change what data we send
        // for clients to start the game
        byteVec.push_back(std::byte{0xff});
        _network->broadcast(byteVec);
    }
    
}


