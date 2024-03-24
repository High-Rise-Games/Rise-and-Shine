#ifndef __BIRD_C__
#define __BIRD_C__

#include "Bird.h"

using namespace cugl;

bool Bird::init(const cugl::Vec2 startP, const cugl::Vec2 endP, const float speed, const float sf) {
    _startPos = startP;
    _endPos = endP;
    _endPos.x = _endPos.x -_radius * 2;
    _speed = speed;
    _scaleFactor = sf;
    birdPosition = _startPos;
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
void Bird::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size, cugl::Vec2 birdPos) {
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
        birdTrans.translate(birdPos);
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
    // Process movement based on flying direction
    cugl::Vec2 target;
    if (_toRight) {
        target = _endPos;
    } else {
        target = _startPos;
    }
    if (_frametimer == 0) {
        int frame = _sprite->getFrame();
        if (frame == _framesize - 1) {
            _frameright = false;
        }
        if (frame == 1) {
            _frameright = true;
        }
        _sprite->setFrame(_frameright ? frame + 1: frame - 1);
        _frametimer = 4;
    } else {
        _frametimer -= 1;
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
