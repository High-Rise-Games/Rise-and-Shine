//
//  SLShip.h
//  Ship Lab
//
//  This class tracks all of the state (position, velocity, rotation) of a
//  single ship. In order to obey the separation of the model-view-controller
//  pattern, controller specific code (such as reading the keyboard) is not
//  present in this class.
//
//  With that said, you will notice several elements that look like they are
//  part of the view, namely the texture and sound.  But this is the texture
//  and sound DATA.  The actual view are the sprite batch and audio engine,
//  respectively, that use this data to provide feedback on the screen.
//
//  Author: Walker White
//  Based on original GameX Ship Demo by Rama C. Hoetzlein, 2002
//  Version: 1/20/22
//
#ifndef __GL_SHIP_H__
#define __GL_SHIP_H__
#include <cugl/cugl.h>

/** The number of frames until we can fire again */
#define RELOAD_RATE 3

/**
 * Model class representing an alien ship.
 */
class Player {
private:
    /** Position of the ship */
    cugl::Vec2 _pos;
    /** Velocity of the ship */
    cugl::Vec2 _vel;
        
    // The following are protected, because they have no accessors
    /** Current angle of the ship */
    float _ang;
    /** Accumulator variable to turn faster as key is held down */
    float _dang;
    /** Countdown to limit refire rate */
    int _refire;
    /** The amount of health this ship has */
    int _health;

    // JSON DEFINED ATTRIBUTES
    /** Mass/weight of the ship. Used in collisions. */
    float _mass;
    /** The number of frames until we can fire again */
    int _firerate;
    /** The number of columns in the sprite sheet */
    int _framecols;
    /** The number of frames in the sprite sheet */
    int _framesize;
    /** The sprite sheet frame for being at rest */
    int _frameflat;
    /** The shadow offset in pixels */
    float _shadows;
    /** Amount to adjust forward movement from input */
    float _thrust;
    /** The maximum allowable velocity */
    float _maxvel;
    /** The banking factor */
    float _banking;
    /** The maximum banking amount */
    float _maxbank;
    /** Amount to dampedn angular movement over time */
    float _angdamp;
    
    // Asset references. These should be set by GameScene
    /** Reference to the ships sprite sheet */
    std::shared_ptr<cugl::SpriteSheet> _sprite;
    /** Radius of the ship in pixels (derived from sprite sheet) */
    float _radius;

public:
#pragma mark Constructors
    /**
     * Creates a ship wiht the given position and data.
     *
     * The JsonValue should be a reference of all of the constants
     * that necessary to set the "hidden physical properties".
     *
     * @param pos   The ship position
     * @param data  The data defining the physics constants
     */
    Player(const cugl::Vec2& pos, std::shared_ptr<cugl::JsonValue> data);
    
    /**
     * Disposes the ship, releasing all resources.
     */
    ~Player() {}

    
#pragma mark Properties
    /**
     * Returns the position of this ship.
     *
     * This is location of the center pixel of the ship on the screen.
     *
     * @return the position of this ship
     */
    const cugl::Vec2& getPosition() const { return _pos; }
    
    /**
     * Sets the position of this ship.
     *
     * This is location of the center pixel of the ship on the screen.
     * Setting this value does NOT respect wrap around. It is possible
     * to use this method to place the ship off screen (so be careful).
     *
     * @param value the position of this ship
     */
    void setPosition(cugl::Vec2 value) { _pos = value; }
    
    /**
     * Sets the position of this ship, supporting wrap-around.
     *
     * This is the preferred way to "bump" a ship in a collision.
     *
     * @param value     The position of this ship
     * @param size      The size of the window (for wrap around)
     */
    void setPosition(cugl::Vec2 value, cugl::Vec2 size);
    
    /**
     * Returns the velocity of this ship.
     *
     * This value is necessary to control momementum in ship movement.
     *
     * @return the velocity of this ship
     */
    const cugl::Vec2& getVelocity() const { return _vel; }

    /**
     * Sets the velocity of this ship.
     *
     * This value is necessary to control momementum in ship movement.
     *
     * @param value the velocity of this ship
     */
    void setVelocity(cugl::Vec2 value) { _vel = value; }
    
