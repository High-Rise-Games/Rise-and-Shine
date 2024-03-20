//
//  CollisionController.h
//
//  Author: High Rise Games
//
#include "CollisionController.h"

/** Impulse for giving collisions a slight bounce. */
#define COLLISION_COEFF     0.1f

using namespace cugl;

/**
 * Returns true if there is a player-projectile collision
 *
 * In addition to checking for the collision, this method also resolves it.
 * That means it applies the effect to the player for EACH projectile encountered.
 * It does not, however, play the sound. That happens in the main controller
 *
 * @param player  The player
 * @param pset    The projectile set
 *
 * @return true if there is a ship-asteroid collision
 */
bool CollisionController::resolveCollision(const std::shared_ptr<Player>& player, ProjectileSet& pset) {
    bool collision = false;
    auto it = pset.current.begin();
    while (it != pset.current.end()) {
        // Calculate the normal of the (possible) point of collision
        std::shared_ptr<ProjectileSet::Projectile> proj = *it;

        // This loop finds the NEAREST collision if we include wrap for the asteroid/ship
        Vec2 norm = player->getPosition() - proj->position;
        float distance = norm.length();
        float impactDistance = (player->getRadius() + proj->getRadius() * proj->getScale());

        // finds the NEAREST collision 
        Vec2 pos = proj->position;
        pos = player->getPosition() - pos;
        float dist = pos.length();
        if (dist < distance) {
            distance = dist;
            norm = pos;
        }

        // If this normal is too small, there was a collision
        if (distance < impactDistance) {

            // Damage and/or stun the player as the last step
            player->setHealth(player->getHealth() - proj->getDamage());
            player->setStunFrames(proj->getStunTime());

            // delete projectile from set after colliding
            it = pset.current.erase(it);

            collision = true;
        }
        else {
            ++it;
        }
    }
    return collision;
}

/** Returns true if there is a player bird collision*/
bool CollisionController::resolveBirdCollision(const std::shared_ptr<Player>& player, Bird& bird, float radiusMultiplier) {
    bool collision = false;

    Vec2 norm = player->getPosition() - bird.birdPosition;
    float distance = norm.length();
    float impactDistance = (player->getRadius() + bird.getRadius() * bird.getScale() * radiusMultiplier);

    // finds the NEAREST collision
    Vec2 pos = bird.birdPosition;
    pos = player->getPosition() - pos;
    float dist = pos.length();
    if (dist < distance) {
        distance = dist;
        norm = pos;
    }

    // If this normal is too small, there was a collision
    if (distance < impactDistance) {
        collision = true;
    }
    return collision;
}
