//
//  TutorialController.h
//
//  Author: High Rise Games
//
#ifndef __TUTORIAL_CONTROLLER_H__
#define __TUTORIAL_CONTROLLER_H__
#include <cugl/cugl.h>
#include <vector>
#include <unordered_set>

// models
#include "PlayerCharacter.h"
#include "WindowGrid.h"
#include "ProjectileSet.h"
#include "Bird.h"

// controllers
#include "InputController.h"
#include "DirtThrowInputController.h"
#include "CollisionController.h"
#include "AudioController.h"
#include "GameplayController.h"


/**
 * This class is the primary gameplay constroller for the demo.
 *
 * A world has its own objects, assets, and input controller.  Thus this is
 * really a mini-GameEngine in its own right.  As in 3152, we separate it out
 * so that we can have a separate mode for the loading screen.
 */
class TutorialController : public GameplayController {
protected:

    std::vector<int> dirt_x_values = { 2, 1, 1, 0, 2 };
    std::vector<int> dirt_y_values = { 0, 2, 3, 5, 5 };


public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new game mode with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    TutorialController() {}


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
    bool initHost(const std::shared_ptr<cugl::AssetManager>& assets) override;


#pragma mark Graphics

#pragma mark Gameplay Handling

    /**
    * Initializes the level of this game session.
    * @param assets     game assets
    * @param level     the leve of this game
    * Returns true if level set up is successful
    */
    bool initLevel(int level) override;

    /**
    * Sets the character of your player (Mushroom) and your opponent
    * (Flower) for the tutorial.
    */
    void setCharacters(std::vector<std::string>& chars) override;

    /**
     * The method called to update the game mode.
     *
     * This method contains any gameplay code that is not an OpenGL call.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     * @param worldPos  The position of the user's touch in world positions, used for dirt throwing
     * @param dirtCon   The dirt throw input controller used by the game scene
     * @param dirtThrowButton   The dirt throw button from the game scene
     * @param dirtThrowArc   The dirt throw arc from the game scene
     */
    void update(float timestep, cugl::Vec2 worldPos, DirtThrowInputController& dirtCon, std::shared_ptr<cugl::scene2::Button> dirtThrowButton, std::shared_ptr<cugl::scene2::SceneNode> dirtThrowArc) override;

    /**
     * Draws all this scene to the given SpriteBatch.
     *
     * The default implementation of this method simply draws the scene graph
     * to the sprite batch.  By overriding it, you can do custom drawing
     * in its place.
     *
     * @param batch     The SpriteBatch to draw with.
     */
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;

    /**
     * Resets the status of the game so that we can play again.
     */
    void hostReset() override;
};

#endif __TUTORIAL_CONTROLLER_H__
