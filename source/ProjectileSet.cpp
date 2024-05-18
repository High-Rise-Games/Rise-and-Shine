#ifndef __PROJECTILE_C__
#define __PROJECTILE_C__

#include "ProjectileSet.h"

using namespace cugl;

/** Use this constructor to generate a specific projectile */
ProjectileSet::Projectile::Projectile(const cugl::Vec2 p, const cugl::Vec2 v, const cugl::Vec2 dest, std::shared_ptr<cugl::Texture> texture, float sf, const ProjectileType t, int s = 1) {
    position = p;
    startPos = p;
    velocity = v;
    velocity.y = 0;
    destination = dest;
    type = t;
    spawnAmount = s;
    _inMiddle = false;

    _projectileTexture = texture;
    _radius = std::min(texture->getSize().height, texture->getSize().width) / 2;
    if (type == ProjectileType::DIRT) {
        _scaleFactor = sf;
    }
    else if(type == ProjectileType::POOP) {
        _projectileSFTexture = cugl::SpriteSheet::alloc(texture, 2,5, 10);
        _projectileSFTexture->setFrame(0);
        _scaleFactor = sf / _projectileSFTexture->getFrameSize().height * 2;
    }
    _maxPooSFFrame = 4;
    _pooSFFrames = 0;
    _progress = 0;
}

/**
* Moves the projectile one animation frame
*
* This method performs no collision detection.
* Collisions are resolved afterwards.
*/
bool ProjectileSet::Projectile::update(Size size) {
    _progress = fmin(1.0f, _progress + 2.5f /startPos.distance(destination));
//    CULog("progress: %f", _progress);
    if (_progress <= 0.05f) {
        // start
        velocity.y = -2.5f  * (_progress / 0.05f);
    } else if (_progress >= 0.95f) {
        // end
        float p = (1.0f - _progress) / 0.05f;
        velocity.y = - 2.5f * p;
    } else {
        // middle
        velocity.y = -2.5f;
    }
//    CULog("progress velocity: %f", velocity.y);
//    CULog("drawing proj progress: %f", 1 - progress);
    Vec2 newPosition = position + velocity;
    // when the new position is over the destination, remove it
    if (type == ProjectileType::DIRT && std::min(position.x, newPosition.x) <= destination.x && destination.x <= std::max(position.x, newPosition.x)) {
        return true;
    }
    if (type == ProjectileType::POOP && (int) _progress == 1) {
//        CULog("reached destination");
        return true;
    }
    // when the projectile move over the edge, remove it
    position = newPosition;
    float r = getRadius() * getScale();
    if (position.x - r > size.width || position.x + r < 0 || position.y - r > size.height * 2 || position.y + r < 0) {
        // delete the projectile once it goes completely off screen
        return true;
    }
    return false;
}


bool ProjectileSet::init(std::shared_ptr<cugl::JsonValue> data) {
    if (data) {
        // Reset all data
        current.clear();
        _pending.clear();
        return true;
    }
    return false;
}

/**
* Sets the texture scale factors to be smaller than size of the window.
*
* This must be called during the initialization of projectile set in GameScene
* otherwise projectiles may "collide" with the player if it is too large at the
* very start of the game
*
* @param windowHeight  the height of each window grid
* @param windowWidth   the width of each window grid
*/
void ProjectileSet::setTextureScales(const float windowHeight, const float windowWidth) {
    _dirtScaleFactor = std::min(windowWidth / _dirtTexture->getWidth(), windowHeight / _dirtTexture->getHeight()) / 1.5;
//    _poopFlightScaleFactor = windowHeight / _poopFlightTexture->getFrameSize().height * 2;
    _poopInFlightScaleFactor =  windowHeight;
}



void ProjectileSet::spawnProjectile(cugl::Vec2 p, cugl::Vec2 v, cugl::Vec2 dest, Projectile::ProjectileType t, int amt) {
    // Determine direction and velocity of the projectile.
    std::shared_ptr<cugl::Texture> texture = _dirtTexture;
    float scale = _dirtScaleFactor;
    std::shared_ptr<Projectile> proj;
    if (t == Projectile::ProjectileType::POOP) {
        proj = std::make_shared<Projectile>(p, v, dest, _poopInFlightTexture, _poopInFlightScaleFactor, t, amt);
    } else {
        proj = std::make_shared<Projectile>(p, v, dest, texture, scale, t, amt);
    }
    _pending.emplace(proj);
}

