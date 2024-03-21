//
//  GameplayController.h
//
//  Author: High Rise Games
//
#ifndef __GAME_CONTROLLER_H__
#define __GAME_CONTROLLER_H__
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
#include "GameAudioController.h"
#include "Bird.h"



/**
 * This class is the primary gameplay constroller for the demo.
 *
 * A world has its own objects, assets, and input controller.  Thus this is
 * really a mini-GameEngine in its own right.  As in 3152, we separate it out
 * so that we can have a separate mode for the loading screen.
 */
class GameplayController {
protected:
    /** The asset manager for this game mode. */
    std::shared_ptr<cugl::AssetManager> _assets;
        
    /** Whether this player is the host */
    bool _ishost;
    
    /** ID of the player to distinguish in multiplayer */
    int _id;
    /** Seconds left in the game */
    int _gameTime;
    
    /** The FPS of the game, as set by the App */
    int _fps;
    
    /** The current frame incremeted by 1 every frame (resets to 0 every time we reach 60 frames) */
    int _frame;

    /** Size of the scene */
    cugl::Size _size;
    
    
    
    // CONTROLLERS are attached directly to the scene (no pointers)
    /** The input controller to manage the character movement */
    InputController _input;
    /** The input controller to manage the dirt throw logic*/
    // DirtThrowInputController _dirtThrowInput;
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
    

    /** True if game scene is active and that gameplay is currently active */
    bool _isActive;
    
    /** Which board is the player currently on, 0 for his own board, -1 for left neighbor, 1 for right neighbor */

    /** Which board is the player currently on, 0 for own board, -1 for left neighbor, 1 for right neighbor */

    int _curBoard;
    /** Which board is the player currently on, 0 for own board, -1 for left neighbor, 1 for right neighbor */
    int _curBoardRight;
    /** Which board is the player currently on, 0 for own board, -1 for left neighbor, 1 for right neighbor */
    int _curBoardLeft;
    /** Which board the bird enemy is currently on, 0 for own board, -1 for left neighbor, 1 for right neighbor, 2 for none of these */
    int _curBirdBoard;
    /** The position of the bird enemy for drawing, if they are on your board */
    cugl::Vec2 _curBirdPos;
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
    /** Bird enemy for entire game */
    Bird _bird;
    /** True if bird exists in this level */;
    bool _birdActive;
    /** The current board that the bird is on */
    int _boardWithBird;
    
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
    
    /** Gameplay Audio Controller */
    GameAudioController _audioController;

    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new game mode with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    GameplayController() {}

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
    bool init(const std::shared_ptr<cugl::AssetManager>& assets, int fps, cugl::Rect bounds, cugl::Size size);

    /** Initializes the player models for all players, whether host or client. */
    bool initPlayers(const std::shared_ptr<cugl::AssetManager>& assets);
    
    /** Returns true if gameplay is active, and false if not. Tells us if the game is running */
    bool isActive() {
        return _isActive;
    }
    
    /** Sets the gameplay controller as active or inactive, letting us know if the game is in session */
    void setActive(bool f);
    

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

    /** Returns the id of this player. */
    const int getId() const { return _id; }

    /**
    * Sets the id of this player.
    * @param id     the id of this player to set
    */
    void setId(int id) { _id = id; }

    /** Returns the size of the scene */
    cugl::Size getSize() { return _size; }
    
    /** Returns the current dirt amount in the dirt bucket */
    int getCurDirtAmount() { return _currentDirtAmount; }

    /** Returns the current player's health */
    int getPlayerHealth() { return _player->getHealth(); }

    /** Returns the current game time */
    int getTime() { return _gameTime; }

    /** 
     * Given the world positions, convert it to the board position
     * based off of grid coordinates. Ex. [2, 3] or [2.3, 3] if the
     * player is in the process of moving in between x = 2 and x = 3.
     */
    cugl::Vec2 getBoardPosition(cugl::Vec2 worldPos);

    /**
     * Given the board positions, convert it to the world position.
     */
    cugl::Vec2 getWorldPosition(cugl::Vec2 boardPos);

    /**
     * Method for the scene switch listener used in GameScene
     */
    void switchScene();
    
