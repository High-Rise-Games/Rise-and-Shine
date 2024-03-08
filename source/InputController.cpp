//
//  InputController.cpp
//
//  Author: High Rise Games
//
#include <cugl/cugl.h>
#include "InputController.h"

using namespace cugl;

#pragma mark Input Control

/** How close we need to be for a multi touch */
#define NEAR_TOUCH      100
/** The key for the event handlers */
#define LISTENER_KEY      1

/** This defines the joystick "deadzone" (how far we must move) */
#define JSTICK_DEADZONE  15
/** This defines the joystick radial size (for reseting the anchor) */
#define JSTICK_RADIUS    25
/** How far to display the virtual joystick above the finger */
#define JSTICK_OFFSET    80

// The screen is divided into four zones: Left, Bottom, Right and Main/
// These are all shown in the diagram below.
//
//   |---------------|
//   |   |       |   |
//   | L |   M   | R |
//   |   |       |   |
//   |---------------|
//
// The meaning of any touch depends on the zone it begins in.

/** The portion of the screen used for the left zone */
#define LEFT_ZONE       0.35f
/** The portion of the screen used for the right zone */
#define RIGHT_ZONE      0.35f

/**
 * Creates a new input controller with the default settings
 *
 * This is a very simple class.  It only has the default settings and never
 * needs to attach any custom listeners at initialization.  Therefore, we
 * do not need an init method.  This constructor is sufficient.
 */
InputController::InputController() :
_active(false),
_forward(0),
_turning(0),
_didFire(false),
_joystick(false) {
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
bool InputController::init(const Rect bounds) {
    _sbounds = bounds;
    _tbounds = Application::get()->getDisplayBounds();
    
    createZones();
    clearTouchInstance(_ltouch);
    clearTouchInstance(_rtouch);
    clearTouchInstance(_mtouch);
#ifdef CU_TOUCH_SCREEN
    Touchscreen* touch = Input::get<Touchscreen>();
    if (touch) {
        touch->addBeginListener(LISTENER_KEY, [=](const TouchEvent& event, bool focus) {
            this->touchDownCB(event, focus);
        });
        touch->addEndListener(LISTENER_KEY, [=](const TouchEvent& event, bool focus) {
            this->touchUpCB(event, focus);
        });
        touch->addMotionListener(LISTENER_KEY,[=](const TouchEvent& event, const Vec2& previous, bool focus) {
            this->touchesMovedCB(event, previous, focus);
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
        touch->removeBeginListener(LISTENER_KEY);
        touch->removeEndListener(LISTENER_KEY);
        touch->removeMotionListener(LISTENER_KEY);
        _active = false;
    }
#else
    _active = false
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
    if (keys->keyDown(up) && !keys->keyDown(down)) {
        _forward = 1;
    } else if (keys->keyDown(down) && !keys->keyDown(up)) {
        _forward = -1;
    }
    
    // Movement left/right
    if (!keys->keyDown(right) && (keys->keyDown(left))) {
        _turning = -1;
    } else if (keys->keyDown(right) && !keys->keyDown(left)) {
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
#pragma mark Touch Controls

/**
 * Defines the zone boundaries, so we can quickly categorize touches.
 */
void InputController::createZones() {
    _lzone = _tbounds;
    _lzone.size.width *= LEFT_ZONE;
    _rzone = _tbounds;
    _rzone.size.width *= RIGHT_ZONE;
    _rzone.origin.x = _tbounds.origin.x+_tbounds.size.width-_rzone.size.width;
}

/**
 * Populates the initial values of the input TouchInstance
 */
void InputController::clearTouchInstance(TouchInstance& touchInstance) {
    touchInstance.touchids.clear();
    touchInstance.position = Vec2::ZERO;
}

/**
 * Returns the correct zone for the given position.
 *
 * See the comments above for a description of how zones work.
 *
 * @param  pos  a position in screen coordinates
 *
 * @return the correct zone for the given position.
 */
InputController::Zone InputController::getZone(const Vec2 pos) const {
    if (_lzone.contains(pos)) {
        return Zone::LEFT;
    } else if (_rzone.contains(pos)) {
        return Zone::RIGHT;
    } else if (_tbounds.contains(pos)) {
        return Zone::MAIN;
    }
    return Zone::UNDEFINED;
}

/**
 * Returns the scene location of a touch
 *
 * Touch coordinates are inverted, with y origin in the top-left
 * corner. This method corrects for this and scales the screen
 * coordinates down on to the scene graph size.
 *
 * @return the scene location of a touch
 */
Vec2 InputController::touch2Screen(const Vec2 pos) const {
    float px = pos.x/_tbounds.size.width -_tbounds.origin.x;
    float py = pos.y/_tbounds.size.height-_tbounds.origin.y;
    Vec2 result;
    result.x = px*_sbounds.size.width +_sbounds.origin.x;
    result.y = (1-py)*_sbounds.size.height+_sbounds.origin.y;
    return result;
}

/**
 * Processes movement for the floating joystick.
 *
 * This will register movement as left or right (or neither).  It
 * will also move the joystick anchor if the touch position moves
 * too far.
 *
 * @param  pos  the current joystick position
 */
void InputController::processJoystick(const cugl::Vec2 pos) {
    Vec2 diff =  _ltouch.position-pos;

    // Reset the anchor if we drifted too far
    if (diff.lengthSquared() > JSTICK_RADIUS*JSTICK_RADIUS) {
        diff.normalize();
        diff *= (JSTICK_RADIUS+JSTICK_DEADZONE)/2;
        _ltouch.position = pos+diff;
    }
    _joycenter = touch2Screen(_ltouch.position);
    _joycenter.y += JSTICK_OFFSET;
    if (std::fabsf(diff.length()) > JSTICK_DEADZONE) {
        _joystick = true;
        Vec2 unitVec = diff.getNormalization();
        _moveDir = unitVec;
        _moveDir.x = -_moveDir.x;
    } else {
        _joystick = false;
        _moveDir = Vec2::ZERO;
    }
}

#pragma mark -
#pragma mark Touch Callbacks

void InputController::touchDownCB(const cugl::TouchEvent& event, bool focus) {
    Vec2 pos = event.position;
    Zone zone = getZone(pos);
    switch (zone) {
        case Zone::LEFT:
            // Only process if no touch in zone
            if (_ltouch.touchids.empty()) {
                // Left is the floating joystick
                _ltouch.position = event.position;
                _ltouch.timestamp.mark();
                _ltouch.touchids.insert(event.touch);
                
                _joystick = true;
                _joycenter = touch2Screen(event.position);
                _joycenter.y += JSTICK_OFFSET;
            }
            break;
        case Zone::RIGHT:
            break;
        case Zone::MAIN:
            break;
        default:
            CUAssertLog(false, "Touch is out of bounds");
            break;
    }
}

void InputController::touchUpCB(const cugl::TouchEvent& event, bool focus) {
    Vec2 pos = event.position;
    if (_ltouch.touchids.find(event.touch) != _ltouch.touchids.end()) {
        _ltouch.touchids.clear();
        _moveDir = Vec2::ZERO;
        _joystick = false;
    }
}

void InputController::touchesMovedCB(const TouchEvent& event, const Vec2& previous, bool focus) {
    Vec2 pos = event.position;
    if (_ltouch.touchids.find(event.touch) != _ltouch.touchids.end()) {
        processJoystick(pos);
    }
}
