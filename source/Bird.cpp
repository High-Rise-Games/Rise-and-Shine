#ifndef __BIRD_C__
#define __BIRD_C__

#include "Bird.h"

using namespace cugl;

bool Bird::init(const std::vector<cugl::Vec2> positions, const float speed, const float sf) {
    _checkpoints = positions;
    _speed = speed;
    _scaleFactor = sf;
    birdPosition = _checkpoints[0];
    _nextCheckpoint = 1;
    _toRight = true;
    _framecols = 5;
    _framesize = 5;
    _frameflat = 4;
    _frametimer = 4;
    _frameright = true;
    return true;
}

// sets up the sprite image
void Bird::setTexture(const std::shared_ptr<cugl::Texture>& texture) {
    if (_framecols > 0) {
        int rows = _framesize/_framecols;
        if (_framesize % _framecols != 0) {
            rows++;
        }
        _sprite = SpriteSheet::alloc(texture, rows, _framecols, _framesize);
        _sprite->setFrame(_frameflat);
        _radius = std::min(_sprite->getFrameSize().width, _sprite->getFrameSize().height)/2 * _scaleFactor;
        // shift bird origin to left and down to simulate poop effect from stomach
        _sprite->setOrigin(Vec2(_sprite->getFrameSize().width/2-1000, _sprite->getFrameSize().height/2-400));
    }
}

// draws a static filth on the screen on window pane at location windowPos
void Bird::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size, cugl::Vec2 birdWorldPos) {
    // Don't draw if sprite not set
    if (_sprite) {
        Affine2 birdTrans;
        if (!_toRight) {
            _sprite->setOrigin(Vec2(_sprite->getFrameSize().width/2, _sprite->getFrameSize().height/2-400));
            birdTrans.scale(Vec2(-_scaleFactor, _scaleFactor));
        } else {
            _sprite->setOrigin(Vec2(_sprite->getFrameSize().width/2-1000, _sprite->getFrameSize().height/2-400));
            birdTrans.scale(_scaleFactor);
        }
        birdTrans.translate(birdWorldPos);
        // Transform to place the shadow, and its color
//        Affine2 shadtrans = shiptrans;
//        shadtrans.translate(_shadows,-_shadows);
//        Color4f shadow(0,0,0,0.5f);
//        
//        _sprite->draw(batch,shadow,shadtrans);
        _sprite->draw(batch,birdTrans);
        
         
    }
}

/** Moves the bird on game board in direction based on current position. */
void Bird::move() {
    // Process movement based on current place and next destination
    cugl::Vec2 target = _checkpoints[_nextCheckpoint];
    float dist = target.distance(birdPosition);
    cugl::Vec2 newTarget = target - birdPosition;
    if (dist > _speed) {
//        CULog("(%f, %f)", birdPosition.x, birdPosition.y);
        birdPosition = birdPosition.add(_speed * newTarget.normalize());
    } else {
        birdPosition = _checkpoints[_nextCheckpoint];
        if (_nextCheckpoint == _checkpoints.size() - 1) {
            _nextCheckpoint = 0;
        } else {
            _nextCheckpoint += 1;
        }
    }
    if (birdPosition.x <= target.x) {
        _toRight = true;
    } else {
        _toRight = false;
    }
}

void Bird::advanceBirdFrame() {
    if (_frametimer == 0) {
        int frame = _sprite->getFrame();
        if (frame == _framesize - 1) {
            _frameright = false;
        }
        if (frame == 1) {
            _frameright = true;
        }
        _sprite->setFrame(_frameright ? frame + 1 : frame - 1);
        _frametimer = 4;
    }
    else {
        _frametimer -= 1;
    }
}

/** Updates(randomize row) bird position when bird moves to other player's board */
void Bird::resetBirdPath(const int nVertial, const int nHorizontal, const int randomRow) {
    cugl::Vec2 birdTopLeftPos;
    cugl::Vec2 birdTopRightPos;
    cugl::Vec2 birdBotLeftPos;
    cugl::Vec2 birdBotRightPos;
    if (randomRow < 3) {
        birdTopLeftPos = cugl::Vec2(0.5, randomRow + 0.5);
        birdTopRightPos = cugl::Vec2(nHorizontal - 0.5, randomRow + 0.5);
        birdBotLeftPos = cugl::Vec2(0.5, randomRow + 3.5);
        birdBotRightPos = cugl::Vec2(nHorizontal - 0.5, randomRow + 3.5);
    } else {
        birdTopLeftPos = cugl::Vec2(0.5, randomRow - 0.5);
        birdTopRightPos = cugl::Vec2(nHorizontal - 0.5, randomRow - 0.5);
        birdBotLeftPos = cugl::Vec2(0.5, randomRow - 3.5);
        birdBotRightPos = cugl::Vec2(nHorizontal - 0.5, randomRow - 3.5);
    }
    std::vector<cugl::Vec2> positions = {birdTopLeftPos, birdTopRightPos, birdBotLeftPos, birdBotRightPos};
    _checkpoints = positions;
    birdPosition = _checkpoints[0];
    _nextCheckpoint = 1;
}

/** Returns column number if bird is at the center of a column, else -1*/
int Bird::atColCenter(const int nHorizontal, const float windowWidth, const float sideGap) {

    for (int i=0; i < nHorizontal; i++) {
        float xPos = i + 0.4;
        if ((_toRight) ? (birdPosition.x < xPos && birdPosition.x + _speed > xPos) :
            (birdPosition.x > xPos && birdPosition.x - _speed < xPos)) {
//            CULog("at column center %i", i);
            return i;
        }
    }
    return -1;
}

#endif
