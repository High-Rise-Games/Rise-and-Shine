//  PlayerCharacter.cpp
//
//  Author: High Rise Games
//
#include "PlayerCharacter.h"

using namespace cugl;

/**
 * Creates a player with the given fields.
 *
 * @param id    The player's id
 * @param pos   The player position
 * @param data  The data defining the constants
 * @param windowWidth   The width of the window panes
 * @param windowHeight  The height of the window panes
 */
Player::Player(const int id, const cugl::Vec2& pos, const float windowWidth, const float windowHeight) {
    _id = id;
    _pos = pos;
    _coors = Vec2();
    _speed = 10;
    _shadows = 10;
    _wipeframecols = 7;
    _wipeframesize = 7;
    _idleframecols = 4;
    _idleframesize = 8;
    _throwframecols = 7;
    _throwframesize = 7;
    _shooframecols = 4;
    _shooframesize = 16;


    statusToInt[AnimStatus::IDLE] = 0;
    statusToInt[AnimStatus::WIPING] = 1;
    statusToInt[AnimStatus::SHOOING] = 2;
    statusToInt[AnimStatus::STUNNED] = 3;
    statusToInt[AnimStatus::THROWING] = 4;
    
    // height of a window pane of the game board
    _windowWidth = windowWidth;
    
    // width of a window pane of the game board
    _windowHeight = windowHeight;

    // number of frames the player is unable to move due to taking a hit
    _stunFrames = 0;
    
    // number of frames the player is frozen for because wiping dirt
    _wipeFrames = 4;
    // number of total frames the player will play wipe animation
    _maxwipeFrame = _wipeFrames * _wipeframesize;
    _wipeFrames = _maxwipeFrame;
    
    // number of frames the player is frozen for because shooing bird
    _shooFrames = 2;
    // number of total frames the player will play shoo animation
    _maxshooFrame = _shooFrames * _shooframesize;
    _shooFrames = _maxshooFrame;
    
    // number of frames the player is frozen for because throwing projectile
    _throwFrames = 2;
    // number of total frames the player will play shoo animation
    _maxthrowFrame = _throwFrames * _throwframesize;
    _throwFrames = _maxthrowFrame;
    
    _idleFrames = 4;
    _maxidleFrame = _idleFrames * _idleframesize;
    
    // rotation property of player when player is stunned
    _stunRotate = 0;

    // radius of player for collisions
    _radius = _windowHeight / 2;
}

#pragma mark Graphics
/**
 * Sets the idle texture for player.
 *
 * @param texture   The texture for the sprite sheet
 */
void Player::setIdleTexture(const std::shared_ptr<cugl::Texture>& texture) {
//    if (_idleframecols > 0) {
//        int rows = _idleframesize/_idleframecols;
//        if (_idleframesize % _idleframecols != 0) {
//            rows++;
//        }
//        _idleSprite = SpriteSheet::alloc(texture, rows, _idleframecols, _idleframesize);
//        _idleSprite->setFrame(1);
//    }
    _idleSprite = texture;
}

/**
 * Sets the wiping texture for player.
 *
 * @param texture   The texture for the sprite sheet
 */
void Player::setWipeTexture(const std::shared_ptr<cugl::Texture>& texture) {
//    if (_framecols > 0) {
//        int rows = _framesize/_framecols;
//        if (_framesize % _framecols != 0) {
//            rows++;
//        }
//        _wipeSprite = SpriteSheet::alloc(texture, rows, _framecols, _framesize);
//        _wipeSprite->setFrame(0);
//    }
    _wipeSprite = texture;
}

/**
 * Sets the shooing texture for player.
 *
 * @param texture   The texture for the sprite sheet
 */
void Player::setShooTexture(const std::shared_ptr<cugl::Texture>& texture) {
    if (_shooframecols > 0) {
        int rows = _shooframesize/_shooframecols;
        if (_shooframesize % _shooframecols != 0) {
            rows++;
        }
        _shooSprite = SpriteSheet::alloc(texture, rows, _shooframecols, _shooframesize);
        _shooSprite->setFrame(0);
    }
}

