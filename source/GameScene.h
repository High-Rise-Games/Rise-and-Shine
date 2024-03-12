//
//  GameScene.h
//
//  Author: High Rise Games
//
#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__
#include <cugl/cugl.h>
#include <vector>
#include <unordered_set>
#include "PlayerCharacter.h"
#include "InputController.h"
#include "DirtThrowInputController.h"
#include "CollisionController.h"
#include "WindowGrid.h"
#include "ProjectileSet.h"
#include "NetworkController.h"


/**
 * This class is the primary gameplay constroller for the demo.
 *
 * A world has its own objects, assets, and input controller.  Thus this is
 * really a mini-GameEngine in its own right.  As in 3152, we separate it out
 * so that we can have a separate mode for the loading screen.
 */
class GameScene : public cugl::Scene2 {
protected:
    /** The asset manager for this game mode. */
    std::shared_ptr<cugl::AssetManager> _assets;
        
    /** Whether this player is the host */
    bool _ishost;

    /** Whether we quit the game */
    bool _quit;
    
    /** ID of the player to distinguish in multiplayer */
    int _id;
    /** Seconds left in the game */
    int _gameTime;
    
    /** The FPS of the game, as set by the App */
    int _fps;
    
    /** The current frame incremeted by 1 every frame (resets to 0 every time we reach 60 frames) */
    int _frame;
    
    
    
    // CONTROLLERS are attached directly to the scene (no pointers)
    /** The input controller to manage the character movement */
    InputController _input;
    /** The input controller to manage the dirt throw logic*/
    DirtThrowInputController _dirtThrowInput;
    /** The controller for managing collisions */
    CollisionController _collisions;
    /** The controller for managing network data */
    NetworkController _network;
    
    
    // MODELS should be shared pointers or a data structure of shared pointers
    /** The JSON value with all of the constants */
    std::shared_ptr<cugl::JsonValue> _constants;

    /** Location and animation information for the player */
    std::shared_ptr<Player> _player;
    /** Location and animation information for the player to the left */
    std::shared_ptr<Player> _playerLeft;
    /** Location and animation information for the player to the right */
    std::shared_ptr<Player> _playerRight;
    /** Location and animation information for the player across the building. Only non-null for host. */
    std::shared_ptr<Player> _playerAcross;

    /** True if a neighobr player's board is on display */
    bool _onAdjacentBoard;
    
    /** Which board is the player currently on, 0 for his own board, -1 for left neighbor, 1 for right neighbor */
    int _curBoard;
    /** Whether the dirt is selected, ONLY active when currently on others board*/
    bool _dirtSelected;
    /** The path from player to the dirt throw destination, ONLY active when currently on player's own board*/
    cugl::Path2 _dirtPath;
    /** The position of the dirt when it is selected*/
    cugl::Vec2 _prevDirtPos;
    
    /** Todo: probably need to change _windows to a vector, length 3 or 4*/
    /** Grid of windows and dirt placement to be drawn */
    WindowGrid _windows;
    /** Grid of windows and dirt placement to be drawn for the left neighbor */
    WindowGrid _windowsLeft;
    /** Grid of windows and dirt placement to be drawn for the right neighbor */
    WindowGrid _windowsRight;
    /** Grid of windows and dirt placement to be drawn for neighbor across the building. Only non-null for host. */
    WindowGrid _windowsAcross;

    /** Random number generator for dirt generation */
    std::mt19937 _rng;
    /** Dirt random generation time stamp*/
    std::set<int> _dirtGenTimes;
    /** Dirt generation speed, equals number of random dirt generated per _fixedDirtUpdateThreshold period*/
    int _dirtGenSpeed;
    /** Current timer value for dirt regeneration. Increments up to _fixedDirtUpdateThreshold and resets to 0*/
    int _dirtThrowTimer;
    /** Timer threshold for fixed period random dirt generation in frames. E.g. 300 is one dirt generation per 5 seconds */
    int _fixedDirtUpdateThreshold;
    /** The max amount of dirt the bucket can hold **/
    int _maxDirtAmount;
    /** The amount of dirt player is currently holdinfg in the bucket **/
    int _currentDirtAmount;

