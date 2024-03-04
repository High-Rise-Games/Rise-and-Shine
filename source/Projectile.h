/*
Class representing a Filth object.
It handles the drawing of a filth object in the game scene as well as internal properties.
Inherits the behaviour of a projectile object.
*/

#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include <cugl/cugl.h>

class Projectile {
public:
    /** projectile location */
    cugl::Vec2 position;
    /** projectile velocity */
    cugl::Vec2 velocity;
    /** discrete location of the projectile on the window board */
    cugl::Vec2 boardPosition;
    
private:
    /** drawing scale */
    float _scaleFactor;

    /** projectile texture in flight */
    std::shared_ptr<cugl::Texture> _projectileTexture;


public:
    
    /** Use this consturctor to generate flying projectile */
    Projectile(const cugl::Vec2 p, const cugl::Vec2 v);

    /** sets filth texture */
    void setProjectileTexture(const std::shared_ptr<cugl::Texture>& value) { _projectileTexture = value; }
    
    /** if true, get inflight texture, else get static */
    const std::shared_ptr<cugl::Texture>& getTexture(bool inflight) const { return _projectileTexture; }
    
    /** moves the filth object in each frame */
    void update(cugl::Size size);
    
    /** draws a projectile */
    void drawProjectile(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size, cugl::Affine2 projectileTrans);
    
};

#endif
