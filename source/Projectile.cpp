#ifndef __PROJECTILE_C__
#define __PROJECTILE_C__

#include "Projectile.h"

using namespace cugl;

/** Use this consturctor to generate flying projectile */
Projectile::Projectile(const cugl::Vec2 p, const cugl::Vec2 v) {
    position = p;
    velocity = v;
}

/** moves the filth object in each frame */
void Projectile::update(cugl::Size size) {
 // TODO
}

/** draws a projectile */
void Projectile::drawProjectile(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size, cugl::Affine2 projectileTrans) {
    // TODO
}

#endif