    /** Projectile generation chance, increases over time */
    float _projectileGenChance;
    /** Projectile generation timer, on timer generate projectile based on chance, then reset timer*/
    float _projectileGenCountDown;
    /** The projectile set of your board */
    ProjectileSet _projectiles;
    /** The projectile set of your left neighbor */
    ProjectileSet _projectilesLeft;
    /** The projectile set of your right neighbor */
    ProjectileSet _projectilesRight;
    /** The projectile set of the neighbor across the building. Only non-null if host */
    ProjectileSet _projectilesAcross;
    
    // for host only
    /** Number of players in the lobby */
    int _numPlayers;
    /** The amount of dirt held by each player in the lobby */
    std::vector<int> _allDirtAmounts;
    /** The current board being displayed for each player in the lobby */
    std::vector<int> _allCurBoards;
    
    cugl::scheduable t;
    
    // VIEW items are going to be individual variables
    // In the future, we will replace this with the scene graph
    /** The backgrounnd image */
    std::shared_ptr<cugl::Texture> _background;
    /** The text with the current health */
    std::shared_ptr<cugl::TextLayout> _healthText;
    /** The text with the current time */
    std::shared_ptr<cugl::TextLayout> _text;
    /** Empty bucket texture image */
    std::shared_ptr<cugl::Texture> _emptyBucket;
    /** Full bucket texture image */
    std::shared_ptr<cugl::Texture> _fullBucket;
    /** The text with the current dirt */
    std::shared_ptr<cugl::TextLayout> _dirtText;

    /** The scene node for the UI elements (buttons, labels) */
    std::shared_ptr<cugl::scene2::SceneNode> _scene_UI;
    /** The back button for the menu scene */
    std::shared_ptr<cugl::scene2::Button> _backout;
//    /** Switch scene button texture image */
//    std::shared_ptr<cugl::Texture> _switchSceneButton;
//    /** Return scene button texture image */
//    std::shared_ptr<cugl::Texture> _returnSceneButton;
    /** Switch scene button */
    std::shared_ptr<cugl::scene2::Button> _tn_button;

    /** The sound of a ship-asteroid collision */
    std::shared_ptr<cugl::Sound> _bang;

    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new game mode with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    GameScene() : cugl::Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~GameScene() { dispose(); }
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose() override;

    /**
     * Sets whether the scene is currently active
     *
     * This method should be used to toggle all the UI elements.  Buttons
     * should be activated when it is made active and deactivated when
     * it is not.
     *
     * @param value whether the scene is currently active
     */
    void setActive(bool value) override;
    
    /**
     * Initializes the controller contents, and starts the game
     *
     * The constructor does not allocate any objects or memory.  This allows
     * us to have a non-pointer reference to this controller, reducing our
     * memory allocation.  Instead, allocation happens in this method.
     *
     * @param assets    The (loaded) assets for this game mode
     *
     * @return true if the controller is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets, int fps);

    /** Initializes the player models for all players, whether host or client. */
    bool initPlayers(const std::shared_ptr<cugl::AssetManager>& assets);

    /**
     * Initializes the extra controllers needed for the host of the game.
     *
     * The constructor does not allocate any objects or memory.  This allows
     * us to have a non-pointer reference to this controller, reducing our
     * memory allocation.  Instead, allocation happens in this method.
     *
     * @param assets    The (loaded) assets for this game mode
     *
     * @return true if the controller is initialized properly, false otherwise.
     */
    bool initHost(const std::shared_ptr<cugl::AssetManager>& assets);

    
#pragma mark -
#pragma mark Gameplay Handling

    /**
    * Returns the id of this player.
    * @return the id of this player
    */
    const int getId() const { return _id; }
    
    

    /**
    * Sets the id of this player.
    * @param id     the id of this player to set
    */
    void setId(int id) { _id = id; }
    
