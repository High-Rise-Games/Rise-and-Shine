/*
Class representing a Filth object.
It handles the drawing of a filth object in the game scene as well as internal properties.
Inherits the behaviour of a projectile object.
*/

#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include <cugl/cugl.h>

/**
 * Model class representing a collection of projectiles.
 *
 * All projectiles share the same physical information. Therefore, we put
 * all common information in the ProjectileSet. Individual projectile information (scale, texture,
 * velocity, and position) goes in the projectile itself.
 *
 * ProjectileSet is composed of two unordered sets: current and pending. Since it is not safe
 * to ADD elements to a set when you loop over it, when we spawn a new projectile, it is not added to the
 * current set immediately. Instead, it is added to the pending set. Projectiles are
 * moved from the pending set to the current set when we call {@link #update}. So
 * you can delete an projectile from current and spawn more projectiles in pending
 * without worrying about an infinite loop.
 */
class ProjectileSet {
public:
    class Projectile {
    public:
        enum class ProjectileType { DIRT, POOP };

        /** projectile location */
        cugl::Vec2 position;
        /** projectile velocity */
        cugl::Vec2 velocity;
        /** projectile destination, ONLY active for dirt. In board position. */
        cugl::Vec2 destination;
        /** initial position, for drawing bird poo */
        cugl::Vec2 startPos;
        /** type of projectile */
        ProjectileType type;
        /** amount of dirt to land */
        int spawnAmount;
        
        /** total number of frames for poo start and end */
        int _maxPooSFFrame;
        // current poo SF frame
        int _pooSFFrames;
        bool _inMiddle;
        float _progress;

    
    private:
        /** The drawing scale factor for this projectile */
        float _scaleFactor;
        /** The amount of damage caused by a projectile */
        float _damage;
        /** The radius of the projectile */
        float _radius;
        
        /** projectile texture in flight */
        std::shared_ptr<cugl::Texture> _projectileTexture;
        /** projectile texture on start or finish, only used by poop */
        std::shared_ptr<cugl::SpriteSheet> _projectileSFTexture;


    public:

        /** Use this constructor to generate a specialized projectile
         * @param t     type of projectile
        */
        Projectile(const cugl::Vec2 p, const cugl::Vec2 v, const cugl::Vec2 dest, std::shared_ptr<cugl::Texture> texture, float sf, const ProjectileType t, int s);

        /** sets projectile scale for drawing */
        void setScale(float s) { _scaleFactor = s; }

        /** gets projectile scale for drawing */
        float getScale() { return _scaleFactor; }
    
        /** get projectile texture */
        const std::shared_ptr<cugl::Texture>& getTexture() const { return _projectileTexture; }
        const std::shared_ptr<cugl::SpriteSheet>& getSFTexture() const { return _projectileSFTexture; }

        /**
         * Returns the radius of the projectile.
         *
         * @return radius
         */
        float getRadius() const { return _radius; }

        /** Sets the radius of the projectile. */
        void setRadius(float r) { _radius = r; }
    
        /**
         * Moves the projectile one animation frame
         *
         * This method performs no collision detection.
         * Collisions are resolved afterwards.
         *
         * @return whether the projectile should be removed
         */
        bool update(cugl::Size size);
    
    };

public:
    /** The collection of all active projectiles. */
    std::unordered_set<std::shared_ptr<Projectile>> current;

private:
    /** The collection of all pending projectiles (for next frame). */
    std::unordered_set<std::shared_ptr<Projectile>> _pending;
    /** The texture for dirt projectiles */
    std::shared_ptr<cugl::Texture> _dirtTexture;
    /** The texture for poop  middle part  */
    std::shared_ptr<cugl::Texture> _poopInFlightTexture;
    /** The tecture for poop inflight */
    std::shared_ptr<cugl::SpriteSheet> _poopFlightTexture;
    /** The scale factor for the dirt texture based on window grid size */
    float _dirtScaleFactor;
    /** The scale factor for the poop texture based on window grid size */
    float _poopInFlightScaleFactor;

public:
    /**
     * Creates an projectile set with the default values.
     *
     * To properly initialize the projectile set, you should call the init
     * method with the JSON value. We cannot do that in this constructor
     * because the JSON value does not exist at the time the constructor
     * is called (because we do not create this object dynamically).
     */
    ProjectileSet() {}