/**
* Sets the player dirt throwing sprite.
*
* @param texture   The texture for the sprite sheet
*/
void Player::setThrowTexture(const std::shared_ptr<cugl::Texture>& texture) {
    if (_throwframecols > 0) {
        int rows = _throwframesize / _throwframecols;
        if (_throwframesize % _throwframecols != 0) {
            rows++;
        }
        _throwSprite = SpriteSheet::alloc(texture, rows, _throwframecols, _throwframesize);
        _throwSprite->setFrame(0);
    }
}

void Player::advanceWipeFrame() {
//    int step = _maxwipeFrame / _wipeframesize;
    _wipeFrames = _wipeFrames + 1;
    if (_wipeFrames == _maxwipeFrame) {
        _wipeFrames = 0;
        setAnimationState(AnimStatus::IDLE);
    }
};

/**
     * Sets the player's movement freeze time to the given time in frames
     * .Used when player shoos bird
     */
void Player::advanceShooFrame() {
    int step = _maxshooFrame / _shooframesize;
    if (_shooFrames < _maxshooFrame) {
        if (_shooFrames % step == 0) {
            _shooSprite->setFrame((int)_shooFrames / step);
            // CULog("drawing frame %d", (int) (_wipeFrames / step) % _framesize);
        }
        _shooFrames += 1;
    }
    else {
        _shooSprite->setFrame(0);
        setAnimationState(AnimStatus::IDLE);
    }
};

/**
 * Sets the player's movement freeze time to the given time in frames
 * .Used when player throws projectile
 */
void Player::advanceThrowFrame() {
    int step = _maxthrowFrame / _throwframesize * 2;
    if (_throwFrames < _maxthrowFrame) {
        if (_throwFrames % step == 0) {
            _throwSprite->setFrame((int)_throwFrames / step);
        }
        _throwFrames += 1;
    }
    else {
        _throwSprite->setFrame(0);
        setAnimationState(AnimStatus::IDLE);
    }
};

/**
 * Advance animation for player idle
 */
void Player::advanceIdleFrame() {
//    int step = _maxidleFrame / _idleframesize;
//    if (_idleFrames % step == 0) {
//        _idleSprite->setFrame((int)(_idleFrames / step));
//    }
    _idleFrames = _idleFrames + 1;
    if (_idleFrames == _maxidleFrame) {
        _idleFrames = 0;
    }
};


/** Decreases the stun frames by one, unless it is already at 0 then does nothing. */
void Player::decreaseStunFrames() {
    if (_stunFrames > 0) {
        _stunFrames -= 1;
    }
    else {
        setAnimationState(IDLE);
    }
}

