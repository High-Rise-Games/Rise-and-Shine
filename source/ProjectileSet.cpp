#ifndef __PROJECTILE_C__
#define __PROJECTILE_C__

#include "ProjectileSet.h"

using namespace cugl;

/** Use this constructor to generate flying projectile */
ProjectileSet::Projectile::Projectile(const cugl::Vec2 p, const cugl::Vec2 v, std::shared_ptr<cugl::Texture> texture) {
    position = p;
    velocity = v;
    type = ProjectileType::POOP;
    _scaleFactor = 1;
    _damage = 1;
    _stunTime = 0;
    _projectileTexture = texture;
    _radius = std::max(texture->getSize().height, texture->getSize().width) / 2;
}

/** Use this constructor to generate a specific projectile */
ProjectileSet::Projectile::Projectile(const cugl::Vec2 p, const cugl::Vec2 v, std::shared_ptr<cugl::Texture> texture, const ProjectileType t) {
    position = p;
    velocity = v;
    type = t;
    _projectileTexture = texture;
    _radius = std::max(texture->getSize().height, texture->getSize().width) / 2;
    if (type == ProjectileType::DIRT) {
        _scaleFactor = 1;
        _damage = 0;
        _stunTime = 1;
    }
    else if(type == ProjectileType::POOP) {
        _scaleFactor = 1;
        _damage = 1;
        _stunTime = 0;
    }
}

/** sets projectile texture */
void ProjectileSet::Projectile::setProjectileTexture(const std::shared_ptr<cugl::Texture>& value) {
    Size size = value->getSize();
    _radius = std::max(size.width, size.height) / 2;
    _projectileTexture = value;
}

/**
* Moves the projectile one animation frame
*
* This method performs no collision detection.
* Collisions are resolved afterwards.
*/
void ProjectileSet::Projectile::update(Size size) {
    position += velocity;
}


bool ProjectileSet::init(std::shared_ptr<cugl::JsonValue> data) {
    if (data) {
        // Reset all data
        current.clear();
        _pending.clear();

        // This is an iterator over all of the elements of projectiles
        if (data->get("start")) {
            auto projs = data->get("start")->children();
            for (auto it = projs.begin(); it != projs.end(); ++it) {
                std::shared_ptr<JsonValue> entry = (*it);
                Vec2 pos;
                pos.x = entry->get(0)->get(0)->asFloat(0);
                pos.y = entry->get(0)->get(1)->asFloat(0);
                Vec2 vel;
                vel.x = entry->get(1)->get(0)->asFloat(0);
                vel.y = entry->get(1)->get(1)->asFloat(0);
                std::string type = entry->get(2)->asString();
                if (type == "DIRT") {
                    spawnProjectile(pos, vel, Projectile::ProjectileType::DIRT);
                }
                else if (type == "POOP") {
                    spawnProjectile(pos, vel, Projectile::ProjectileType::POOP);
                }
                
            }
        }
        return true;
    }
    return false;
}


void ProjectileSet::spawnProjectile(cugl::Vec2 p, cugl::Vec2 v, Projectile::ProjectileType t) {
    // Determine direction and velocity of the projectile.
    std::shared_ptr<cugl::Texture> texture = _dirtTexture;
    if (t == Projectile::ProjectileType::POOP) {
        texture = _poopTexture;
    }
    std::shared_ptr<Projectile> proj = std::make_shared<Projectile>(p, v, texture, t);
    _pending.emplace(proj);
}

/**
* Moves all the projectiles in the active set.
*
* In addition, if any projectiles are in the pending set, they will appear
* (unmoved) in the current set. The pending set will be cleared.
*
* This movement code does not support "wrap around".
* This method performs no collision detection. Collisions
* are resolved afterwards.
*/
void ProjectileSet::update(Size size) {
    // Move projectiles, updating the animation frame
    for (auto it = current.begin(); it != current.end(); ++it) {
        (*it)->update(size);
    }

    // Move from pending to current
    for (auto it = _pending.begin(); it != _pending.end(); ++it) {
        current.emplace(*it);
    }
    _pending.clear();
}

/**
* Draws all active projectiles to the sprite batch within the given bounds.
*
* Pending projectiles are not drawn.
*
* @param batch     The sprite batch to draw to
*/
void ProjectileSet::draw(const std::shared_ptr<SpriteBatch>& batch, Size size, float windowWidth, float windowHeight) {
    auto it = current.begin();
    while (it != current.end()) {
        
        std::shared_ptr<Texture> texture = (*it)->getTexture();
        if (texture == nullptr) {
            continue;
        }

        Vec2 pos = (*it)->position;

        float scaleFactor = std::min(windowWidth / texture->getWidth(), windowHeight / texture->getHeight());
        (*it)->setScale(scaleFactor);
        float dirtWidth = (float)texture->getWidth() * scaleFactor;
        float dirtHeight = (float)texture->getHeight() * scaleFactor;

        (*it)->setRadius((*it)->getRadius() * scaleFactor);
        Vec2 origin((*it)->getRadius(), (*it)->getRadius());


        Affine2 trans;
        trans.scale(scaleFactor);
        trans.translate(pos);
        auto sprite = (*it)->getTexture();

        float r = (*it)->getRadius() * scaleFactor;
        batch->draw(texture, origin, trans);

        if (pos.x - r > size.width || pos.x + r < 0 || pos.y - r > size.height || pos.y + r < 0) {
            // delete the projectile once it goes completely off screen
            it = current.erase(it);
        }
        else {
            it++;
        }
    }
    
}

#endif
