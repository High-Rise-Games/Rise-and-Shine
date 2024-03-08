#pragma once
//
//  NetworkController.h
//
//  This class handles all network connection and sending.  
//  We just keep track of the connection and trade game states 
//  back-and-forth across the network.
//
//  Author: High Rise Games
//
#ifndef NetworkController_h
#define NetworkController_h
#include <cugl/cugl.h>
#include <vector>
class NetworkController {
protected:
	/** The network connection (as made by this scene) */
	std::shared_ptr<cugl::net::NetcodeConnection> _network;

    /** Whether we quit the game */
    bool _quit;

public:
    /**
     * Creates a new NetworkController with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    NetworkController() {}

    /**
     * The method called to update the scene.
     *
     * We need to update this method to constantly talk to the server
     */
    void update();

    /**
     * Returns the network connection (as made by this scene)
     *
     * This value will be reset every time the scene is made active.
     *
     * @return the network connection (as made by this scene)
     */
    std::shared_ptr<cugl::net::NetcodeConnection> getConnection() const {
        return _network;
    }

    /**
     * Returns the network connection (as made by this scene)
     *
     * This value will be reset every time the scene is made active.
     *
     * @return the network connection (as made by this scene)
     */
    void setConnection(const std::shared_ptr<cugl::net::NetcodeConnection>& network) {
        _network = network;
    }

    /**
     * Returns true if the player quits the game.
     *
     * @return true if the player quits the game.
     */
    bool didQuit() const { return _quit; }

    /**
     * Disconnects this scene from the network controller.
     *
     * Technically, this method does not actually disconnect the network controller.
     * Since the network controller is a smart pointer, it is only fully disconnected
     * when ALL scenes have been disconnected.
     */
    void disconnect() { _network = nullptr; }
    
private:
    /**
     * Processes data sent over the network.
     *
     * Once connection is established, all data sent over the network consistes of
     * byte vectors. This function is a call back function to process that data.
     * Note that this function may be called *multiple times* per animation frame,
     * as the messages can come from several sources.
     *
     * Typically this is where players would communicate their names after being
     * connected. In this lab, we only need it to do one thing: communicate that
     * the host has started the game.
     *
     * @param source    The UUID of the sender
     * @param data      The data received
     */
    void processData(const std::string source, const std::vector<std::byte>& data);

    /**
     * Checks that the network connection is still active.
     *
     * Even if you are not sending messages all that often, you need to be calling
     * this method regularly. This method is used to determine the current state
     * of the scene.
     *
     * @return true if the network connection is still active.
     */
    bool checkConnection();

    /**
     * Transmits a color change to all other devices.
     *
     * Because a device does not receive messages from itself, this method should
     * also set the color.
     *
     * @param color The new color
     */
    // void transmitColor(cugl::Color4 color);
};

#endif NetworkController_h