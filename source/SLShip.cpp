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
Player::Player(const cugl::Vec2& pos, std::shared_ptr<cugl::JsonValue> data) {
    _pos = pos;
    _coors = Vec2();
    _ang  = 0;
    _dang = 0;
    _refire = 0;
    _radius = 0;
    
    // Physics
    _mass = data->getFloat("mass",1.0);
    _shadows  = data->getFloat("shadow",0.0);
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
void Player::setHealth(int value) {
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
void Player::setTexture(const std::shared_ptr<cugl::Texture>& texture) {
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
* Calculates the coordinates of the player in relation to the window grid
* using the scene position of the player (_pos).
*/
const cugl::Vec2& Player::getCoorsFromPos(const float windowHeight, const float windowWidth, const float sideGap) {
    int x_coor = (int)((_pos.x - sideGap) / windowWidth);
    int y_coor = (int)(_pos.y / windowHeight);
    return Vec2(x_coor, y_coor);
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
void Player::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, Size bounds) {
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
 * Sets the position of this ship
 *
 * This is the preferred way to "bump" a ship in a collision.
 *
 * @param value     The position of this ship
 * @param size      The size of the window (for wrap around)
 */
void Player::setPosition(cugl::Vec2 value, cugl::Vec2 size) {
    _pos = value;

}

/**
 * Moves the ship by the specified amount.
 *
 * Forward is the amount to move forward, while turn is the angle to turn the ship.
 * Makes sure that the ship is within the bounds of the window building grid.
 * Also, can only move along one axis at a time. sideGap argument is the sideGap
 * property of the window building grid in order to make it eaiser to check bounds
 * for player movement.
 *
 * @param forward    Amount to move forward
 * @param turn        Amount to turn the ship
 */
void Player::move(float forward, float turn, Size size, float sideGap) {
    // Process the ship turning.

    // Process forward key press
    if (forward != 0.0f) {
        Vec2 dir(0,10);
        _vel = dir * forward;
    }
    
    if (turn != 0.0f && forward == 0.0f) {
        Vec2 dir(10,0);
        _vel = dir * turn;
    }
    
    if (turn == 0.0f && forward == 0.0f) {
        Vec2 dir(0,0);
        _vel = dir * turn;
    }
    
    
    // Move the ship position by the ship velocity.
    // Velocity always remains unchanged.
    // Also does not add velocity to position in the event that movement would go beyond the window building grid.
    if (!(_pos.x + _vel.x <= sideGap) && !(_pos.x + _vel.x >= 3.3*sideGap) && !(_pos.y + _vel.y >= size.height-20) && !(_pos.y + _vel.y <= 40)) {
        _pos += _vel;
    }

}


