//
//  VictoryScene.h
//  Shine
//

#ifndef VictoryScene_h
#define VictoryScene_h
#include <cugl/cugl.h>
#include "AudioController.h"
#include "GameplayController.h"

class VictoryScene : public cugl::Scene2 {
    
public:
    /** The audio controller, set ny app */
    std::shared_ptr<AudioController> _audioController;
    /** The asset manager for this scene. */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** The entire node of the victory scene*/
    std::shared_ptr<cugl::scene2::SceneNode> _scene;
    /** The button for going back to character select scene */
    std::shared_ptr<cugl::scene2::Button> _backbutton;
    /** The building image */
    std::shared_ptr<cugl::Texture> _building;
    
    /** Winner text textures */
    std::shared_ptr<cugl::Texture> _winnerRedText;
    std::shared_ptr<cugl::Texture> _winnerBlueText;
    std::shared_ptr<cugl::Texture> _winnerYellowText;
    std::shared_ptr<cugl::Texture> _winnerGreenText;
    
    
//    std::shared_ptr<cugl::SpriteSheet> winnerTexture;
//    std::shared_ptr<cugl::SpriteSheet> loserTexture;
    /** The mushroom image as winner*/
    std::shared_ptr<cugl::SpriteSheet> _winnerMushroom;
    /** The frog image as winner*/
    std::shared_ptr<cugl::SpriteSheet> _winnerFrog;
    /** The charmeleon image as winner*/
    std::shared_ptr<cugl::SpriteSheet> _winnerChameleon;
    /** The flower image as winner*/
    std::shared_ptr<cugl::SpriteSheet> _winnerFlower;
    /** The mushroom image as loser*/
    std::shared_ptr<cugl::SpriteSheet> _loserMushroom;
    /** The frog image as loser*/
    std::shared_ptr<cugl::SpriteSheet> _loserFrog;
    /** The charmeleon image as loser*/
    std::shared_ptr<cugl::SpriteSheet> _loserChameleon;
    /** The flower image as loser*/
    std::shared_ptr<cugl::SpriteSheet> _loserFlower;
    int animFrameCounter;
    int loseFrameCounter;
    /** Whether the back button is pressed*/
    bool _quit;
    /** The winner character */
    std::string _winnerChar;
    /** Other player character */
    std::vector<std::string> _otherChars;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new victory scene with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    VictoryScene() : cugl::Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~VictoryScene() { dispose(); }
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose() override;
    
    /**
     * Initializes the controller contents.
     *
     * In previous labs, this method "started" the scene.  But in this
     * case, we only use to initialize the scene user interface.  We
     * do not activate the user interface yet, as an active user
     * interface will still receive input EVEN WHEN IT IS HIDDEN.
     *
     * That is why we have the method {@link #setActive}.
     *
     * @param assets    The (loaded) assets for this game mode
     *
     * @return true if the controller is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    
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
    
    /** Sets the pointer to the audio controller from app */
    void setAudioController(std::shared_ptr<AudioController> audioController) {_audioController = audioController;};
    
    /** Set the winner and other playing in the game */
    void setCharacters(std::shared_ptr<GameplayController> gameplay);
    
    /**
     * Sets whether the scene is currently active
     *
     * This method should be used to toggle all the UI elements.  Buttons
     * should be activated when it is made active and deactivated when
     * it is not.
     *
     * @param value whether the scene is currently active
     */
    virtual void setActive(bool value) override;
    
    /**
     * Get if the player has quit the victory scene
     * @return true if player press the back button
     */
    bool didQuit() const { return _quit; }
};

#endif /* VictoryScene_h */
