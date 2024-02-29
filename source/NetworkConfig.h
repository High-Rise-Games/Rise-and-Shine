//
//  NetworkConfig.hpp
//  Ship
//
//  Created by Troy Moslemi on 2/29/24.
//

#ifndef NetworkConfig_hpp
#define NetworkConfig_hpp

#include <stdio.h>

#include <cugl/cugl.h>
#include <vector>


/**
 * This class provides the interface to make a new game.
 *
 * Most games have a since "matching" scene whose purpose is to initialize the
 * network controller.  We have separate the host from the client to make the
 * code a little more clear.
 */
class NetworkConfig {
public:
    /**
     * The configuration status
     *
     * This is how the application knows to switch to the next scene.
     */
    enum Status {
        /** Host is waiting on a connection */
        WAIT,
        /** Host is waiting on all players to join */
        IDLE,
        /** Time to start the game */
        START,
        /** Game was aborted; back to main menu */
        ABORT,
        /** Client is connecting to the host */
        JOIN,
    };
    
protected:
    
    /** The asset manager  for main game scene to access server json file. */
    std::shared_ptr<cugl::AssetManager> _assets;

    std::shared_ptr<cugl::net::NetcodeConnection> _network;

    std::shared_ptr<cugl::scene2::Label> _gameid;
    /** The players label (for updating) */
    std::shared_ptr<cugl::scene2::Label> _player;
    
    /** The network configuration */
    cugl::net::NetcodeConfig _config;
    
    /** The current status */
    Status _status;
    
    /** If owner of this NetworkConfig object is host. */
    bool _host;
    
    /** Whether _network is null pointer or not. True if not. */
    bool _active;

public:
#pragma mark -
#pragma mark Constructors
    
    NetworkConfig(); // constructor
    
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~NetworkConfig() { dispose(); }
    
    /**
     * Kill Network connection.
     */
    void dispose();
    

    // True if host, false if not
    // Asset manager to access json file for server
    bool init(const std::shared_ptr<cugl::AssetManager>& assets, bool host);
    
    /**
     * Sets whether the _network is null or not. True if not null.
     */
    virtual void setActive(bool value);
    
    void isActive();
    
    /**
     * Returns the network connection (as made by this scene)
     *
     * This value will be reset every time the scene is made active.
     * In addition, this method will return nullptr if {@link #disconnect}
     * has been called.
     *
     * @return the network connection (as made by this scene)
     */
    std::shared_ptr<cugl::net::NetcodeConnection> getConnection() const {
        return _network;
    }

    /**
     * Returns the scene status.
     *
     * Any value other than WAIT will transition to a new scene.
     *
     * @return the scene status
     *
     */
    Status getStatus() const { return _status; }

    /**
     * The method called to update the scene.
     *
     * We need to update this method to constantly talk to the server
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void update(float timestep);
    
    /**
     * Disconnects this scene from the network controller.
     *
     * Technically, this method does not actually disconnect the network controller.
     * Since the network controller is a smart pointer, it is only fully disconnected
     * when ALL scenes have been disconnected.
     */
    void disconnect() { _network = nullptr; }

private:
    
    // Sets the NetworkConfig of this object to be host of the network game
    void setHost() {_host = true; }
    
    // Check if the player with this NetworkConfig object is host
    bool isHost() {return _host; }
    

    /**
     * Connects to the game server as specified in the assets file
     *
     * The {@link #init} method set the configuration data. This method simply uses
     * this to create a new {@Link NetworkConnection}. It also immediately calls
     * {@link #checkConnection} to determine the scene state.
     *
     * @return true if the connection was successful
     */
    bool connect();

    /**
     * Processes data sent over the network.
     *
     * Once connection is established, all data sent over the network consistes of
     * byte vectors. This function is a call back function to process that data.
     * Note that this function may be called *multiple times* per animation frame,
     * as the messages can come from several sources.
     *
     * In this lab, this method does not do all that much. Typically this is where
     * players would communicate their names after being connected.
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
     * Starts the game.
     *
     * This method is called once the requisite number of players have connected.
     * It locks down the room and sends a "start game" message to all other
     * players.
     */
    void startGame();
    
};

#endif /* NetworkConfig_hpp */