    /**
     * Returns the angle that this ship is facing.
     *
     * The angle is specified in degrees. The angle is counter clockwise
     * from the line facing north.
     *
     * @return the angle of the ship
     */
    float getAngle() const { return _ang; }
    
    /**
     * Sets the angle that this ship is facing.
     *
     * The angle is specified in degrees. The angle is counter clockwise
     * from the line facing north.
     *
     * @param value the angle of the ship
     */
    void setAngle(float value) { _ang = value; }
    
    /**
     * Returns the current ship health.
     * 
     * When the health of the ship is 0, it is "dead"
     *
     * @return the current ship health.
     */
    int getHealth() const { return _health; }

    /**
     * Sets the current ship health.
     * 
     * When the health of the ship is 0, it is "dead"
     *
     * @param value The current ship health.
     */
    void setHealth(int value);
    
    /**
     * Returns true if the ship can fire its weapon
     *
     * Weapon fire is subjected to a cooldown. You can modify the
     * value "fire rate" in the JSON file to make this faster or slower.
     *
     * @return true if the ship can fire
     */
    bool canFireWeapon() const {
        return (_refire > _firerate);
    }
    
    /**
     * Resets the reload counter so the ship cannot fire again immediately.
     *
     * The ship must wait a number of frames before it can fire. This
     * value is set by "fire rate" in the JSON file
     */
    void reloadWeapon() {
        _refire = 0;
    }

    /**
     * Returns the mass of the ship.
     *
     * This value is necessary to resolve collisions. It is set by the
     * initial JSON file.
     *
     * @return the ship mass
     */
    float getMass() const {
        return _mass;
    }

    /**
     * Returns the radius of the ship.
     *
     * This value is necessary to resolve collisions. It is computed from
     * the sprite sheet.
     *
     * @return the ship radius
     */
    float getRadius() {
        return _radius;
    }
    
#pragma mark Graphics
    /**
     * Returns the sprite sheet for the ship
     *
     * The size and layout of the sprite sheet should already be specified
     * in the initializing JSON. Otherwise, the contents of the sprite sheet
     * will be ignored.     *
     * @return the sprite sheet for the ship
     */
    const std::shared_ptr<cugl::SpriteSheet>& getSprite() const {
        return _sprite;
    }

    /**
     * Sets the texture for this ship.
     *
     * The texture should be formated as a sprite sheet, and the size and 
     * layout of the sprite sheet should already be specified in the 
     * initializing JSON. If so, this method will construct a sprite sheet
     * from this texture. Otherwise, the texture will be ignored.
     *
     * @param texture   The texture for the sprite sheet
     */
    void setTexture(const std::shared_ptr<cugl::Texture>& texture);
    
    /**
     * Draws this ship to the sprite batch within the given bounds.
     *
     * This drawing code supports "wrap around". If the ship is partly off of
     * one edge, then it will also be drawn across the edge on the opposite
     * side.
     *
     * @param batch     The sprite batch to draw to
     * @param size      The size of the window (for wrap around)
     */
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size);

#pragma mark Movement
    /**
     * Moves the ship by the specified amount.
     *
     * Forward is the amount to move forward, while turn is the angle to turn
     * the ship (used for the "banking" animation). Turning is dampened so that
     * the ship does not turn forever. However, velocity has inertia and must
     * be counter-acted (as with the classics Asteroids game).
     *
     * This movement code supports "wrap around".  If the ship goes off one
     * edge of the screen, then it appears across the edge on the opposite
     * side. However, this method performs no collision detection. Collisions
     * are resolved afterwards.
     *
     * @param forward   Amount to move forward
     * @param turn      Amount to turn the ship
     * @param size      The size of the window (for wrap around)
     */
    void move(float forward, float turn, cugl::Size size, float sideGap);
    
private:
    /**
     * Update the animation of the ship to process a turn
     *
     * Turning changes the frame of the filmstrip, as we change from a level ship
     * to a hard bank. This method also updates the field dang cumulatively.
     *
     * @param turn Amount to turn the ship
     */
    void processTurn(float turn);
    
    /**
     * Applies "wrap around"
     *
     * If the ship goes off one edge of the screen, then it appears across the edge
     * on the opposite side.
     *
     * @param size      The size of the window (for wrap around)
     */
    void wrapPosition(cugl::Size size);

};

#endif /* __SL_SHIP_H__ */

