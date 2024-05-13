//
//  SettingsScene.hpp
//  Shine
//
//  Created by Troy Moslemi on 5/12/24.
//

#ifndef SettingsScene_h
#define SettingsScene_h

#include <stdio.h>

#include <cugl/cugl.h>
#include <vector>
#include "AudioController.h"

using namespace cugl;
using namespace std;

/**
 * This class presents the client join scene to the player.
 *
 * There is no need for an input controller, as all input is managed by
 * listeners on the scene graph.  We only need getters so that the main
 * application can retrieve the state and communicate it to other scenes.
 */
class SettingsScene : public cugl::Scene2 {
    
protected:
    

    /** The asset manager for this scene. */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    
    
    


public:
    
    /** The scene UI */
    std::shared_ptr<cugl::scene2::SceneNode> _settingsUI;
    
    
    /**
     * Creates a new SettingsScene scene with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    SettingsScene() : cugl::Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~SettingsScene() { dispose(); }

    
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
     * Sets whether the scene is currently active
     *
     * This method should be used to toggle all the UI elements.  Buttons
     * should be activated when it is made active and deactivated when
     * it is not.
     *
     * @param value whether the scene is currently active
     */
    virtual void setActive(bool value) override;
    

};


#endif /* SettingsScene_h */
