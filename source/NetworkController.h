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
     * Sets the network connection (as made by this scene)
     *
     * This value will be reset every time the scene is made active.
     *
     * @param network   the network connection (as made by this scene)
     */
    void setConnection(const std::shared_ptr<cugl::net::NetcodeConnection>& network) {
        _network = network;
    }
    
    /**
     *
     * FUNCTION FOR HOST ONLY
     *
     * Connects to the game server as specified in the assets file
     *
     * The {@link #init} method set the configuration data. This method simply uses
     * this to create a new {@Link NetworkConnection}. It also immediately calls
     * {@link #checkConnection} to determine the scene state.
     *
     * @return true if the connection was successful
     */
    bool connect(cugl::net::NetcodeConfig config);
    
    /**
     * FUNCTION FOR CLIENT ONLY
     *
     * Connects to the game server as specified in the assets file
     *
     * The {@link #init} method set the configuration data. This method simply uses
     * this to create a new {@Link NetworkConnection}. It also immediately calls
     * {@link #checkConnection} to determine the scene state.
     *
     * @param room  The room ID to use
     *
     * @return true if the connection was successful
     */
    bool connect(const std::string room, cugl::net::NetcodeConfig config);

    /** Returns the number of players on this network. */
    int getNumPlayers() { return _network->getPeers().size() + 1; }

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
     * We do not handle any gameplay in this method. We simply return the JSON value
     * representing the board state retrieved from the network.
     *
     * Example board state:
     * {
        "player_id":  1,
        "num_dirt": 1,
        "curr_board": 0,
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
     *
     * Example movement message:
     * {
     *    "player_id":  1,
     *    "vel": [0.234, 1.153]
     * }
     *
     *
     * @param source    The UUID of the sender
     * @param data      The data received
     * @returns the data as a JSON value representing a message
     */
    std::shared_ptr<cugl::JsonValue> processMessage(const std::string source, const std::vector<std::byte>& data);

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
     * Transmits the board state belonging to the user given by
     * their id to all other devices.
     *
     * Example board state:
     * {
        "player_id":  1,
        "num_dirt": 1,
        "curr_board": 0,
        "player_x": 3,
        "player_y": 6,
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
     * @param state     The user's board state
     */
    void transmitMessage(const std::shared_ptr<cugl::JsonValue> state);
};

#endif NetworkController_h