void ProjectileSet::spawnProjectileClient(cugl::Vec2 p, cugl::Vec2 v, cugl::Vec2 dest, Projectile::ProjectileType t) {
    // Determine direction and velocity of the projectile.
    std::shared_ptr<cugl::Texture> texture = _dirtTexture;
    float scale = _dirtScaleFactor;
    std::shared_ptr<Projectile> proj;
    if (t == Projectile::ProjectileType::POOP) {
        proj = std::make_shared<Projectile>(p, v, dest, _poopInFlightTexture, _poopInFlightScaleFactor, t, 1);
    } else {
        proj = std::make_shared<Projectile>(p, v, dest, texture, scale, t);
    }
    current.emplace(proj);
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
*
* @returns list of destinations to spawn filth objects
*/
std::vector<std::tuple<cugl::Vec2, int, int>> ProjectileSet::update(Size size) {
    // Move projectiles, updating the animation frame
    std::vector<std::tuple<cugl::Vec2, int, int>> dirtDestsAndAmts;
    auto it = current.begin();
    while (it != current.end()) {
        bool erased = (*it)->update(size);
        if (erased) {
            // delete the projectile once it goes completely off screen
            if ((*it)->type == Projectile::ProjectileType::DIRT) {
                dirtDestsAndAmts.push_back(std::make_tuple((*it)->destination, (*it)->spawnAmount, 0));
            }
            else {
                (*it)->type = Projectile::ProjectileType::POOP;
                dirtDestsAndAmts.push_back(std::make_tuple((*it)->destination, 1, 1));
            }
            it = current.erase(it);
        } else {
            it++;
        }
    }

    // Move from pending to current
    for (auto it = _pending.begin(); it != _pending.end(); ++it) {
        current.emplace(*it);
    }
    _pending.clear();
    return dirtDestsAndAmts;
}

/**
* Draws all active projectiles to the sprite batch within the given bounds.
*
* Pending projectiles are not drawn.
*
* @param batch     The sprite batch to draw to
*/
void ProjectileSet::draw(const std::shared_ptr<SpriteBatch>& batch, Size size, float windowWidth, float windowHeight) {
    for (auto it = current.begin(); it != current.end(); ++it) {
        std::shared_ptr<Projectile> proj = (*it);

        std::shared_ptr<Texture> texture = proj->getTexture();
        if (texture == nullptr) {
            continue;
        }

        Vec2 pos = proj->position;

        // may need in future? don't need right now
        // float projWidth = (float)texture->getWidth() * scaleFactor;
        // float projHeight = (float)texture->getHeight() * scaleFactor;

        // float r = proj->getRadius() * proj->getScale();
        Vec2 origin(0, 0);
//        float progress = proj->position.distance(proj->destination) / proj->startPos.distance(proj->destination);
//        CULog("drawing proj distance: %f", proj->position.distance(proj->destination));
//        CULog("drawing proj progress: %f", 1 - progress);
        if (proj->type == Projectile::ProjectileType::POOP) {
//            CULog("drawing poop frame: %d", proj->_pooSFFrames);
            if (proj->startPos.distance(proj->destination) < 100) {
                proj->getSFTexture()->setFrame(std::min((int)(proj->_pooSFFrames / 20) + 4, 9));
                proj->_pooSFFrames += 1;
            } else {
                if (proj->position.distance(proj->destination) > 100) {
                    if (!proj->_inMiddle) {
                        proj->getSFTexture()->setFrame(std::min((int)(proj->_pooSFFrames / 20), 3));
                        proj->_pooSFFrames += 1;
                    }
                    if (proj->getSFTexture()->getFrame() == 3) {
                        proj->_pooSFFrames = 0;
                        proj->_inMiddle = true;
                    }
                } else {
                    if (proj->_inMiddle) {
                        proj->_pooSFFrames = 0;
                        proj->_inMiddle = false;
                    }
                    proj->getSFTexture()->setFrame(std::min((int)(proj->_pooSFFrames / 10) + 4, 9));
                    proj->_pooSFFrames += 1;
                }
            }
//            CULog("drawing frame %d", proj->getSFTexture()->getFrame());
        }
        Affine2 trans = Affine2();
        // trans.translate(texture->getSize() / -2.0);
        if (proj->type == Projectile::ProjectileType::POOP) {
            trans.translate( -(int)(proj->getSFTexture()->getFrameSize().width)/2 , -(int)(proj->getSFTexture()->getFrameSize().height) / 2);
            trans.scale(proj->getScale());
        } else {
            trans.scale(proj->getScale());
        }
        trans.translate(pos);
        
        if (proj->type == Projectile::ProjectileType::POOP) {
            proj->getSFTexture()->draw(batch, trans);
        } else {
            batch->draw(proj->getTexture(), Vec2(), trans);
        }
    }
    
}

#endif
