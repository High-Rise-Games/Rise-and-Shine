/*
Class representing a Filth object.
It handles the drawing of a filth object in the game scene as well as internal properties.
Inherits the behaviour of a projectile object.
*/

#ifndef __BIRD_H__
#define __BIRD_H__

#include <cugl/cugl.h>

class Bird {
public:
    /** current position of the bird*/
    cugl::Vec2 birdPosition;
    
private:
    
    /** speed of the bird */
    float _speed;
    /** locations the bird will fly through */
    std::vector<cugl::Vec2> _checkpoints;
    /** next checkpoint */
    int _nextCheckpoint;
    /** true if flying to the right */
    bool _toRight;
    /** drawing scale */
    float _scaleFactor;
    /** The shadow offset in pixels */
    float _shadows;
    
    /** The number of columns in the sprite sheet */
    int _framecols;
    /** The number of frames in the sprite sheet */
    int _framesize;
    /** The sprite sheet frame for being at rest */
    int _frameflat;
    /** order of the sprite playback, to right then to left */
    bool _frameright;
    /** on count down to 0, play one frame of sprite animation*/
    int _frametimer;
    /** Reference to the bird sprite sheet */
    std::shared_ptr<cugl::SpriteSheet> _sprite;
    /** Radius of the bird in pixels (derived from sprite sheet) */
    float _radius;

public:
    /**
     * Creates a bird with the default values.
     *
     * To properly initialize the bird you should call the init
     * method with the JSON value. We cannot do that in this constructor
     * because the JSON value does not exist at the time the constructor
     * is called (because we do not create this object dynamically).
     */
    Bird() {}
    
    /** Use this consturctor to generate bird on window board moving through a list of positions  */
    bool init(const std::vector<cugl::Vec2> positions, const float speed, const float sf, const float windowHeight);

    float getRadius() {return _radius;}

    float getScale() {return _scaleFactor;}

    bool isFacingRight() { return _toRight; }

    void setFacingRight(bool val) { _toRight = val; }
    
    /** Gets bird sprite.
     *
     * The size and layout of the sprite sheet should already be specified
     * in the initializing step.
     * @return the sprite sheet for the ship
     */
    const std::shared_ptr<cugl::SpriteSheet>& getSprite() const {
        return _sprite;
    }

    /**
     * Sets the texture for the bird.
     *
     * The texture should be formated as a sprite sheet, and the size and
     * layout of the sprite sheet should already be specified in the
     * initializing step. If so, this method will construct a sprite sheet
     * from this texture.
     * @param texture   The texture for the sprite sheet
     */
    void setTexture(const std::shared_ptr<cugl::Texture>& texture);
    
    /** draws the bird on game board */
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size, cugl::Vec2 birdWorldPos);
    
    /** moves the bird on game board in direction based on current position. */
    void move();

    /** Advances the bird frame by 1 */
    void advanceBirdFrame();
    
    /** Updates(randomize row) bird position when bird moves to other player's board */
    void resetBirdPath(const int nVertial, const int nHorizontal, const int randomRow);
    
    /** Updates bird position when bird is shooed, flies away and upon reaching destination go to other player's board */
    void resetBirdPathToExit(const int nHorizontal);
    
    /** Returns True when bird position reaches exit */
    bool birdReachesExit();
    
    /**
     * Returns column number if bird is at the center of a column, else -1
     */
    int atColCenter(const int nHorizontal, const float windowWidth, const float sideGap);
};

#endif
