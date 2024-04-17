//
//  LobbyScene.h
//
//  This creates the lobby scene, which is handled differently for each player.
//  For example, if the player is the host, the game code is automatically
//  generated for them while the client has to enter the game code to join
//  the game. This is done per Walker White's advice. This scene also
//  generates the networkcontroller for each player. After the game is started,
//  the network controller is transfered to the gamescene.
//
//  Created by Troy Moslemi on 2/29/24.
//

#ifndef LobbyScene_h
#define LobbyScene_h

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
class LobbyScene  : public cugl::Scene2 {
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

    std::string character;
    
protected:
    
    /** The asset manager  for main game scene to access server json file. */
    std::shared_ptr<cugl::AssetManager> _assets;

    std::shared_ptr<cugl::net::NetcodeConnection> _network;
    
    /** Frame variable used to increment frames used to determine
     time to display invalid character choice image**/
    int _invalid_frames;
    
    /** Whether we've quit this scene */
    bool _quit;
    /** Image to draw when player pickes an already selected character **/
    std::shared_ptr<cugl::scene2::SceneNode> _invalid;

    /** Character select buttons */
    std::shared_ptr<cugl::scene2::Button> _select_red;
    std::shared_ptr<cugl::scene2::Button> _select_blue;
    std::shared_ptr<cugl::scene2::Button> _select_green;
    std::shared_ptr<cugl::scene2::Button> _select_yellow;
    std::shared_ptr<cugl::scene2::SceneNode> _character_field_red;
    std::shared_ptr<cugl::scene2::SceneNode> _character_field_blue;
    std::shared_ptr<cugl::scene2::SceneNode> _character_field_green;
    std::shared_ptr<cugl::scene2::SceneNode> _character_field_yellow;

    /** HOST ONLY. List of all client's character selections, default mushroom */
    std::vector<std::string> _all_characters;
    
    /** HOST ONLY. 0 if not selected, 1 if selected.
     *
     * Mushroom = position 0
     * Frog = position 1
     * Chamelon = position 2
     * Flower = position 3
     *
     **/
    std::vector<int> _all_characters_select;

    /** The game id label (for updating) */
    std::shared_ptr<cugl::scene2::Label> _gameid_host;
    
    /** The game id label (for updating) */
    std::shared_ptr<cugl::scene2::TextField> _clientField;
    
    /** The game id label passed from client id input */
    std::string _gameid_client;
    
    /** The players label (for updating) */
    std::shared_ptr<cugl::scene2::Label> _player_field;

    /** The level label (for updating) */
    std::shared_ptr<cugl::scene2::Label> _level_field;

    /** The menu button for starting a game */
    std::shared_ptr<cugl::scene2::Button> _startgame;
    
    /** The back button for the menu scene */
    std::shared_ptr<cugl::scene2::Button> _backout;
    
    /** True when player picks an invalid character selection **/
    bool _invalid_character_selection;
    
    /** The network configuration */
    cugl::net::NetcodeConfig _config;
    
    /** The current status */
    Status _status;
    
    /** If owner of this NetworkConfig object is host. */
    bool _host;

    /** The id of this player to be passed into the game controller */
    int _id;
    
    /** The level chosen for this gameplay*/
    int _level;


public:
#pragma mark -
#pragma mark Constructors
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~LobbyScene() { dispose(); }
    
    /**
     * Dispose the scene and its children
     */
    void dispose() override;
    
//
//    void setInvalidCharacterChoice(bool b) {
//        _invalid=b;
//    }


    bool init_host(const std::shared_ptr<cugl::AssetManager>& assets);

    
    bool init_client(const std::shared_ptr<cugl::AssetManager>& assets);
    
    /**
     * Sets whether scene is active or not, and whether scene is drawn as host
     * depending on decision in MenuScene
     */
    virtual void setActive(bool value) override;
    
    /**
     * Returns true if the scene is currently active
     *
     * @return true if the scene is currently active
     */
    bool isActive( ) const { return _active; }
    
    /** Maps character to int to determin position of character in _all_characters_select **/
    int mapToSelectList(std::string chare) {
        if (chare == "Mushroom") {
            return 0;
        } else if (chare == "Chameleon") {
            return 2;
        } else if (chare == "Frog") {
            return 1;
        } else if (chare == "Flower") {
            return 3;
        } return -1;
    };

    /** Returns the id of this player based on when they joined */
    int getId() { return _id; }
    
    /** set client room ID */
    void setGameidClient(std::string client_id) { _gameid_client = client_id; }

    /** Returns the number of peers/players currently in this lobby. */
    int getNumPlayers() { return _network->getPeers().size() + 1; }

    /** HOST ONLY. Returns all character selections for players in this lobby. */
    std::vector<std::string>& getAllCharacters() { return _all_characters; }
    
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
    void update(float timestep) override;
    
    /**
     * Disconnects this scene from the network controller.
     *
     * Technically, this method does not actually disconnect the network controller.
     * Since the network controller is a smart pointer, it is only fully disconnected
     * when ALL scenes have been disconnected.
     */
    void disconnect() { _network = nullptr; }
    
    // Sets the NetworkConfig of this object to be host of the network game
    void setHost(bool host) {_host = host; }
    
    // Sets the level chosen for current gameplay for host only.
    void setLevel(int level) {_level = level; }

    // Gets the level chosen for current gameplay for host only.
    int getLevel() { return _level; }
    /**
     * Returns true if the player quits the game.
     *
     * @return true if the player quits the game.
     */
    bool didQuit() const { return _quit; }

private:
    
    
    // Check if the player with this NetworkConfig object is host
    bool isHost() {return _host; }
    
    /**
     * Updates the text in the given button.
     *
     * Techincally a button does not contain text. A button is simply a scene graph
     * node with one child for the up state and another for the down state. So to
     * change the text in one of our buttons, we have to descend the scene graph.
     * This method simplifies this process for you.
     *
     * @param button    The button to modify
     * @param text      The new text value
     */
    void updateText(const std::shared_ptr<cugl::scene2::Button>& button, const std::string text);

    

    /**
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
    bool connect();
    
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
    bool connect(const std::string room);

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
     * Reconfigures the start button for this scene
     *
     * This is necessary because what the buttons do depends on the state of the
     * networking.
     */
    void configureStartButton();
    
    /**
     * Starts the game.
     *
     * This method is called once the requisite number of players have connected.
     * It locks down the room and sends a "start game" message to all other
     * players.
     */
    void startGame();
    
};

#endif /* LobbyScene_h */
