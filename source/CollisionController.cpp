//
//  CollisionController.h
//
//  Author: High Rise Games
//
#include "CollisionController.h"
#include "ProjectileSet.h"

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
std::pair<bool, std::optional<std::tuple<cugl::Vec2, int, int>>> CollisionController::resolveCollision(const std::shared_ptr<Player>& player, std::shared_ptr<ProjectileSet>& pset) {
    bool collision = false;
    std::optional<std::tuple<cugl::Vec2, int, int>> landedDirt;

    auto it = pset->current.begin();
    while (it != pset->current.end()) {
        // Calculate the normal of the (possible) point of collision
        std::shared_ptr<ProjectileSet::Projectile> proj = *it;

        // This loop finds the NEAREST collision if we include wrap for the asteroid/ship
        Vec2 norm = player->getPosition() - proj->position;
        float distance = norm.length();
        float impactDistance;
        if (proj->type == ProjectileSet::Projectile::ProjectileType::POOP) {
            impactDistance = (player->getRadius() + proj->getScale());
        } else {
            impactDistance = (player->getRadius() + proj->getRadius() * proj->getScale());
        }

        // finds the NEAREST collision
        Vec2 pos = proj->position;
        if (proj->_inMiddle) {
            Vec2 topBound = Vec2(proj->position.x, proj->position.y - 100.f);
            Vec2 bottomBound = Vec2(proj->position.x, proj->position.y - 100.f);
            
            // Calculate distances from player to the top and bottom bounds
            float distToTop = (player->getPosition() - topBound).length();
            float distToBottom = (player->getPosition() - bottomBound).length();
            if (distToTop < distance) {
                distance = distToTop;
                norm = pos;
            }
            if (distToBottom < distance) {
                distance = distToBottom;
                norm = pos;
            }
        } else {
            pos = player->getPosition() - pos;
            float dist = pos.length();
            if (dist < distance) {
                distance = dist;
                norm = pos;
            }
        }

        // If this normal is too small, there was a collision
        if (distance < impactDistance) {

            // Damage and/or stun the player
            if (player->getAnimationState() != Player::STUNNED) {
                player->setAnimationState(Player::STUNNED);
            }

            if ((*it)->type == ProjectileSet::Projectile::ProjectileType::DIRT) {
                // if dirt, include the player's current position in the return so that dirt
                // lands on top and around the player
                landedDirt = std::make_tuple(player->getPosition(), (*it)->spawnAmount, 0);
            }

            // delete projectile from set after colliding
            it = pset->current.erase(it);
            collision = true;
        }
        else {
            ++it;
        }
    }
    return std::make_pair(collision, landedDirt);
}

/** Returns true if there is a player bird collision*/
bool CollisionController::resolveBirdCollision(const std::shared_ptr<Player>& player, Bird& bird, cugl::Vec2 birdWorldPos, float radiusMultiplier) {
    bool collision = false;

    Vec2 norm = player->getPosition() - birdWorldPos;
    float distance = norm.length();
    float impactDistance = (player->getRadius() + bird.getRadius() * radiusMultiplier);

    // finds the NEAREST collision
    Vec2 pos = birdWorldPos;
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
