/*
Class representing a Filth object.
It handles the drawing of a filth object in the game scene as well as internal properties.
Inherits the behaviour of a projectile object.
*/

#ifndef __STATIC_FILTH_H__
#define __STATIC_FILTH_H__

#include <cugl/cugl.h>

class StaticFilth {
public:
    /** discrete location of the filth on the window board */
    cugl::Vec2 boardPosition;
    /** time to clean the filth */
    float timeToClean;
    
    
private:
    /** drawing scale */
    float _scaleFactor;

    /** Filth texture on board */
    std::shared_ptr<cugl::Texture> _filthStaticTexture;
    /** Filth texture in flight */
    std::shared_ptr<cugl::Texture> _filthFlightTexture;
    /** Whether the texture is birdPoo*/
    float _birdPoo;


public:

    /** Use this consturctor to generate static filth on window board */
    StaticFilth(const cugl::Vec2 p);

    /** sets window pane texture */
    void setStaticTexture(const std::shared_ptr<cugl::Texture>& value) { _filthStaticTexture = value; }
    
    /** if true, get inflight texture, else get static */
    const std::shared_ptr<cugl::Texture>& getTexture(bool inflight) const { return _filthStaticTexture; }
    
    /** sets window pane texture */
    void setBrdPoo(bool value) { if (value) {
        _birdPoo = 1;
    } else {
        _birdPoo = 0;
    };  }
    
    /** Gets whether the texture is bird poo
     1 = true, 0 = false. */
    float getBirdPoo() {
        return _birdPoo;
    }
    
    /** draws a filth on static window plane */
    void drawStatic(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size, cugl::Affine2 filthTrans);
    
    
};

#endif
