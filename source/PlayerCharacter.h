//
//  PlayerCharacter.h
//
//  Author: High Rise Games
//
#ifndef __SHIP_H__
#define __SHIP_H__
#include <cugl/cugl.h>

/** The number of frames until we can fire again */
#define RELOAD_RATE 3

/**
 * Model class representing a player.
 */
class Player {
private:
    /** The player's id */
    int _id;
    /** Position of the player */
    cugl::Vec2 _pos;
    /** Velocity of the player */
    cugl::Vec2 _vel;
    /** Coordinates in relation to window grid of the player */
    cugl::Vec2 _coors;
    
    // TODO: do we still need window height and width?
    // height of a window pane of the game board
    // used to discretize movement
    float _windowHeight;
    
    // width of a window pane of the game board
    // used to discretize movement
    float _windowWidth;
       
    // TODO: remove unnecessary fields from here and constants json file
    // The following are protected, because they have no accessors
    /** Current angle of the ship */
    float _ang;
    /** Accumulator variable to turn faster as key is held down */
    float _dang;
    /** Countdown to limit refire rate */
    int _refire;
    /** The amount of health this ship has */
    int _health;
    /** The amount of time in frames for the player to be stunned */
    int _stunFrames;
    
    /** A property to adjust the rotation of the player when player colides. Resets to zero when stun frames is zero. */
    float _stunRotate;

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
     * Creates a player with the given fields.
     *
     * @param id    The player's id
     * @param pos   The player position
     * @param data  The data defining the constants
     * @param windowWidth   The width of the window panes
     * @param windowHeight  The height of the window panes
     */
    Player(const int id, const cugl::Vec2& pos, std::shared_ptr<cugl::JsonValue> data, const float windowWidth, const float windowHeight);
    
    /**
     * Disposes the ship, releasing all resources.
     */
    ~Player() {}

    
#pragma mark Properties

    /** Returns the id of the player. */
    const int getId() const { return _id; }

    /** Sets the id of the player. */
    void setId(int id) { _id = id; }

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
     * Returns the coordinates of the player in relation to the window grid.
     */
    const cugl::Vec2& getCoors() const { return _coors; }

    /** 
     * Sets the coordinates of the player in relation to the window grid.
     */
    void setCoors(cugl::Vec2 value) { _coors = value; }

    /** 
     * Calculates the coordinates of the player in relation to the window grid
     * using the scene position of the player (_pos). 
     */
    const cugl::Vec2& getCoorsFromPos(const float windowHeight, const float windowWidth, const float sideGap);
    
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
     * Returns the current player's health.
     * 
     * When the health of the player is 0, it is "dead"
     *
     * @return the current player health.
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
     * Returns the current player's stunned time in frames.
     */
    int getStunFrames() const { return _stunFrames; }

    /**
     * Sets the player's stun time to the given time in frames to stun the player.
     *
     * @param value The time in frames to stun the player.
     */
    void setStunFrames(int value) { _stunFrames = value; }

    /** Decreases the stun frames by one, unless it is already at 0 then does nothing. */
    void decreaseStunFrames();
    
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
     * @param dir        The unit vector of moving direction
     * @param size      The size of the window (for wrap around)
     * @return True if moved
     */
    bool move(cugl::Vec2 dir, cugl::Size size, float sideGap);
    
    /**
     * Returns edge if player is at the edge of building
     *
     * @return -1 if the player is at left edge, 0 not at edge, and 1 at right edge
     */
    int getEdge(float sideGap, cugl::Size size);
    
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

