//
//  GameScene.h
//
//  Author: High Rise Games
//
#ifndef __TUTORIAL_SCENE_H__
#define __TUTORIAL_SCENE_H__
#include <cugl/cugl.h>
#include <vector>
#include <unordered_set>

#include "TutorialController.h"
#include "DirtThrowInputController.h"


/**
 * This class is the primary gameplay constroller for the demo.
 *
 * A world has its own objects, assets, and input controller.  Thus this is
 * really a mini-GameEngine in its own right.  As in 3152, we separate it out
 * so that we can have a separate mode for the loading screen.
 */
class TutorialScene : public cugl::Scene2 {
protected:
    /** The asset manager for this game mode. */
    std::shared_ptr<cugl::AssetManager> _assets;
        
    /** Whether this player is the host */
    bool _ishost;

    /** Whether we quit the game */
    bool _quit;
    
    /** ID of the player to distinguish in multiplayer */
    int _id;
    
    /** The FPS of the game, as set by the App */
    int _fps;
    
    /** The current frame incremeted by 1 every frame (resets to 0 every time we reach 60 frames) */
    int _frame;

    int _countdownFrame;
    
    /** The win screen scene */
    std::shared_ptr<cugl::scene2::SceneNode> _winBackground;
    /** The win screen scene */
    std::shared_ptr<cugl::scene2::SceneNode> _loseBackground;

    /** The game controller for this scene */
    std::shared_ptr<TutorialController> _gameController;
    /** The input controller to manage the dirt throw logic*/
    DirtThrowInputController _dirtThrowInput;

    // MODELS should be shared pointers or a data structure of shared pointers
    /** The JSON value with all of the constants */
    std::shared_ptr<cugl::JsonValue> _constants;
    
    cugl::scheduable t;
    
    /** Progress bars vector for players in the lobby **/
    std::vector<std::shared_ptr<cugl::scene2::ProgressBar>>  _player_bars;
    std::map<std::string, int> _char_to_barIdx;

    std::shared_ptr<cugl::scene2::SceneNode> _profilePlayer;
    std::shared_ptr<cugl::scene2::SceneNode> _profilePlayerLeft;
    std::shared_ptr<cugl::scene2::SceneNode> _profilePlayerRight;
    std::shared_ptr<cugl::scene2::SceneNode> _profilePlayerAcross;
    
    // VIEW items are going to be individual variables
    // In the future, we will replace this with the scene graph
    
    /** The text bubble image */
    std::shared_ptr<cugl::Texture> _textBubble;
    /** The mushroom pointing image */
    std::shared_ptr<cugl::Texture> _mushroomPoint;
    /** The text layout for text bubble */
    std::shared_ptr<cugl::TextLayout> _textOnBubble;
    /** The backgrounnd image */
    std::shared_ptr<cugl::Texture> _background;
    /** The parallax image */
    std::shared_ptr<cugl::Texture> _parallax;
    /** The text with the current health */
    std::shared_ptr<cugl::TextLayout> _healthText;
    /** The text with the current time */
    std::shared_ptr<cugl::TextLayout> _timeText;
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
    /** Dirt throw button */
    std::shared_ptr<cugl::scene2::Button> _dirtThrowButton;
    /** The semicircle that constains the dirt throw button*/
    std::shared_ptr<cugl::scene2::SceneNode> _dirtThrowArc;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new game mode with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    TutorialScene() : cugl::Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~TutorialScene() { dispose(); }
    
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


    
#pragma mark -
#pragma mark Gameplay Handling
    
    /**
     void renderCountdown(std::shared_ptr<cugl::SpriteBatch> batch);
     
     * The method called to update the game mode.
     *
     * This method contains any gameplay code that is not an OpenGL call.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void update(float timestep) override;

    /** method for drawing the countdown upon game start */
    void renderCountdown(std::shared_ptr<cugl::SpriteBatch> batch);

    /** sets empty bucket texture */
    void setEmptyBucket(const std::shared_ptr<cugl::Texture>& value) { _emptyBucket = value; }
    /** sets full bucket texture */
    void setFullBucket(const std::shared_ptr<cugl::Texture>& value) { _fullBucket = value; }

    /**
     * Sets whether the player is host.
     *
     * We may need to have gameplay specific code for host.
     *
     * @param host  Whether the player is host.
     */
    void setHost(bool host) { _ishost = host; }

    /** Sets the controller for this scene */
    void setController(std::shared_ptr<TutorialController>& gc) { _gameController = gc; }

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
     * Draws a text prompt.

     *
     * @param text     The text to draw the  prompt with
     * @param batch     The sprite batch to draw on
     *
     */
    void drawPrompt(std::string text, const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Vec2 location);
    
    
    /**
     * Returns true if the player quits the game.
     *
     * @return true if the player quits the game.
     */
    bool didQuit() const { return _quit; }

    /**
     * Resets the status of the game so that we can play again.
     */
    // void reset() override;

};

#endif __TUTORIAL_SCENE_H__
