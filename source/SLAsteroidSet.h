//
//  AsteroidSet.h
//
//  This class implements a collection of asteroids. As with the ship, we are
//  going to dynamically allocate asteroids and use smart pointers to reference
//  them. That way they are garbage collected when we delete an asteroid.
//
//  But we also need something to store all these smart pointers.  We cannot
//  have a variable for each asteriod -- we don't know how many there will be.
//  Technically, we could just use an unordered_set for this.  This is a collection
//  type in C++. But as we saw in the intro class, sometimes it makes sense to
//  factor out information that is common to all asteroids into the container
//  class.  And that is what we are doing here.
//
//  Author: Walker White
//  Based on original GameX Ship Demo by Rama C. Hoetzlein, 2002
//  Version: 2/21/21
//
#ifndef __SL_ASTERIOD_SET_H__
#define __SL_ASTERIOD_SET_H__
#include <cugl/cugl.h>
#include <unordered_set>

/**
 * Model class representing a collection of asteroids.
 *
 * All asteroids share the same texture and physical information. Therefore, we put
 * all common information in the AsteroidSet. Individual asteroid information (scale,
 * velocity, and position) goes in the asteroid itself.
 *
 * One of the things you will notice about this class is that AsteroidSet is composed
 * of two unordered sets: current and pending. That is because of how we plan to
 * destroy asteroids. In C++ it is 100% to delete items from a set that you are
 * looping over (as long as you do it correctly).  But it is not safe to ADD elements
 * to a set when you loop over it.  But when we destroy an asteroid, we want to replace
 * it with three smaller asteroids.
 *
 * To make this action safe, when you spawn a new asteroid, it is not added to the
 * current set immediately. Instead, it is added to the pending set. Asteroids are
 * moved from the pending set to the current set when you call {@link #update}. So
 * you can delete an asteroid from current and spawn smaller asteroids in pending
 * without worrying about an infinite loop.
 */
class AsteroidSet {
#pragma mark Asteroid
public:
    /**
     * An inner class that represents a single Asteroid.
     *
     * Asteroids come in three different sizes, represented by their types.
     */
    class Asteroid {
    // It is okay for the user to access these directly
    public:
        /** Photon position */
        cugl::Vec2 position;
        /** Photon velocity */
        cugl::Vec2 velocity;
        // But these need to be protected for invariant reasons
    private:
        /** The type of the asteroid: 1, 2, or 3 */
        int _type;
        /** The drawing scale of the asteroid (to vary the size) */
        float _scale;
        /** The sprite sheet for animating the asteroid */
        std::shared_ptr<cugl::SpriteSheet> _sprite;
        
    public:
        /**
         * Allocates an asteroid by setting its position and velocity.
         *
         * A newly allocated asteroid has type 3, the largest type.
         *
         * @param p     The position
         * @param v     The velocity
         */
        Asteroid(const cugl::Vec2 p, const cugl::Vec2 v);

        /**
         * Allocates an asteroid by setting its position, velocity, and type
         *
         * @param p     The position
         * @param v     The velocity
         * @param type  The type (1, 2, or 3)
         */
        Asteroid(const cugl::Vec2 p, const cugl::Vec2 v, int type);
        
        /**
         * Returns the scale of this asteroid.
         *
         * Multiply this scale by the standard photon radius
         * ({@link AsteroidSet#getRadius}) to get the "true" radius
         * of an asteroid.
         *
         * @return the scale of this asteroid.
         */
        float getScale() const { return _scale; }

        /**
         * Returns the type of this asteroid.
         *
         * All asteroids have types 1, 2, or 3.  3 is the largest type of
         * asteroid (scale 1.25), while 1 is the smallest (scale of 0.5).
         *
         * @return the type of this asteroid.
         */
        int getType() const { return _type; }
        
        /**
         * Returns the type of this asteroid.
         *
         * All asteroids have types 1, 2, or 3.  3 is the largest type of
         * asteroid (scale 1.25), while 1 is the smallest (scale of 0.5).
         *
         * @param type  The type of this asteroid.
         */
        void setType(int type);

        
        /**
         * Returns the sprite sheet for the asteroid
         *
         * Note the return type. The const is applied to the pointer,
         * not the sprite sheet. So this means it is not okay to
         * change the contents of the pointer, but it is okay to
         * change the contents (the frame) of the sprite sheet
         *
         * @return the sprite sheet for the ship
         */
        const std::shared_ptr<cugl::SpriteSheet>& getSprite() const {
            return _sprite;
        }

        /**
         * Sets the sprite sheet for this asteroid.
         *
         * Note the parameter type. The const is applied to the pointer,
         * not the sprite sheet. So this means it is not okay to
         * change the contents of the pointer, but it is okay to
         * change the contents (the frame) of the sprite sheet
         *
         * @param texture   The sprite sheet for this asteroid.
         */
        void setSprite(const std::shared_ptr<cugl::SpriteSheet>& sprite);
        
        /**
         * Moves the asteroid one animation frame
         *
         * This movement code supports "wrap around".  If the asteroid
         * goes off one edge of the screen, then it appears across the
         * edge on the opposite side. However, this method performs no
         * collision detection. Collisions are resolved afterwards.
         */
        void update(cugl::Size size);
        
    };

private:
    /** The texture for the asteroid sprite sheet. */
    std::shared_ptr<cugl::Texture> _texture;