void Player::advanceAnimation() {
    switch (_animState) {
    case IDLE:
        advanceIdleFrame();
        break;
    case WIPING:
        advanceWipeFrame();
        break;
    case STUNNED:
        decreaseStunFrames();
        break;
    case SHOOING:
        advanceShooFrame();
        break;
    case THROWING:
        advanceThrowFrame();
        break;
    default:
        advanceIdleFrame();
        break;
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
 * @param batch     The sprite batch to draw to
 * @param size      The size of the window (for wrap around)
 */
void Player::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, Size bounds) {
    // Transform to place the ship, start with centered version
    Affine2 player_trans;
    double player_scale;
    std::shared_ptr<cugl::Texture> curSubTexture;
    std::shared_ptr<cugl::Texture> nextSubTexture;
    std::pair<int,float> animProgress;
    switch (_animState) {
        case IDLE:
            animProgress = getTextureIdxWithEase(cugl::EasingFunction::Type::SINE_IN_OUT, _maxidleFrame, _idleFrames, _idleframesize);
//            CULog("frame: %d, progress: %f", animProgress.first, animProgress.second);
            curSubTexture = Player::getSubTexture(_idleSprite, 8, 4, animProgress.first);
            nextSubTexture = Player::getSubTexture(_idleSprite, 8, 4, (animProgress.first + 1) % _idleframesize);
            player_trans.translate( -(int)(curSubTexture->getWidth())/2 , -(int)(curSubTexture->getHeight()) / 2);
            player_scale = _windowHeight / curSubTexture->getHeight();
            player_trans.scale(player_scale);
            break;
        case WIPING:
            animProgress = getTextureIdxWithEase(cugl::EasingFunction::Type::SINE_IN_OUT, _maxwipeFrame, _wipeFrames, _wipeframesize);
            curSubTexture = Player::getSubTexture(_wipeSprite, 7, 7, animProgress.first);
            nextSubTexture = Player::getSubTexture(_wipeSprite, 7, 7, (animProgress.first + 1) % _wipeframesize);
            player_trans.translate( -(int)(curSubTexture->getWidth())/2 , -(int)(curSubTexture->getWidth()) / 2);
            player_scale = _windowHeight / curSubTexture->getHeight();
            player_trans.scale(player_scale);
            break;
        case STUNNED:
            animProgress = getTextureIdxWithEase(cugl::EasingFunction::Type::SINE_IN_OUT, _maxidleFrame, _idleFrames, _idleframesize);
//            CULog("frame: %d, progress: %f", animProgress.first, animProgress.second);
            curSubTexture = Player::getSubTexture(_idleSprite, 8, 4, animProgress.first);
            nextSubTexture = Player::getSubTexture(_idleSprite, 8, 4, (animProgress.first + 1) % _idleframesize);
            player_trans.translate( -(int)(curSubTexture->getWidth())/2 , -(int)(curSubTexture->getHeight()) / 2);
            player_scale = _windowHeight / curSubTexture->getHeight();
            player_trans.scale(player_scale);
            _stunRotate += 0.1;
            player_trans.rotate(_stunRotate * M_PI);
            break;
        case SHOOING:
            player_trans.translate( -(int)(_shooSprite->getFrameSize().width)/2 , -(int)(_shooSprite->getFrameSize().height) / 2);
            player_scale = _windowHeight / _shooSprite->getFrameSize().height;
            player_trans.scale(player_scale);
            break;
        default: // default IDLE - throwing status should only be called while peeking, in which case drawPeeking handles
            animProgress = getTextureIdxWithEase(cugl::EasingFunction::Type::SINE_IN_OUT, _maxidleFrame, _idleFrames, _idleframesize);
//            CULog("frame: %d, progress: %f", animProgress.first, animProgress.second);
            curSubTexture = Player::getSubTexture(_idleSprite, 8, 4, animProgress.first);
            nextSubTexture = Player::getSubTexture(_idleSprite, 8, 4, (animProgress.first + 1) % _idleframesize);
            player_trans.translate( -(int)(curSubTexture->getWidth())/2 , -(int)(curSubTexture->getHeight()) / 2);
            player_scale = _windowHeight / curSubTexture->getHeight();
            player_trans.scale(player_scale);
            break;
    }
    // Don't draw if texture not set
    if (_animState != STUNNED) {
        _stunRotate = 0;
        player_trans.rotate(0);
    }

    player_trans.translate(_pos);
    Affine2 shadtrans = player_trans;
    shadtrans.translate(_shadows,-_shadows);
    Affine2 bgtrans = player_trans;
    Color4f shadow(0,0,0,0.5f);
    Color4f bg(1.f,1.f,1.f,1.f);
    batch->setTexture(curSubTexture);
    Color4f transition = bg;
//    batch->setSrcBlendFunc(GL_ONE);
//    batch->setDstBlendFunc(GL_ONE_MINUS_SRC_ALPHA);
    switch (_animState) {
        case IDLE:
//            alphaValue = ((float) (_idleFrames % animProgress.second)) / (animProgress.second - 1);
//            CULog("cur frame: %d", cur_frame);
//            CULog("next frame: %d", next_frame);
            batch->draw(curSubTexture, shadow, Vec2(), shadtrans);
//            batch->draw(curSubTexture, bg, Vec2(), bgtrans);
//            _idleSprite->draw(batch, player_trans);
            transition.a = 1.f; // Alpha for current sub-texture
//            transition.scale(1.f - alphaValue);
            batch->draw(curSubTexture, transition, Vec2(), player_trans);
//            CULog("cur frame transparency: %f", transition.a);
//            transition = bg;
            transition.a = animProgress.second; // Alpha for next sub-texture
//            transition.scale(alphaValue);
            batch->draw(nextSubTexture, transition, Vec2(), player_trans);
//            CULog("next frame transparency: %f", transition.a);
            break;
        case WIPING:
            batch->draw(curSubTexture, shadow, Vec2(), shadtrans);
            transition.a = 1.f; // Alpha for current sub-texture
            batch->draw(curSubTexture, transition, Vec2(), player_trans);
            transition.a = animProgress.second; // Alpha for next sub-texture
            batch->draw(nextSubTexture, transition, Vec2(), player_trans);
            break;
        case SHOOING:
            _shooSprite->draw(batch, shadow, shadtrans);
            _shooSprite->draw(batch, player_trans);
            break;
        default: // STUNNED
            batch->draw(curSubTexture, shadow, Vec2(), shadtrans);
            transition.a = 1.f; // Alpha for current sub-texture
            batch->draw(curSubTexture, transition, Vec2(), player_trans);
            transition.a = animProgress.second; // Alpha for next sub-texture
            batch->draw(nextSubTexture, transition, Vec2(), player_trans);
            break;
    }
//    batch->setSrcBlendFunc(GL_SRC_ALPHA);
//    batch->setDstBlendFunc(GL_ONE_MINUS_SRC_ALPHA);
}

/**
* Draws the peeking player texture on one of the sides, depending on peek angle.
*
* @param batch     The sprite batch to draw to
* @param size      The size of the window (for wrap around)
* @param peekDirection The direction (-1 for left, 1 for right) that the player is peeking from. Draw on the opposite side.
* @param sideGap   The size of the side gap for the window grid
*/
void Player::drawPeeking(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size, int peekDirection, float sideGap) {
    Affine2 player_trans;
    double player_scale;
    
    if (_throwSprite) {
        if (_animState == THROWING) {
            advanceThrowFrame();
        }
        player_trans.translate(0, -(int)(_throwSprite->getFrameSize().height) / 2);
        player_scale = _windowHeight / _throwSprite->getFrameSize().height;

        // flip sprite and translate position depending on peeking side
        if (peekDirection == -1) {
            player_trans.translate(-(int)(_throwSprite->getFrameSize().width)*0.65, 0);
            player_trans.scale(-player_scale, player_scale);
            player_trans.translate(size.width - sideGap, _pos.y);
        }
        else if (peekDirection == 1) {
            player_trans.translate(-(int)(_throwSprite->getFrameSize().width)*0.65, 0);
            player_trans.scale(player_scale);
            player_trans.translate(sideGap, _pos.y);
        }
        _throwSprite->draw(batch, player_trans);
    }
}

#pragma mark Movement

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
 * @return 0 if moved, -1 if moving off of left edge, 1 if moving off of right edge, 2 otherwise
 */
int Player::move(Vec2 dir, Size size, std::shared_ptr<WindowGrid> windows) {
    // Process moving direction
    if (!_targetDist.isZero()) {
        if (abs(_targetDist.x - _vel.x) > abs(_vel.x) || abs(_targetDist.y - _vel.y) > abs(_vel.y)) {
            _targetDist = _targetDist - _vel;
            _pos += _vel;
        } else {
            _pos += _targetDist;
            _targetDist.setZero();
        }
        return 0;
    } else {
        _vel.setZero();
        if (dir.x != 0.0f) {
            _targetDist = dir * _windowHeight;
            Vec2 originIndices = windows->getGridIndices(_pos, size);
            Vec2 targetPosition = Vec2(_pos) + _targetDist;
            Vec2 targetIndices = windows->getGridIndices(targetPosition, size);

            if (!windows->getCanMoveBetween(originIndices.x, originIndices.y, targetIndices.x, targetIndices.y)) {
                _targetDist.setZero();
                // check if player is trying to switch scenes
                if (targetIndices.x < 0) {
                    return -1;
                }
                else if (targetIndices.x >= windows->getNHorizontal()) {
                    return 1;
                }
                return 2;
            }
            _vel = dir * _speed;
        } else if (dir.y != 0.0f) {
            _targetDist = dir * _windowWidth;
            Vec2 originIndices = windows->getGridIndices(_pos, size);
            Vec2 targetPosition = Vec2(_pos) + _targetDist;
            Vec2 targetIndices = windows->getGridIndices(targetPosition, size);
            if (!windows->getCanMoveBetween(originIndices.x, originIndices.y, targetIndices.x, targetIndices.y)) {
                _targetDist.setZero();
                return 2;
            }
            _vel = dir * _speed;
        }
    }
    return 2;
}

/** Continues a movement between two grid spots */
bool Player::move() {
    if (!_targetDist.isZero()) {
        if (abs(_targetDist.x - _vel.x) > abs(_vel.x) || abs(_targetDist.y - _vel.y) > abs(_vel.y)) {
            _targetDist = _targetDist - _vel;
            _pos += _vel;
        }
        else {
            _pos += _targetDist;
            _targetDist.setZero();
        }
        return true;
    }
    return false;
}

/**
 * Returns edge if player is at the edge of building
 *
 * @return -1 if the player is at left edge, 0 not at edge, and 1 at right edge
 */
int Player::getEdge(float sideGap, Size size) {
    if (_pos.x + _targetDist.x <= sideGap) {
        return -1;
    } else if (_pos.x + _targetDist.x >= size.getIWidth() - sideGap) {
        return 1;
    }
    return 0;
};


std::shared_ptr<cugl::Texture> Player::getSubTexture(std::shared_ptr<cugl::Texture> texture, int size, int cols, int frame) {
    int rows = size/cols;
    if (size % cols != 0) {
        rows++;
    }
    
    float col = frame % cols;
    float row = frame / cols;

    GLfloat minS = col /cols;
    GLfloat maxS = (col + 1)/cols;
    GLfloat minT = row / rows;
    GLfloat maxT = (row + 1) / rows;

//    CULog("%f, %f, %f, %f", minS, maxS, minT, maxT);
    std::shared_ptr<Texture> subTexture = texture->getSubTexture(minS, maxS, minT, maxT);
    return subTexture;
}

std::pair <int,float> Player::getTextureIdxWithEase(cugl::EasingFunction::Type ef, int maxFrameNum, int curFrame, int spriteSize) {
    float t = (float) curFrame / (maxFrameNum - 1);
    float eased_t;
    switch (ef) {
        case cugl::EasingFunction::Type::SINE_IN_OUT:
            eased_t = cugl::EasingFunction::sineInOut(t);
            break;
        case cugl::EasingFunction::Type::CIRC_IN_OUT :
            eased_t = cugl::EasingFunction::circInOut(t);
            break;
        default:
            eased_t = t;
            break;
    }
    int curSpriteIdx = int(eased_t * (spriteSize - 1));
    int nextSpriteIdx = curSpriteIdx + 1 < spriteSize ? curSpriteIdx + 1 : curSpriteIdx;

    float t_cur = (float) curSpriteIdx / (spriteSize - 1);
    float t_next = (float) nextSpriteIdx / (spriteSize - 1);

    if (t_next - t_cur == 0) {
            return std::make_pair(curSpriteIdx, 0.0f);
        }

    float progress = (eased_t - t_cur) / (t_next - t_cur);
    
    return std::make_pair(curSpriteIdx, progress);
}

