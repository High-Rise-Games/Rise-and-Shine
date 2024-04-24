//
//  LevelScene.h
//  Shine
//

#ifndef LevelScene_h
#define LevelScene_h
#include <cugl/cugl.h>
#include <vector>
#include "AudioController.h"

/**
 * This class presents the level select scene to the player.
 *
 * There is no need for an input controller, as all input is managed by
 * listeners on the scene graph.  We only need getters so that the main
 * application can retrieve the state and communicate it to other scenes.
 */
class LevelScene : public cugl::Scene2 {

public:
    /**
     * The level scene choice.
     *
     * This state allows the top level application to know what the user
     * chose.
     */
    enum Choice {
        /** User has not yet made a choice */
        NONE,
        /** User wants to back to menu scene */
        BACK,
        /** User wants to continue to lobby scene */
        NEXT
    };
    
protected:
    
    /** The audio controller, set ny app */
    std::shared_ptr<AudioController> _audioController;
    /** The asset manager for this scene. */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** The button for going back to character select scene */
    std::shared_ptr<cugl::scene2::Button> _backbutton;
    /** The button for continuing to character select scene */
    std::shared_ptr<cugl::scene2::Button> _nextbutton;
    /** The level buttons to be selected */
    std::vector<std::shared_ptr<cugl::scene2::Button>> _levelbuttons;
    /** The highlighted level texture which is selected */
    std::vector<std::shared_ptr<cugl::scene2::SceneNode>> _highlightedlevels;
    /** The player selected level, -1 if none chosen */
    int _selectedlevel;
    /** The player's action choice */
    Choice _choice;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new level scene with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    LevelScene() : cugl::Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~LevelScene() { dispose(); }
    
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
    
    /** Sets the pointer to the audio controller from app */
    void setAudioController(std::shared_ptr<AudioController> audioController) {_audioController = audioController;};

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
     * Returns the user's level choice.
     *
     * This will return 0 if the user had no yet made a choice.
     *
     * @return the user's level choice.
     */
    int getLevel() const { return _selectedlevel; }
    
    /**
     * Returns the user's action choice.
     *
     * This will return NONE if the user had no yet made a choice.
     *
     * @return the user's level choice.
     */
    Choice getChoice() const { return _choice; }

};

#endif /* LevelScene_h */
