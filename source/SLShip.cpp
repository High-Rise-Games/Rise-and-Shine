//
//  SLShip.cpp
//  Ship Lab
//
//  This class tracks all of the state (position, velocity, rotation) of a
//  single ship. In order to obey the separation of the model-view-controller
//  pattern, controller specific code (such as reading the keyboard) is not
//  present in this class.
//
//  With that said, you will notice several elements that look like they are
//  part of the view, namely the texture and sound.  But this is the texture
//  and sound DATA.  The actual view are the sprite batch and audio engine,
//  respectively, that use this data to provide feedback on the screen.
//
//  Author: Walker White
//  Based on original GameX Ship Demo by Rama C. Hoetzlein, 2002
//  Version: 1/20/22
//
#include "SLShip.h"

using namespace cugl;

/**
 * Creates a ship wiht the given position and data.
 *
 * The JsonValue should be a reference of all of the constants
 * that necessary to set the "hidden physical properties".
 *
 * @param pos   The ship position
 * @param data  The data defining the physics constants
 */
Ship::Ship(const cugl::Vec2& pos, std::shared_ptr<cugl::JsonValue> data) {
    _pos = pos;
    _ang  = 0;
    _dang = 0;
    _refire = 0;
    _radius = 0;
    
    // Physics
    _mass = data->getFloat("mass",1.0);
    _firerate = data->getInt("fire rate",0);
    _shadows  = data->getFloat("shadow",0.0);
    _thrust   = data->getFloat("thrust factor",0.0);
    _maxvel   = data->getFloat("max velocity",0.0);
    _banking  = data->getFloat("bank factor",0.0);
    _maxbank  = data->getFloat("max bank",0.0);
    _angdamp  = data->getFloat("angular damp",0.0);

    // Sprite sheet information
    _framecols = data->getInt("sprite cols",0);
    _framesize = data->getInt("sprite size",0);
    _frameflat = data->getInt("sprite frame",0);
    
    _health = data->getInt("health",0);
}

/**
 * Sets the current ship health.
 *
 * When the health of the ship is 0, it is "dead"
 *
 * @param value The current ship health.
 */
void Ship::setHealth(int value) {
    if (value >= 0) {
        // Do not allow health to go negative
        _health = value;
    } else {
        _health = 0;
    }
}

#pragma mark Graphics
/**
 * Sets the texture for this ship.
 *
 * The texture should be formated as a sprite sheet, and the size and
 * layout of the sprite sheet should already be specified in the
 * initializing JSON. If so, this method will construct a sprite sheet
 * from this texture. Otherwise, the texture will be ignored.
 *
 * @param texture   The texture for the sprite sheet
 */
void Ship::setTexture(const std::shared_ptr<cugl::Texture>& texture) {
    if (_framecols > 0) {
        int rows = _framesize/_framecols;
        if (_framesize % _framecols != 0) {
            rows++;
        }
        _sprite = SpriteSheet::alloc(texture, rows, _framecols, _framesize);
        _sprite->setFrame(_frameflat);
        _radius = std::max(_sprite->getFrameSize().width, _sprite->getFrameSize().height)/2;
        _sprite->setOrigin(_sprite->getFrameSize()/2);
    }
}

/**
 * Draws this ship on the screen within the given bounds.
 *
 * This drawing code supports "wrap around". If the ship is partly off of
 * one edge, then it will also be drawn across the edge on the opposite
 * side.
 *
 * @param batch     The sprite batch to draw to
 * @param size      The size of the window (for wrap around)
 */
void Ship::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, Size bounds) {
    // Don't draw if sprite not set
    if (_sprite) {
        // Transform to place the ship
        Affine2 shiptrans;
        shiptrans.rotate(_ang*M_PI/180);
        shiptrans.translate(_pos);
        // Transform to place the shadow, and its color
        Affine2 shadtrans = shiptrans;
        shadtrans.translate(_shadows,-_shadows);
        Color4f shadow(0,0,0,0.5f);
        
        _sprite->draw(batch,shadow,shadtrans);
        _sprite->draw(batch,shiptrans);
        
        // Duplicate images to support wrap
        if (_pos.x+_radius > bounds.width) {
            shiptrans.translate(-bounds.width,0);
            shadtrans.translate(-bounds.width,0);
            _sprite->draw(batch,shadow,shadtrans);
            _sprite->draw(batch,shiptrans);
            shiptrans.translate(bounds.width,0);
            shadtrans.translate(bounds.width,0);
        } else if (_pos.x-_radius < 0) {
            shiptrans.translate(bounds.width,0);
            shadtrans.translate(bounds.width,0);
            _sprite->draw(batch,shadow,shadtrans);
            _sprite->draw(batch,shiptrans);
            shiptrans.translate(-bounds.width,0);
            shadtrans.translate(-bounds.width,0);
        }
        if (_pos.y+_radius > bounds.height) {
            shiptrans.translate(0,-bounds.height);
            shadtrans.translate(0,-bounds.height);
            _sprite->draw(batch,shadow,shadtrans);
            _sprite->draw(batch,shiptrans);
            shiptrans.translate(0,bounds.height);
            shadtrans.translate(0,bounds.height);
        } else if (_pos.y-_radius < 0) {
            shiptrans.translate(0,bounds.height);
            shadtrans.translate(0,bounds.height);
            _sprite->draw(batch,shadow,shadtrans);
            _sprite->draw(batch,shiptrans);
            shiptrans.translate(0,-bounds.height);
            shadtrans.translate(0,-bounds.height);
        }
    }
}

