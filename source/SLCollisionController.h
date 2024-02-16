//
//  SLCollisionController.h
//  Programming Lab
//
//  Unless you are making a point-and-click adventure game, every single
//  game is going to need some sort of collision detection.  In a later
//  lab, we will see how to do this with a physics engine. For now, we use
//  custom physics.
//
//  You might ask why we need this file when we have Box2d. That is because
//  we are trying to make this code as close to that of 3152 as possible. At
//  this point in the semester of 3152, we had not covered Box2d.
//
//  As you work on this class, there is an interesting thing to note.  The
//  constructor and the initializer are SEPARATE.  That is because the
//  constructor is called as soon as the object is created and that happens
//  BEFORE we know the window size.
//
//  Author: Walker M. White
//  Based on original GameX Ship Demo by Rama C. Hoetzlein, 2002
//  Version: 1/20/22
//
#ifndef __SL_COLLISION_CONTROLLER_H__
#define __SL_COLLISION_CONTROLLER_H__
#include <cugl/cugl.h>
#include "SLShip.h"
#include "SLAsteroidSet.h"

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
        
    /**
     * Returns true if there is a ship-asteroid collision
     *
     * In addition to checking for the collision, this method also resolves it.
     * That means it applies damage to the ship for EACH asteroid encountered.
     * It does not, however, play the sound. That happens in the main controller
     *
     * Note that this method must take wrap into consideration as well. If the
     * asteroid/ship can be drawn at multiple points on the screen, then it can
     * collide at multiple places as well.
     *
     * @param ship  The players ship
     * @param aset  The asteroid set
     *
     * @return true if there is a ship-asteroid collision
     */
    bool resolveCollision(const std::shared_ptr<Player>& ship, AsteroidSet& ast);

};

#endif /* __SL_COLLISION_CONTROLLER_H__ */