    /** The collection of all pending asteroids (for next frame). */
    std::unordered_set<std::shared_ptr<Asteroid>> _pending;
    
    /** The mass of a general asteroid */
    float _mass;
    /** The amount of damage caused by an asteroid */
    float _damage;
    /** The radius of a general asteroid */
    float _radius;
    /** The number of columns in the sprite sheet */
    int _framecols;
    /** The number of frames in the sprite sheet */
    int _framesize;
    /** Percentage of sprite radius hit box (needed as sprite are not tight) */
    float _hitratio;
    
#pragma mark The Set
public:
    /** The collection of all ACTIVE photons. Allow the user direct access */
    std::unordered_set<std::shared_ptr<Asteroid>> current;
    
    /**
     * Creates an asteroid set with the default values.
     *
     * To properly initialize the asteroid set, you should call the init
     * method with the JSON value. We cannot do that in this constructor
     * because the JSON value does not exist at the time the constructor
     * is called (because we do not create this object dynamically).
     */
    AsteroidSet();

    /**
     * Initializes asteroid data with the given JSON
     *
     * This JSON contains all shared information like the mass and the
     * sprite sheet dimensions. It also contains a list of asteroids to
     * spawn initially.
     *
     * If this method is called a second time, it will reset all
     * asteroid data.
     *
     * @param data  The data defining the asteroid settings
     *
     * @return true if initialization was successful
     */
    bool init(std::shared_ptr<cugl::JsonValue> data);
    
    /**
     * Returns true if the asteroid set is empty.
     *
     * This means that both the pending and the current set are empty.
     *
     * @return true if the asteroid set is empty.
     */
    bool isEmpty() const { return current.empty() && _pending.empty(); }

    /**
     * Returns the amount of damage caused by an asteroid.
     *
     * Because we want to keep health as an int, we DO NOT scale this
     * value.  Those little ones are deceptively dangerous.
     *
     * @return
     */
    int getDamage() const { return _damage; }

    /**
     * Returns the default mass of a photon
     *
     * This value should be multiplied by the photon scale to get the
     * true mass.
     */
    float getMass() const { return _mass; }

    /**
     * Returns the default radius of a photon
     *
     * This value should be multiplied by the photon scale to get the
     * true radius. This value is computed from the texture.  If there
     * is no active photon texture, this value is 0.
     */
    float getRadius() const { return _radius*_hitratio; }
    
    /**
     * Returns the image for a single asteroid; reused by all asteroid.
     *
     * This value should be loaded by the GameScene and set there. However,
     * we have to be prepared for this to be null at all times.  This
     * texture will be used to generate the sprite sheet for each
     * asteroid. Asteroids must have different sprite sheets because,
     * while they share a texture, they do not share the same animation
     * frame.
     *
     * The sprite sheet information (size, number of columns) should have
     * been set in the initial JSON. If not, this texture will be ignored.
     *
     * @return the image for a single asteroid; reused by all asteroids.
     */
    const std::shared_ptr<cugl::Texture>& getTexture() const {
        return _texture;
    }

    /**
     * Sets the image for a single photon; reused by all photons.
     *
     * This value should be loaded by the GameScene and set there. However,
     * we have to be prepared for this to be null at all times.  This
     * texture will be used to generate the sprite sheet for each
     * asteroid. Asteroids must have different sprite sheets because,
     * while they share a texture, they do not share the same animation
     * frame.
     *
     * The sprite sheet information (size, number of columns) should have
     * been set in the initial JSON. If not, this texture will be ignored.
     *
     * @param value the image for a single asteroid; reused by all asteroids.
     */
    void setTexture(const std::shared_ptr<cugl::Texture>& value);

    /**
     * Adds an asteroid to the active queue.
     *
     * All asteroids are added to a pending set; they do not appear in the current
     * set until {@link #update} is called. This makes it safe to add new asteroids
     * while still looping over the current asteroids.
     *
     * @param p     The asteroid position.
     * @param v     The asteroid velocity.
     * @param type  The asteroid type.
     */
    void spawnAsteroid(cugl::Vec2 p, cugl::Vec2 v, int type=3);

    /**
     * Moves all the asteroids in the active set.
     *
     * In addition, if any asteroids are in the pending set, they will appear
     * (unmoved) in the current set. The pending set will be cleared.
     *
     * This movement code supports "wrap around".  If the photon goes off one
     * edge of the screen, then it appears across the edge on the opposite
     * side. However, this method performs no collision detection. Collisions
     * are resolved afterwards.
     */
    void update(cugl::Size size);

    /**
	 * Draws all active asteroids to the sprite batch within the given bounds.
	 *
	 * This drawing code supports "wrap around". If a photon is partly off of
	 * one edge, then it will also be drawn across the edge on the opposite
	 * side.
	 *
	 * Pending asteroids are not drawn.
     *
     * @param batch     The sprite batch to draw to
     * @param size      The size of the window (for wrap around)
     */
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size);
};

#endif /* __SL_ASTEROID_SET_H__ */
