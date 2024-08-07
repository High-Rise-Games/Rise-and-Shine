//
//  PlayerCharacter.h
//
//  Author: High Rise Games
//
#ifndef __SHIP_H__
#define __SHIP_H__
#include <cugl/cugl.h>
#include "WindowGrid.h"

/** The number of frames until we can fire again */
#define RELOAD_RATE 3

/**
 * Model class representing a player.
 */
class Player {
    
public:
    
    enum AnimStatus {
        /** Character in idle state */
        IDLE,
        /** Character in wiping state */
        WIPING,
        /** Character in shooing bird state */
        SHOOING,
        /** Character in stunned state */
        STUNNED,
        /** Character in throwing state */
        THROWING,
    };
    
    
    const std::vector<AnimStatus> animStatusNames = { IDLE, WIPING, SHOOING, STUNNED, THROWING };
    std::map<AnimStatus, int> statusToInt;
    
private:
    /** The player's id */
    int _id;
    /** The player's selected character */
    std::string _character;
    /** Player movement speed*/
    float _speed;
    /** Player distance to target of movement, set to 0 when reaches target*/
    cugl::Vec2 _targetDist;
    /** Position of the player */
    cugl::Vec2 _pos;
    /** Velocity of the player */
    cugl::Vec2 _vel;
    /** Coordinates in relation to window grid of the player */
    cugl::Vec2 _coors;
    /** Character Animation State */
    AnimStatus _animState;
    
    // TODO: do we still need window height and width?
    // height of a window pane of the game board
    // used to discretize movement
    float _windowHeight;
    
    // width of a window pane of the game board
    // used to discretize movement
    float _windowWidth;
    
    /** The amount of time in frames for the player to be stunned */
    int _stunFrames;
    
    /** A property to adjust the rotation of the player when player colides. Resets to zero when stun frames is zero. */
    float _stunRotate;
    
    /** The shadow offset in pixels */
    float _shadows;
    // Asset references. These should be set by GameScene
    /** The number of columns in the sprite sheet */
    int _framecols;
    /** The number of frames in the sprite sheet */
    int _framesize;
    /** total number of frames*/
    int _maxwipeFrame;
    // number of frames that the player is wiping for
    int _wipeFrames;
    /** The number of columns in the sprite sheet */
    int _idleframecols;
    /** The number of frames in the sprite sheet */
    int _idleframesize;
    /** total number of frames */
    int _maxidleFrame;
    // number of frames that the player is idling for
    int _idleFrames;
    /** The number of columns in the sprite sheet */
    int _shooframecols;
    /** The number of frames in the sprite sheet */
    int _shooframesize;
    /** total number of frames */
    int _maxshooFrame;
    // number of frames that the player is shooing
    int _shooFrames;
    /** The number of columns in the throw sprite sheet */
    int _throwframecols;
    /** The number of frames in the throw sprite sheet */
    int _throwframesize;
    /** total number of frames */
    int _maxthrowFrame;
    // number of frames that the player is throwing
    int _throwFrames;

    /** player profile texture */
    std::shared_ptr<cugl::Texture> _profileTexture;
    /** player idle sprite sheet */
    std::shared_ptr<cugl::SpriteSheet> _idleSprite;
    /** Reference to the player wiping animation sprite sheet */
    std::shared_ptr<cugl::SpriteSheet> _wipeSprite;
    /** Reference to the player shooing animation sprite sheet */
    std::shared_ptr<cugl::SpriteSheet> _shooSprite;
    /** Reference to the player throwing animation sprite sheet */
    std::shared_ptr<cugl::SpriteSheet> _throwSprite;
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
    Player(const int id, const cugl::Vec2& pos, const float windowWidth, const float windowHeight);
    
    /**
     * Disposes the ship, releasing all resources.
     */
    ~Player() {}

    
#pragma mark Properties

    /** Returns the id of the player. */
    const int getId() const { return _id; }

    /** Sets the id of the player. */
    void setId(int id) { _id = id; }

    /** Returns the character of this player. Frog, Mushroom, Flower, Chameleon. */
    const std::string getChar() { return _character; }

    /** Sets the character of the player. */
    void setChar(std::string c) { CULog("here: %a", c.c_str()); _character = c; }

    /** Sets the profile texture of the player */
    void setProfileTexture(std::shared_ptr<cugl::Texture> t) { _profileTexture = t; }
    
    /** Gets the profile texture of the player */
    std::shared_ptr<cugl::Texture> getProfileTexture() { return _profileTexture; }

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
    
    void setAnimationState(AnimStatus as) {
        if (as != _animState) {
            resetAnimationFrames();
            // _throwing = true;
            _animState = as;
        } 
    };
    
    AnimStatus getAnimationState() { return _animState; };

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
    