    /**
     * Initializes projectile data with the given JSON
     *
     * This JSON contains all shared information like the velocity and
     * a list of asteroids to spawn initially.
     *
     * If this method is called a second time, it will reset all
     * projectile data.
     *
     * @param data  The data defining the projectile settings
     *
     * @return true if initialization was successful
     */
    bool init(std::shared_ptr<cugl::JsonValue> data);

    /**
     * Returns true if the projectile set is empty.
     *
     * This means that both the pending and the current set are empty.
     *
     * @return true if the projectile set is empty.
     */
    bool isEmpty() const { return current.empty() && _pending.empty(); }

    /** Clears the projectile current/active set. */
    void clearCurrentSet() {
        current.clear();
    }

    /**
     * Returns the image for a single dirt projectile; reused by all dirt projectiles.
     *
     * This value should be loaded by the GameScene and set there. However,
     * we have to be prepared for this to be null at all times.
     *
     * @return the image for a single dirt projectile; reused by all dirt projectiles.
     */
    const std::shared_ptr<cugl::Texture>& getDirtTexture() const {
        return _dirtTexture;
    }

    /**
     * Sets the image for a single dirt projectile; reused by all dirt projectiles.
     *
     * This value should be loaded by the GameScene and set there. However,
     * we have to be prepared for this to be null at all times.
     *
     * @param value the image for a dirt projectile; reused by all dirt projectiles.
     */
    void setDirtTexture(const std::shared_ptr<cugl::Texture>& value) {
        _dirtTexture = value;
    }
    
    const std::shared_ptr<cugl::Texture>& getPoopInFlightTexture() const {
        return _poopInFlightTexture;
    }

    /**
     * Sets the image for a single poop projectile; reused by all poop projectiles.
     *
     * This value should be loaded by the GameScene and set there. However,
     * we have to be prepared for this to be null at all times.
     *
     * @param value the image for a poop projectile; reused by all poop projectiles.
     */
    
    void setPoopInFlightTexture(const std::shared_ptr<cugl::Texture>& value) {
        _poopInFlightTexture = value;
    }
    
    /**
     * Sets the texture scale factors.
     *
     * This must be called during the initialization of projectile set in GameScene
     * otherwise projectiles may "collide" with the player if it is too large at the
     * very start of the game
     *
     * @param windowHeight  the height of each window grid
     * @param windowWidth   the width of each window grid
     */
    void setTextureScales(const float windowHeight, const float windowWidth);

    /**
     * Adds a projectile to the active queue.
     *
     * All projectiles are added to a pending set; they do not appear in the current
     * set until {@link #update} is called. This makes it safe to add new projectiles
     * while still looping over the current projectiles.
     *
     * @param p     The projectile position.
     * @param v     The projectile velocity.
     * @param dest  The projectile's destination.
     * @param t     The projectile type.
     * @param amt   The amount of filth to spawn when landing
     */
    void spawnProjectile(cugl::Vec2 p, cugl::Vec2 v, cugl::Vec2 dest, Projectile::ProjectileType t, int amt = 1);

    /**
     * ONLY CALLED ON CLIENT SIDE. Adds a projectile directly to the current set.
     *
     * We do not need to add it to a pending set because the only thing the client needs to do is draw
     * projectiles in the current set.
     *
     * If we only added projectiles to the pending set, then projectiles will never be drawn as the
     * pending set will repeatedly be cleared then repopulated before any move to the current set.
     *
     * @param p     The projectile position.
     * @param v     The projectile velocity.
     * @param dest  The projectile's destination.
     * @param t     The projectile type.
     */
    void spawnProjectileClient(cugl::Vec2 p, cugl::Vec2 v, cugl::Vec2 dest, Projectile::ProjectileType t);

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
    std::vector<std::tuple<cugl::Vec2, int, int>> update(cugl::Size size);

    /**
     * Draws all active projectiles to the sprite batch within the given bounds.
     *
     * Pending projectiles are not drawn.
     *
     * @param batch     The sprite batch to draw to
     */
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size, float windowWidth, float windowHeight);
};

#endif