#pragma mark Movement
/**
 * Sets the position of this ship, supporting wrap-around.
 *
 * This is the preferred way to "bump" a ship in a collision.
 *
 * @param value     The position of this ship
 * @param size      The size of the window (for wrap around)
 */
void Ship::setPosition(cugl::Vec2 value, cugl::Vec2 size) {
    _pos = value;
    wrapPosition(size);
}

/**
 * Moves the ship by the specified amount.
 *
 * Forward is the amount to move forward, while turn is the angle to turn the ship
 * (used for the "banking" animation. This method performs no collision detection.
 * Collisions are resolved afterwards.
 *
 * @param forward    Amount to move forward
 * @param turn        Amount to turn the ship
 */
void Ship::move(float forward, float turn, Size size) {
    // Process the ship turning.
    processTurn(turn);

    // Process the ship thrust.
    if (forward != 0.0f) {
        // Thrust key pressed; increase the ship velocity.
        float rads = M_PI*_ang/180.0f+M_PI_2;
        Vec2 dir(cosf(rads),sinf(rads));
        _vel += dir * forward * _thrust;
    }
    if (_vel.length() > 10.f) {
        _vel.normalize();
        _vel = 10.0f*_vel;
    }

    // Move the ship, updating it.
    // Adjust the angle by the change in angle
    setAngle(_ang+_dang);
    
    // INVARIANT: 0 <= ang < 360
    if (_ang > 360)
        _ang -= 360;
    if (_ang < 0)
        _ang += 360;
    
    
    // Move the ship position by the ship velocity
    _pos += _vel;
    wrapPosition(size);

    //Increment the refire readiness counter
    if (_refire <= _firerate) {
        _refire++;
    }
}

/**
 * Update the animation of the ship to process a turn
 *
 * Turning changes the frame of the filmstrip, as we change from a level ship to
 * a hard bank.  This method also updates the field dang cumulatively.
 *
 * @param turn Amount to turn the ship
 */
void Ship::processTurn(float turn) {
    int frame = (_sprite == nullptr ? 0 : _sprite->getFrame());
    int fsize = (_sprite == nullptr ? 0 : _sprite->getSize());
    if (turn != 0.0f) {
        // The turning factor is cumulative.
        // The longer it is held down, the harder we bank.
        _dang -= turn/_banking;
        if (_dang < -_maxbank) {
            _dang = -_maxbank;
        } else if (_dang > _maxbank) {
            _dang = _maxbank;
        }

        // SHIP_IMG_RIGHT represents the hardest bank possible.
        if (turn < 0 && frame < fsize-1) {
            frame++;
        } else if (turn > 0 && frame > 0) {
            frame--;
        }
    } else {
        // If neither key is pressed, slowly flatten out ship.
        if (_dang != 0) {
            _dang *= _angdamp;   // Damping factor.
        }
        if (frame < _frameflat) {
            frame++;
        } else if (frame > _frameflat) {
            frame--;
        }
    }
    
    if (_sprite) {
        _sprite->setFrame(frame);
    }
}

/**
 * Applies "wrap around"
 *
 * If the ship goes off one edge of the screen, then it appears across the edge
 * on the opposite side.
 *
 * @param size      The size of the window (for wrap around)
 */
void Ship::wrapPosition(cugl::Size size) {
    while (_pos.x > size.width) {
        _pos.x -= size.width;
    }
    while (_pos.x < 0) {
        _pos.x += size.width;
    }
    while (_pos.y > size.height) {
        _pos.y -= size.height;
    }
    while (_pos.y < 0) {
        _pos.y += size.height;
    }
}
