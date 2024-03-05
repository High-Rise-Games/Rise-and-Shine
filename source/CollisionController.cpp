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

        // This loop finds the NEAREST collision if we include wrap for the asteroid/ship
        for (int ii = -1; ii <= 1; ii++) {
            for (int jj = -1; jj <= 1; jj++) {
                Vec2 pos = proj->position;
                pos.x += (ii)*_size.width;
                pos.y += (jj)*_size.height;
                pos = player->getPosition() - pos;
                float dist = pos.length();
                if (dist < distance) {
                    distance = dist;
                    norm = pos;
                }
            }
        }

        // If this normal is too small, there was a collision
        if (distance < impactDistance) {
            // "Roll back" time so that the ships are barely touching (e.g. point of impact).
            norm.normalize();
            Vec2 temp = norm * ((impactDistance - distance) / 2);
            player->setPosition(player->getPosition() + temp);
            proj->position = proj->position - temp;

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