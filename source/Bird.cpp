#ifndef __BIRD_C__
#define __BIRD_C__

#include "Bird.h"

using namespace cugl;

bool Bird::init(const std::vector<cugl::Vec2> positions, const float speed, const float sf, const float windowHeight) {
    _checkpoints = positions;
    _speed = speed;
    _scaleFactor = sf;
    birdPosition = _checkpoints[0];
    _radius = windowHeight / 2;
    _nextCheckpoint = 1;
    _toRight = true;
    _shadows = 10;
    _framecols = 5;
    _framesize = 10;
    _frameflat = 4;
    _frametimer = 3;
    _frameright = true;
    _cooldown = 20;
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
        // shift bird origin to left and down to simulate poop effect from stomach
        _sprite->setOrigin(Vec2(_sprite->getFrameSize().width/2, _sprite->getFrameSize().height/2));
    }
}

// draws a static filth on the screen on window pane at location windowPos
void Bird::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size, cugl::Vec2 birdWorldPos) {
    // Don't draw if sprite not set
    if (_sprite) {
        Affine2 birdTrans;
        double bird_scale = _radius * 2 / _sprite->getFrameSize().height;
        _sprite->setOrigin(Vec2(_sprite->getFrameSize().width/2, _sprite->getFrameSize().height/2));
        if (!_toRight) {
            birdTrans.scale(Vec2(-bird_scale, bird_scale));
        } else {
            birdTrans.scale(bird_scale);
        }
        birdTrans.translate(birdWorldPos);
        // Transform to place the shadow, and its color
        Affine2 shadtrans = birdTrans;
        shadtrans.translate(_shadows,-_shadows);
        Color4f shadow(0,0,0,0.5f);
        
        _sprite->draw(batch,shadow,shadtrans);
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
        _frametimer = 3;
    }
    else {
        _frametimer -= 1;
    }
}

/** Updates(randomize row) bird position when bird moves to other player's board */
void Bird::resetBirdPath(const int nVertial, const int nHorizontal, const int randomRow) {
    cugl::Vec2 birdPos1;
    cugl::Vec2 birdPos2;
    cugl::Vec2 birdPos3;
    cugl::Vec2 birdPos4;
    if (randomRow < 3 ) {
        // move in z shape upward
        birdPos1 = cugl::Vec2(0.4, randomRow + 0.5);
        birdPos2 = cugl::Vec2(nHorizontal - 0.6, randomRow + 0.5);
        birdPos3 = cugl::Vec2(0.4, randomRow + 3.5);
        birdPos4 = cugl::Vec2(nHorizontal - 0.6, randomRow + 3.5);
    } else {
        // move in z shape downward
        birdPos1 = cugl::Vec2(0.4, randomRow + 0.5);
        birdPos2 = cugl::Vec2(nHorizontal - 0.6, randomRow + 0.5);
        birdPos3 = cugl::Vec2(0.4, randomRow - 2.5);
        birdPos4 = cugl::Vec2(nHorizontal - 0.6, randomRow - 2.5);
    }
    std::vector<cugl::Vec2> positions = {birdPos1, birdPos2, birdPos3, birdPos4};
    _checkpoints = positions;
    birdPosition = _checkpoints[0];
    _nextCheckpoint = 1;
}

/** Updates  bird position when bird is shooed, flies away and upon reaching destination go to other player's board */
void Bird::resetBirdPathToExit(const int nHorizontal) {
    if (_checkpoints.size() != 1) {
        cugl::Vec2 birdExit;
        if (_toRight) {
            birdExit = cugl::Vec2(- nHorizontal/2, birdPosition.y + 1);
        } else {
            birdExit = cugl::Vec2(nHorizontal + nHorizontal/2, birdPosition.y + 1);
        }
        _speed *= 4;
        std::vector<cugl::Vec2> positions = {birdExit};
        _checkpoints = positions;
        _nextCheckpoint = 0;
    }
}

/** Returns True when bird position reaches exit */
bool Bird::birdReachesExit() {
    if (_checkpoints.size() == 1 && birdPosition == _checkpoints[0]) {
        _speed /= 4;
        return true;
    }
    return false;
}

/** Returns column number if bird is at the center of a column, else -1*/
int Bird::atColCenter(const int nHorizontal, const float windowWidth, const float sideGap) {

    for (int i=0; i < nHorizontal; i++) {
        float xPos = i + 0.4;
        if ((_toRight) ? (birdPosition.x < xPos && birdPosition.x + _speed > xPos) :
            (birdPosition.x > xPos && birdPosition.x - _speed < xPos)) {
//             CULog("at column center %i", i);
            return i;
        }
    }
    return -1;
}

#endif
