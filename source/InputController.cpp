//
//  GLInputController.cpp
//  Ship Lab
//
//  This class buffers in input from the devices and converts it into its
//  semantic meaning. If your game had an option that allows the player to
//  remap the control keys, you would store this information in this class.
//  That way, the main game scene does not have to keep track of the current
//  key mapping.
//
//  Author: Walker M. White
//  Based on original GameX Ship Demo by Rama C. Hoetzlein, 2002
//  Version: 1/20/22
//
#include <cugl/cugl.h>
#include "InputController.h"

using namespace cugl;

#pragma mark Input Control
/**
 * Creates a new input controller with the default settings
 *
 * This is a very simple class.  It only has the default settings and never
 * needs to attach any custom listeners at initialization.  Therefore, we
 * do not need an init method.  This constructor is sufficient.
 */
InputController::InputController() :
_forward(0),
_turning(0),
_didFire(false),
_touchDown(false),
_touchKey(0) {
}

/**
 * Initializes the control to support keyboard or touch.
 *
 * This method attaches all of the listeners. It tests which
 * platform we are on (mobile or desktop) to pick the right
 * listeners.
 *
 * This method will fail (return false) if the listeners cannot
 * be registered or if there is a second attempt to initialize
 * this controller
 *
 * @return true if the initialization was successful
 */
bool InputController::init() {
#ifdef CU_TOUCH_SCREEN
    Touchscreen* touch = Input::get<Touchscreen>();
    if (touch) {
        _touchKey = touch->acquireKey();
        touch->addBeginListener(_touchKey, [=](const TouchEvent& event, bool focus) {
            this->touchDownCB(event, focus);
        });
        touch->addEndListener(_touchKey, [=](const TouchEvent& event, bool focus) {
            this->touchUpCB(event, focus);
        });
        _active = true;
    }
#else
    return true;
#endif
    return _active;
}

/**
 * Disposes this input controller, deactivating all listeners.
 *
 * As the listeners are deactived, the user will not be able to
 * monitor input until the controller is reinitialized with the
 * {@link #init} method.
 */
void InputController::dispose() {
#ifdef CU_TOUCH_SCREEN
    if (_active) {
        Touchscreen* touch = Input::get<Touchscreen>();
        touch->removeBeginListener(_touchKey);
        touch->removeEndListener(_touchKey);
        touch->removeMotionListener(_touchKey);
        _active = false;
    }
#endif
}

/**
 * Updates the input controller for the latest frame.
 *
 * It might seem weird to have this method given that everything
 * is processed with call back functions.  But we need some way
 * to synchronize the input with the animation frame.  Otherwise,
 * how can we know what was the touch location *last frame*?
 * Maybe there has been no callback function executed since the
 * last frame. This method guarantees that everything is properly
 * synchronized.
 */
void InputController::update() {
#ifdef CU_TOUCH_SCREEN
    _forward = _turning = 0;
    _didFire = false;
    _didReset = false;
    
    if (_touchReleased) {
        float absX = abs(_moveDis.x);
        float absY = abs(_moveDis.y);
        if (absX > absY && absX > 50) {
            if (_moveDis.x > 0) {
                _turning = 1;
            } else {
                _turning = -1;
            }
        } else if (absY >= absX && absY > 50) {
            if (_moveDis.y > 0) {
                _forward = -1;
            } else {
                _forward = 1;
            }
        }
    }
    _touchReleased = false;
#else
    // This makes it easier to change the keys later
    KeyCode up    = KeyCode::ARROW_UP;
    KeyCode down  = KeyCode::ARROW_DOWN;
    KeyCode left  = KeyCode::ARROW_LEFT;
    KeyCode right = KeyCode::ARROW_RIGHT;
    KeyCode shoot = KeyCode::SPACE;
    KeyCode reset = KeyCode::R;

    // Convert keyboard state into game commands
    _forward = _turning = 0;
    _didFire = false;
    _didReset = false;
    
    // Movement forward/backward
    Keyboard* keys = Input::get<Keyboard>();
    if (keys->keyPressed(up) && !keys->keyDown(down)) {
        _forward = 1;
    } else if (keys->keyPressed(down) && !keys->keyDown(up)) {
        _forward = -1;
    }
    
    // Movement left/right
    if (!keys->keyDown(right) && (keys->keyPressed(left))) {
        _turning = -1;
    } else if (keys->keyPressed(right) && !keys->keyDown(left)) {
        _turning = 1;
    }

    // Shooting
    if (keys->keyDown(shoot)) {
        _didFire = true;
    }
    
    // Reset the game
    if (keys->keyDown(reset)) {
        _didReset = true;
    }
#endif
}

#pragma mark -
#pragma mark Touch Callbacks

void InputController::touchDownCB(const cugl::TouchEvent& event, bool focus) {
    if (!_touchDown) {
        _touchDown = true;
        _touchID = event.touch;
        _startPos = event.position;
    }
}

void InputController::touchUpCB(const cugl::TouchEvent& event, bool focus) {
    if (_touchDown && _touchID == event.touch) {
        cugl::Vec2 endPos = event.position;
        _moveDis = endPos - _startPos;
        _touchDown = false;
        _touchReleased = true;
    }
}
