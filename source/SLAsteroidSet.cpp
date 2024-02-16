//
//  AsteroidSet.cpp
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
#include "SLAsteroidSet.h"

using namespace cugl;

#pragma mark Asteroid
/**
 * Allocates an asteroid by setting its position and velocity.
 *
 * A newly allocated asteroid has type 3, the largest type.
 *
 * @param p     The position
 * @param v     The velocity
 */
AsteroidSet::Asteroid::Asteroid(const cugl::Vec2 p, const cugl::Vec2 v) : Asteroid(p,v,3) {}

/**
 * Allocates an asteroid by setting its position, velocity, and type
 *
 * @param p     The position
 * @param v     The velocity
 * @param type  The type (1, 2, or 3)
 */
AsteroidSet::Asteroid::Asteroid(const cugl::Vec2 p, const cugl::Vec2 v, int type) {
    position = p;
    velocity = v;
    setType(type);
}

/**
 * Returns the type of this asteroid.
 *
 * All asteroids have types 1, 2, or 3.  3 is the largest type of
 * asteroid (scale 1.25), while 1 is the smallest (scale of 0.5).
 *
 * @param type  The type of this asteroid.
 */
void AsteroidSet::Asteroid::setType(int type) {
    CUAssertLog(type > 0 && type <= 3, "type must be 1, 2, or 3");
    _type = type;
    switch (type) {
        case 3:
            _scale = 1.25;
            break;
        case 2:
            _scale = 0.85;
            break;
        case 1:
            _scale = .5;
            break;
        default:
            _scale = 0.0f;
            break;
    }
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
void AsteroidSet::Asteroid::setSprite(const std::shared_ptr<cugl::SpriteSheet>& sprite) {
    _sprite = sprite;
}

/**
 * Moves the asteroid one animation frame
 *
 * This movement code supports "wrap around".  If the asteroid
 * goes off one edge of the screen, then it appears across the
 * edge on the opposite side. However, this method performs no
 * collision detection. Collisions are resolved afterwards.
 */
void AsteroidSet::Asteroid::update(Size size) {
    position += velocity;
    while (position.x > size.width) {
        position.x -= size.width;
    }
    while (position.x < 0) {
        position.x += size.width;
    }
    while (position.y > size.height) {
        position.y -= size.height;
    }
    while (position.y < 0) {
        position.y += size.height;
    }
}

#pragma mark Asteroid Set
/**
 * Creates an asteroid set with the default values.
 *
 * To properly initialize the asteroid set, you should call the init
 * method with the JSON value. We cannot do that in this constructor
 * because the JSON value does not exist at the time the constructor
 * is called (because we do not create this object dynamically).
 */
AsteroidSet::AsteroidSet() :
_mass(0),
_radius(0),
_framecols(0),
_framesize(0) {}
    
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
bool AsteroidSet::init(std::shared_ptr<cugl::JsonValue> data) {
    if (data) {
        // Reset all data
        current.clear();
        _pending.clear();
        
        _mass = data->getFloat("mass",0);
        _damage = data->getInt("damage",0);
        _hitratio = data->getFloat("hit ratio",1);
        _framecols = data->getFloat("sprite cols",0);
        _framesize = data->getFloat("sprite size",0);
        
        // This is an iterator over all of the elements of rocks
        if (data->get("start")) {
            auto rocks = data->get("start")->children();
            for(auto it = rocks.begin(); it != rocks.end(); ++it) {
                std::shared_ptr<JsonValue> entry = (*it);
                Vec2 pos;
                pos.x = entry->get(0)->get(0)->asFloat(0);
                pos.y = entry->get(0)->get(1)->asFloat(0);
                Vec2 vel;
                vel.x = entry->get(1)->get(0)->asFloat(0);
                vel.y = entry->get(1)->get(1)->asFloat(0);
                spawnAsteroid(pos,vel);
            }
        }

        return true;
    }
    return false;
}

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
void AsteroidSet::spawnAsteroid(Vec2 p, Vec2 v, int t) {
    // Determine direction and velocity of the photon.
    std::shared_ptr<Asteroid> rock = std::make_shared<Asteroid>(p,v,t);
    if (_texture) {
        int rows = _framesize/_framecols;
        if (_framesize % _framecols != 0) {
            rows++;
        }
        
        rock->setSprite(SpriteSheet::alloc(_texture,rows,_framecols,_framesize));
        rock->getSprite()->setOrigin(Vec2(_radius,_radius));     
    }
    _pending.emplace(rock);
}
    
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
void AsteroidSet::update(Size size) {
    // Move asteroids, updating the animation frame
    for(auto it = current.begin(); it != current.end(); ++it) {
        (*it)->update(size);
        auto sprite = (*it)->getSprite();
        int frame = sprite->getFrame()+1;
        if (frame >= sprite->getSize()) {
            frame = 0;
        }
        sprite->setFrame(frame);
    }
    
    // Move from pending to current
    for(auto it = _pending.begin(); it != _pending.end(); ++it) {
        current.emplace(*it);
    }
    _pending.clear();
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
void AsteroidSet::setTexture(const std::shared_ptr<cugl::Texture>& value) {
    if (value && _framecols > 0) {
        int rows = _framesize/_framecols;
        if (_framesize % _framecols != 0) {
            rows++;
        }
        Size size = value->getSize();
        size.width /= _framecols;
        size.height /= rows;

        _radius = std::max(size.width,size.height)/2;
        _texture = value;
        
        // Update the sprite sheets of the asteroids as necessary
        for(auto it = current.begin(); it != current.end(); ++it) {
            std::shared_ptr<Asteroid> rock = (*it);
            rock->setSprite(SpriteSheet::alloc(value, rows, _framecols, _framesize));
            rock->getSprite()->setOrigin(Vec2(_radius,_radius));
        }
        for(auto it = _pending.begin(); it != _pending.end(); ++it) {
            std::shared_ptr<Asteroid> rock = (*it);
            rock->setSprite(SpriteSheet::alloc(value, rows, _framecols, _framesize));
            rock->getSprite()->setOrigin(Vec2(_radius,_radius));
        }
    } else {
        _radius = 0;
        _texture = nullptr;
    }
}

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
void AsteroidSet::draw(const std::shared_ptr<SpriteBatch>& batch, Size size) {
    if (_texture) {
        for(auto it = current.begin(); it != current.end(); ++it) {
            float scale = (*it)->getScale();
            Vec2 pos = (*it)->position;
            Vec2 origin(_radius,_radius);

            Affine2 trans;
            trans.scale(scale);
            trans.translate(pos);
            auto sprite = (*it)->getSprite();

            float r = _radius*scale;
            sprite->draw(batch,trans);
        }
    }
}
