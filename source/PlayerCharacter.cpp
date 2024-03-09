//  PlayerCharacter.cpp
//
//  Author: High Rise Games
//
#include "PlayerCharacter.h"

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
Player::Player(const cugl::Vec2& pos, std::shared_ptr<cugl::JsonValue> data, const float windowWidth, const float windowHeight) {
    _pos = pos;
    _coors = Vec2();
    _ang  = 0;
    _dang = 0;
    _refire = 0;
    _radius = 0;
    
    // height of a window pane of the game board
    _windowWidth = windowWidth;
    
    // width of a window pane of the game board
    _windowHeight = windowHeight;
    
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

/** Decreases the stun frames by one, unless it is already at 0 then does nothing. */
void Player::decreaseStunFrames() {
    if (_stunFrames > 0) {
        _stunFrames -= 1;
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
    // int cast should be inside the bracket, otherwise causes numerical inprecision results in +1 x coord when at right edge
    int x_coor = ((int)(_pos.x - sideGap) / windowWidth);
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
 * Dir is the amount to move forward and direction to move for the player.
 * Makes sure that the player is within the bounds of the window building grid.
 * Also, can only move along one axis at a time. sideGap argument is the sideGap
 * property of the window building grid in order to make it eaiser to check bounds
 * for player movement.
 *
 * @param dir       Amount to move forward
 * @param size      Size of the game scene
 * @return True if moved
 */
bool Player::move(Vec2 dir, Size size, float sideGap) {

    // Process moving direction
    _vel = dir * 5;
    
    
    // Move the ship position by the ship velocity.
    // Velocity always remains unchanged.
    // Also does not add velocity to position in the event that movement would go beyond the window building grid.
    int atEdge = getEdge(sideGap, size);
    if (!atEdge && _pos.y + _vel.y >= 0 && _pos.y + _vel.y <= size.height) {
        _pos += _vel;
        return true;
    }
    
    //clamp to the left or right if at edge
    if (atEdge > 0) {
        _pos.x = size.getIWidth() - sideGap;
    } else {
        _pos.x = sideGap;
    }
    _pos.y += _vel.y;
    return false;

}

/**
 * Returns edge if player is at the edge of building
 *
 * @return -1 if the player is at left edge, 0 not at edge, and 1 at right edge
 */
int Player::getEdge(float sideGap, Size size) {
    if (_pos.x + _vel.x <= sideGap) {
        return -1;
    } else if (_pos.x + _vel.x >= size.getIWidth() - sideGap) {
        return 1;
    }
    return 0;
};


