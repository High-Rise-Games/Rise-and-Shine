#ifndef __BIRD_C__
#define __BIRD_C__

#include "Bird.h"

using namespace cugl;

bool Bird::init(const cugl::Vec2 startP, const cugl::Vec2 endP, const float speed, const float sf) {
    _startPos = startP;
    _endPos = endP;
    _endPos.x = _endPos.x -_radius * sf * 2;
    _speed = speed;
    _scaleFactor = sf;
    birdPosition = _startPos;
    _toRight = true;
    return true;
}

// draws a static filth on the screen on window pane at location windowPos
void Bird::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size, cugl::Vec2 birdPos) {
    // calculate origin of bird
    float r = _radius * _scaleFactor;
    Vec2 origin(r, r);
    
    Affine2 birdTrans;
    birdTrans.scale(_scaleFactor);
    birdTrans.translate(birdPos);
    
    batch->draw(_birdTexture, origin, birdTrans);
}

/** Moves the bird on game board in direction based on current position. */
void Bird::move() {
    // Process movement based on flying direction
    cugl::Vec2 target;
    if (_toRight) {
        target = _endPos;
    } else {
        target = _startPos;
    }
    float targetDist = target.distance(birdPosition);
    if (targetDist - _speed > 0) {
        birdPosition = birdPosition.add(_speed * (_toRight ? 1 : -1), 0);
    } else {
        birdPosition = (_toRight ? _endPos : _startPos);
        _toRight = !_toRight;
    }
}

/** Returns column number if bird is at the center of a column, else -1*/
int Bird::atColCenter(const int nHorizontal, const float windowWidth, const float sideGap) {
    for (int i=0; i < nHorizontal; i++) {
        float windowXPos = sideGap - 20 + ((i == 0) ? 0 : (i * windowWidth)) + (float) windowWidth /2;
        if ((_toRight) ? (birdPosition.x < windowXPos && birdPosition.x + _speed > windowXPos) :
            (birdPosition.x > windowXPos && birdPosition.x - _speed < windowXPos)) {
//            CULog("at column center %i", i);
            return i;
        }
    }
    return -1;
}

#endif