    /** sets empty bucket texture */
    void setEmptyBucket(const std::shared_ptr<cugl::Texture>& value) { _emptyBucket = value; }
    /** sets full bucket texture */
    void setFullBucket(const std::shared_ptr<cugl::Texture>& value) { _fullBucket = value; }
//    /** sets switch scene button texture */
//    void setSwitchSceneButton(const std::shared_ptr<cugl::Texture>& value) { _switchSceneButton = value; }
//    /** sets return scene button texture */
//    void setReturnSceneButton(const std::shared_ptr<cugl::Texture>& value) { _returnSceneButton = value; }
    
    /**
    * Called by the client only. Client calls this function to transmit message
    * to request from the host to switch scenes.
    *
    * Player ids assigned clockwise with host at top
    *
    *          host: 1
    * left: 4            right: 2
    *         across: 3
    *
     * Example Scene Request Message:
     * {
     *    "player_id":  1,
     *    "vel": [0.234, 1.153]
     * }
    * @param returning  whether the player is returning to their board
    */
    void SceneSwitchRequest(bool returning);
    
    
    /** Host function to process switch scene requests */
    void ProcessSceneSwitchRequest(std::shared_ptr<cugl::JsonValue> data);
    
    /** Checks whether board is full */
    const bool checkBoardFull();
    
    /** Checks whether board is empty */
    const bool checkBoardEmpty();
    
    /** update when dirt is generated */
    void updateDirtGenTime();
    
    /** generates dirt in a fair manner */
    void generateDirt();

    /** generates poo before bird is ready */
    void generatePoo();
    /**
     * Converts game state into a JSON value for sending over the network
     * 
     * @param id    the id of the player of the board state to get
     * @returns JSON value representing game board state
     */
    std::shared_ptr<cugl::JsonValue> getJsonBoard(int id);

    /**
     * Converts a movement vector into a JSON value for sending over the network.
     * 
     * @param move    the movement vector
     * @returns JSON value representing a movement
     */
    std::shared_ptr<cugl::JsonValue> getJsonMove(const cugl::Vec2 move);

    /**
     * Updates a neighboring or own board given the JSON value representing its game state
     * 
     * @params data     The data to update
     */
    void updateBoard(std::shared_ptr<cugl::JsonValue> data);

    /**
     * Called by the host only. Updates a client player's board for player at player_id
     * based on the movement or other action data stored in the JSON value.
     *
     * @params data     The data to update
     */
    void updateFromAction(std::shared_ptr<cugl::JsonValue> data);
    
    /**
     * The method called to update the game mode.
     *
     * This method contains any gameplay code that is not an OpenGL call.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void update(float timestep) override;

    /**
     * This method does all the heavy lifting work for update.
     * The host steps forward each player's game state, given references to the player, board, and projectile set.
     */
    void stepForward(std::shared_ptr<Player>& player, const cugl::Vec2 moveVec, WindowGrid& windows, ProjectileSet& projectiles);

    /**
     * Draws all this scene to the given SpriteBatch.
     *
     * The default implementation of this method simply draws the scene graph
     * to the sprite batch.  By overriding it, you can do custom drawing
     * in its place.
     *
     * @param batch     The SpriteBatch to draw with.
     */
    void render(const std::shared_ptr<cugl::SpriteBatch>& batch) override;
    
    /**
     * Sets whether the player is host.
     *
     * We may need to have gameplay specific code for host.
     *
     * @param host  Whether the player is host.
     */
    void setHost(bool host) { _ishost = host; }
    
    /**
     * Returns true if the player quits the game.
     *
     * @return true if the player quits the game.
     */
    bool didQuit() const { return _quit; }

    /**
     * Resets the status of the game so that we can play again.
     */
    void reset() override;

    /**
     * Resets the status of the game so that we can play again.
     */
    void hostReset();

    /**
     * Sets the network connection for this scene's network controller.
     */
    void setConnection(const std::shared_ptr<cugl::net::NetcodeConnection>& network) {
        _network.setConnection(network);
    }

    /**
     * Disconnects this scene from the network controller.
     */
    void disconnect() { _network.disconnect(); }
};

#endif /* __SG_GAME_SCENE_H__ */
