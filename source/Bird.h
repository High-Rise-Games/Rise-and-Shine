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
    /** start position of the bird */
    cugl::Vec2 _startPos;
    /** end position of the bird */
    cugl::Vec2 _endPos;
    /** true if flying to the right */
    bool _toRight;
    /** drawing scale */
    float _scaleFactor;
    float _radius;

    /** Bird texture on board */
    std::shared_ptr<cugl::Texture> _birdTexture;


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
    
    /** Use this consturctor to generate bird on window board moving between startP and endP back and forth  */
    bool init(const cugl::Vec2 startP, const cugl::Vec2 endP, const float speed, const float sf);

    float getRadius() {return _radius;}
    float getScale() {return _scaleFactor;}
    /** sets bird texture */
    void setTexture(const std::shared_ptr<cugl::Texture>& value) {
        cugl::Size s = value->getSize();
        _radius = std::min(s.width, s.height) / 2;
        _birdTexture = value;
    }
    /** gets bird texture */
    const std::shared_ptr<cugl::Texture>& getTexture() const { return _birdTexture; }
    
    /** draws the bird on game board */
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size);
    
    /** moves the bird on game board in direction based on current position. */
    void move();
    
    /**
     * Returns column number if bird is at the center of a column, else -1
     */
    int atColCenter(const int nHorizontal, const float windowWidth, const float sideGap);
};

#endif
