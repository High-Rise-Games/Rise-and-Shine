//
//  CollisionController.h
//
//  Author: High Rise Games
//
#ifndef __COLLISION_CONTROLLER_H__
#define __COLLISION_CONTROLLER_H__
#include <cugl/cugl.h>
#include "PlayerCharacter.h"

/**
 * Namespace of functions implementing simple game physics.
 *
 * This is the simplest of physics engines. In reality, you will probably use
 * box2d just like you did in 3152.
 */
class CollisionController {
private:
    /** The window size (to support wrap-around collisions) */
    cugl::Size _size;

public:
    /**
     * Creates a new collision controller.
     *
     * You will notice this constructor does nothing.  That is because the
     * object is constructed the instance the game starts (main.cpp immediately
     * constructs ShipApp, which immediately constructs GameScene, which then
     * immediately constructs this class), before we know the window size.
     */
    CollisionController() {}
    
    /**
     * Deletes the collision controller.
     *
     * Not much to do here since there was no dynamic allocation.
     */
    ~CollisionController() {}
    
    /**
     * Initializes the collision controller with the given size.
     *
     * This initializer is where we can finally set the window size. This size
     * is used to manage screen wrap for collisions.
     *
     * The pattern we use in this class is that all initializers should return
     * a bool indicating whether initialization was successful (even for
     * initializers that always return true).
     *
     * @param size  The window size
     *
     * @return true if initialization was successful
     */
    bool init(cugl::Size size) {
        _size = size;
        return true;
    }
        


};

#endif /* __SL_COLLISION_CONTROLLER_H__ */