    /**
     * Sets the player's wiping time to the initial frame to play and freeze the player.
     *
     * @param value The time in frames to stun the player.
     */
    void resetAnimationFrames() {
        _wipeFrames = 0;
        _shooFrames = 0;
        _stunFrames = 60;
        _throwFrames = 0;
    }
    
    /**
     * Sets the player's movement freeze time to the given time in frames
     * .Used when player wipes dirt
     */
    void advanceWipeFrame();
    
    /**
     * Sets the player's movement freeze time to the given time in frames
     * .Used when player shoos bird
     */
    void advanceShooFrame();
    
    /**
     * Sets the player's movement freeze time to the given time in frames
     * .Used when player throws projectile
     */
    void advanceThrowFrame();
    
    /**
     * Advance animation for player idle
     */
    void advanceIdleFrame();
    
    void advanceAnimation();

    /** Decreases the stun frames by one, unless it is already at 0 then does nothing. */
    void decreaseStunFrames();

    /**
     * Returns the radius of the ship.
     *
     * This value is necessary to resolve collisions. It is computed from
     * the sprite sheet.
     *
     * @return the player character radius
     */
    float getRadius() {
        return _radius;
    }
    
#pragma mark Graphics
    /**
     * Returns the idle texture for the player
     *
     * //The size and layout of the sprite sheet should already be specified
     * //in the initializing JSON. Otherwise, the contents of the sprite sheet
     * //will be ignored.     *
     * @return the texture for the ship
     */
    const std::shared_ptr<cugl::SpriteSheet>& getIdleSprite() const {
        return _idleSprite;
    }

    /**
     * Sets the idle texture for the player.
     *
     * The texture should be formated as a sprite sheet, and the size and
     * layout of the sprite sheet should already be specified in the
     * initializing JSON. If so, this method will construct a sprite sheet
     * from this texture. Otherwise, the texture will be ignored.
     *
     * @param texture   The texture for the sprite sheet
     */
    void setIdleTexture(const std::shared_ptr<cugl::Texture>& texture);
    
    /** Gets player wipe sprite.
     *
     * The size and layout of the sprite sheet should already be specified
     * in the initializing step.
     * @return the sprite sheet for the ship
     */
    const std::shared_ptr<cugl::SpriteSheet>& getWipeSprite() const {
        return _wipeSprite;
    }
    
    /**
     * Sets the player wipe sprite.
     *
     * The texture should be formated as a sprite sheet, and the size and
     * layout of the sprite sheet should already be specified in the
     * initializing step. If so, this method will construct a sprite sheet
     * from this texture.
     * @param texture   The texture for the sprite sheet
     */
    void setWipeTexture(const std::shared_ptr<cugl::Texture>& texture);
    
    
    /** Gets player shoo sprite.
     *
     * The size and layout of the sprite sheet should already be specified
     * in the initializing step.
     * @return the sprite sheet for the ship
     */
    const std::shared_ptr<cugl::SpriteSheet>& getShooSprite() const {
        return _shooSprite;
    }
    
    /**
     * Sets the player shoo sprite.
     *
     * The texture should be formated as a sprite sheet, and the size and
     * layout of the sprite sheet should already be specified in the
     * initializing step. If so, this method will construct a sprite sheet
     * from this texture.
     * @param texture   The texture for the sprite sheet
     */
    void setShooTexture(const std::shared_ptr<cugl::Texture>& texture);

   /** Gets player dirt throwing sprite.
    *
    * The size and layout of the sprite sheet should already be specified
    * in the initializing step.
    *
    * @return the sprite sheet
    */
    const std::shared_ptr<cugl::SpriteSheet>& getThrowSprite() const { return _throwSprite; }

    /**
     * Sets the player dirt throwing sprite.
     *
     * The texture should be formated as a sprite sheet, and the size and
     * layout of the sprite sheet should already be specified in the
     * initializing step. If so, this method will construct a sprite sheet
     * from this texture.
     *
     * @param texture   The texture for the sprite sheet
     */
    void setThrowTexture(const std::shared_ptr<cugl::Texture>& texture);
    
    /**
     * Draws this player to the sprite batch within the given bounds.
     *
     * @param batch     The sprite batch to draw to
     * @param size      The size of the window (for wrap around)
     */
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size);

    /**
     * Draws the peeking player texture on one of the sides, depending on peek angle.
     *
     * @param batch     The sprite batch to draw to
     * @param size      The size of the window (for wrap around)
     * @param peekDirection The direction (-1 for left, 1 for right) that the player is peeking from. Draw on the opposite side.
     * @param sideGap   The size of the side gap for the window grid
     */
    void drawPeeking(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size, int peekDirection, float sideGap);

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
     * @return 0 if moved, -1 if moving off of left edge, 1 if moving off of right edge, 2 otherwise
     */
    int move(cugl::Vec2 dir, cugl::Size size, std::shared_ptr<WindowGrid> windows);

    /** Continues a movement between two grid spots */
    bool move();
    
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

