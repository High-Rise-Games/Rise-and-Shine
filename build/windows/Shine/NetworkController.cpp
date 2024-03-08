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
 * The method called to update the scene.
 *
 * We need to update this method to constantly talk to the server
 */
void NetworkController::update() {
    if (_network) {
        _network->receive([this](const std::string source,
            const std::vector<std::byte>& data) {
                processData(source, data);
            });
        checkConnection();
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
 * This is where we handle the gameplay. All connected devices should immediately
 * change their color when directed by the following method. Changing the color
 * means changing the clear color of the entire {@link Application}.
 *
 * @param source    The UUID of the sender
 * @param data  The data received
 */
void NetworkController::processData(const std::string source,
    const std::vector<std::byte>& data) {
    Color4 color((int)data[0], (int)data[1], (int)data[2], (int)data[3]);
    Application::get()->setClearColor(color);
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
        Application::get()->setClearColor(Color4("#c0c0c0"));
        // _quit = true;
        return false;
    }
    return true;
}


/**
 * Transmits a color change to all other devices.
 *
 * Because a device does not receive messages from itself, this method should
 * also set the color (the clear color of the {@link Application} that is).
 *
 * @param color The new color
 
void GameScene::transmitColor(Color4 color) {
    Application::get()->setClearColor(color);

    vector<std::byte> color_vec;
    color_vec.push_back((std::byte)color.r);
    color_vec.push_back((std::byte)color.g);
    color_vec.push_back((std::byte)color.b);
    color_vec.push_back((std::byte)color.a);

    _network->broadcast(color_vec);
} */