    /** Checks whether board is full */
    const bool checkBoardFull(); // TODO: Unimplemented
    
    /** Checks whether board is empty */
    const bool checkBoardEmpty(); // TODO: Unimplemented
    
    /** update when dirt is generated */
    void updateDirtGenTime();
    
    /** generates dirt in a fair manner */
    void generateDirt();

    /** generates poo before bird is ready */
    void generatePoo(ProjectileSet* projectiles);

    /**
     * Called by host only. Converts game state into a JSON value for sending over the network
     * 
     * @param id    the id of the player of the board state to get
     * @returns JSON value representing game board state
     */
    std::shared_ptr<cugl::JsonValue> getJsonBoard(int id);

    /**
     * Called by client only. Converts a movement vector into a JSON value for sending over the network.
     * 
     * @param move    the movement vector
     * @returns JSON value representing a movement
     */
    std::shared_ptr<cugl::JsonValue> getJsonMove(const cugl::Vec2 move);

    /**
    * Called by the client only. Returns a JSON value representing a scene switch request
    * for sending over the network.
    *
    * @param returning  whether the player is returning to their board
    * @returns JSON value representing a scene switch
    */
    std::shared_ptr<cugl::JsonValue> getJsonSceneSwitch(bool returning);

    /**
     * Called by client only. Represents a dirt throw action as a JSON value for sending over the network.
     *
     * @param target The id of the player whose board the current player is sending dirt to
     * @param pos   The starting position of the dirt projectile
     * @param vel   The velocity vector of the dirt projectile
     * @param dest  The destination coordinates of the dirt projectile
     * 
     * @returns JSON value representing a dirt throw action
     */
    std::shared_ptr<cugl::JsonValue> getJsonDirtThrow(const int target, const cugl::Vec2 pos, const cugl::Vec2 vel, const cugl::Vec2 dest);

    /**
     * Updates a neighboring or own board given the JSON value representing its game state
     * 
     * @params data     The data to update
     */
    void updateBoard(std::shared_ptr<cugl::JsonValue> data);

    /**
     * Called by the host only. Updates a client player's board for player at player_id
     * based on the movement stored in the JSON value.
     *
     * @params data     The data to update
     */
    void processMovementRequest(std::shared_ptr<cugl::JsonValue> data);

    /** 
     * Called by host only to process switch scene requests. Updates a client player's
     * currently viewed board for the player at player_id based on the current board
     * value stored in the JSON value.
     * 
     * @params data     The data to update
     */
    void processSceneSwitchRequest(std::shared_ptr<cugl::JsonValue> data);

    /**
     * Called by host only. Updates the boards of both the dirt thrower and the player 
     * receiving the dirt projectile given the information stored in the JSON value.
     *
     * @params data     The data to update
     */
    void processDirtThrowRequest(std::shared_ptr<cugl::JsonValue> data);
    
    /**
     * The method called to update the game mode.
     *
     * This method contains any gameplay code that is not an OpenGL call.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     * @param worldPos  The position of the user's touch in world positions, used for dirt throwing
     * @param dirtCon   The dirt throw input controller used by the game scene
     */
    void update(float timestep, cugl::Vec2 worldPos, DirtThrowInputController& dirtCon);

    /**
     * This method does all the heavy lifting work for update.
     * The host steps forward each player's game state, given references to the player, board, and projectile set.
     */
    void stepForward(std::shared_ptr<Player>& player, WindowGrid& windows, ProjectileSet& projectiles);

    /**
     * Draws all this scene to the given SpriteBatch.
     *
     * The default implementation of this method simply draws the scene graph
     * to the sprite batch.  By overriding it, you can do custom drawing
     * in its place.
     *
     * @param batch     The SpriteBatch to draw with.
     */
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch);
    
    /**
     * Sets whether the player is host.
     *
     * We may need to have gameplay specific code for host.
     *
     * @param host  Whether the player is host.
     */
    void setHost(bool host) { _ishost = host; }
    
    /**
     * Resets the status of the game so that we can play again.
     */
    void reset();

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

#endif __GAME_CONTROLLER_H__