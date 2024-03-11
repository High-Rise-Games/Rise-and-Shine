//
//  NetworkController.h
//
//  This class handles all network connection and sending.  
//  We just keep track of the connection and trade game states 
//  back-and-forth across the network.
//
//  Author: High Rise Games
//
#include <cugl/cugl.h>
#include <iostream>
#include <sstream>
#include <cstddef>
#include <vector>

#include "NetworkController.h"

using namespace cugl;
using namespace cugl::net;
using namespace std;


/**
 * Processes data sent over the network.
 *
 * Once connection is established, all data sent over the network consistes of
 * byte vectors. This function is a call back function to process that data.
 * Note that this function may be called *multiple times* per animation frame,
 * as the messages can come from several sources.
 *
 * We do not handle any gameplay in this method. We simply return the JSON value
 * representing the board state retrieved from the network.
 * 
 * Example board state:
 * {
    "player_id":  1,
    "player_x": 30.2,
    "player_y": 124.2,
    "dirts": [ [0, 1], [2, 2], [0, 2] ],
    "projectiles": [ 
            { 
                "pos": [0.5, 1.676],
                "vel": [2, 3],
                "type: "DIRT"
            },
            {
                "pos": [1.5, 3.281],
                "vel": [0, -2], 
                "type": "POOP
            }
        ]
 * }
 *
 * Example movement message:
 * {
 *    "player_id":  1,
 *    "vel": [0.234, 1.153]
 * }
 * 
 * @param source    The UUID of the sender
 * @param data      The data received
 */
std::shared_ptr<JsonValue> NetworkController::processMessage(const std::string source, const std::vector<std::byte>& data) {
    NetcodeDeserializer netDeserializer;
    netDeserializer.receive(data);
    std::shared_ptr<JsonValue> jsonData = netDeserializer.readJson();
    netDeserializer.reset();
    return jsonData;
}

/**
 * Checks that the network connection is still active.
 *
 * Even if you are not sending messages all that often, you need to be calling
 * this method regularly. This method is used to determine the current state
 * of the scene.
 *
 * @return true if the network connection is still active.
 */
bool NetworkController::checkConnection() {
    net::NetcodeConnection::State s = _network->getState();
    // _player->setText(to_string(_network->getNumPlayers()));
    if (s == net::NetcodeConnection::State::FAILED || s == net::NetcodeConnection::State::DISCONNECTED) {
        _network->close();
        // _quit = true;
        return false;
    }
    return true;
}


/**
 * This method is used to transmit board state, movement message, or any other action messages such
 * throwing dirt.
 *
 * Transmits the board state belonging to the user given by
 * their id to all other devices.
 * 
 * Example board state:
 * {
    "player_id":  1,
    "player_x": 30.2,
    "player_y": 124.2,
    "dirts": [ [0, 1], [2, 2], [0, 2] ],
    ""projectiles": [ 
            { 
                "pos": [0.5, 1.676],
                "vel": [2, 3],
                "type: "DIRT"
            },
            {
                "pos": [1.5, 3.281],
                "vel": [0, -2], 
                "type": "POOP
            }
        ]
 * }
 * 
 * Transmits the movement by the user given by their id to
 * all other devices, only to be handled by the host.
 *
 * Example movement message:
 * {
 *    "player_id":  1,
 *    "vel": [0.234, 1.153]
 * }
 *
 * @param state     The message to be sent
 */
void NetworkController::transmitMessage(const std::shared_ptr<cugl::JsonValue> msg) {
    NetcodeSerializer netSerializer;
    netSerializer.writeJson(msg);
    const std::vector<std::byte>& byteState = netSerializer.serialize();
    _network->broadcast(byteState);
    netSerializer.reset();
} 