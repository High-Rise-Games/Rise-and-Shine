//
//  CUAcclerometer.cpp
//  Cornell University Game Library (CUGL)
//
//  This class provides basic acclerometer support. It uses the joystick
//  subsystem, which is guaranteed to work on both iOS and Android.
//
//  This class is a singleton and should never be allocated directly.  It
//  should only be accessed via the Input dispatcher.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 7/12/16
#include <cugl/input/CUGameController.h>
#include <cugl/base/CUDisplay.h>
#include <SDL.h>
#include <limits.h>
#include <sstream>
#include <iomanip>

using namespace cugl;

/**
 * Returns the float equivalent of a Sint16 value
 *
 * This function is used to normalize axis results
 *
 * @param value The value to convert
 *
 * @return the float equivalent of a Sint16 value
 */
static float Sint16tofloat(Sint16 value) {
    if (value == -32768) {
        return -1.0;
    }
    return value/32767.0f;
}

#pragma mark -
#pragma mark Game Controller Events
/**
 * Constructs a new hat event with the given values
 *
 * @param key   The device UID
 * @param ind   The hat index
 * @param pos   The hat position (in SDL encoding)
 * @param stamp The timestamp for the event
 */
GameControllerHatEvent::GameControllerHatEvent(const std::string key, Uint8 ind, Uint8 pos,
                                               const Timestamp& stamp) {
    uuid = key;
    index = ind;
    timestamp = stamp;
    switch (pos) {
        case SDL_HAT_CENTERED:
            state = GameController::Hat::CENTERED;
            break;
        case SDL_HAT_LEFTUP:
            state = GameController::Hat::LEFT_UP;
            break;
       case SDL_HAT_UP:
            state = GameController::Hat::UP;
            break;
        case SDL_HAT_RIGHTUP:
            state = GameController::Hat::RIGHT_UP;
            break;
        case SDL_HAT_RIGHT:
            state = GameController::Hat::RIGHT;
            break;
        case SDL_HAT_RIGHTDOWN:
            state = GameController::Hat::RIGHT_DOWN;
            break;
        case SDL_HAT_DOWN:
            state = GameController::Hat::DOWN;
            break;
        case SDL_HAT_LEFTDOWN:
            state = GameController::Hat::LEFT_DOWN;
            break;
        case SDL_HAT_LEFT:
            state = GameController::Hat::LEFT_DOWN;
            break;
        default:
            state = GameController::Hat::CENTERED;
            break;
    }
}

#pragma mark -
#pragma mark Game Controller
/**
 * Creates a degnerate game controller
 *
 * This game controller is not actually attached to any devices. To
 * activate a game controller, use {@link GameControllerInput#open}
 * instead.
 */
GameController::GameController() : _input(NULL), _ballCount(0), _focus(0) {}

/**
 * Initializes this device, acquiring any necessary resources
 *
 * @param index The device index
 * @param uid   The device uid (assigned by {@link GameControllerInput})
 *
 * @return true if initialization was successful
 */
bool GameController::init(int index, const std::string uid) {
    _input = SDL_JoystickOpen(index);
    if (_input != NULL) {
        _uid = uid;
        _name = SDL_JoystickName(_input);
        
        for(int ii = 0; ii < SDL_JoystickNumAxes(_input); ii++) {
            _axisState.push_back(false);
        }
        for(int ii = 0; ii < SDL_JoystickNumHats(_input); ii++) {
            _hatState.push_back(false);
        }
        for(int ii = 0; ii < SDL_JoystickNumButtons(_input); ii++) {
            _buttonState.push_back(false);
        }
        _ballCount = SDL_JoystickNumBalls(_input);
        
        return true;
    }
    return false;
}

/**
 * Unintializes this device, returning it to its default state
 *
 * An uninitialized device may not work without reinitialization.
 */
void GameController::dispose() {
    if (_input) {
        SDL_JoystickClose(_input);
        _input = NULL;
    }
    _uid = "";
    _name = "";
    _ballCount = 0;
    _axisState.clear();
    _hatState.clear();
    _buttonState.clear();
}

#pragma mark Input Methods
/**
 * Cleans up a game controller after it has been disconnected.
 *
 * This method is similar to {@link dispose} except that it is aware that
 * SDL has closed the game controller already.
 */
void GameController::forceClose() {
    _input = NULL;
    dispose();
}

/**
 * Closes this game controller, releasing all resources.
 *
 * This method not invalidates this game controller, so any shared pointers
 * still referring to this controller are no longer usable. The only way
 * to access the game controller again is to call
 * {@link GameControllerInput#open}.
 *
 * It is often better to call the method {@link GameControllerInput#close}
 * instead of this one.
 */
void GameController::close() {
    Input::get<GameControllerInput>()->close(_uid);
}

/**
 * Clears the state of this input device, readying it for the next frame.
 *
 * Many devices keep track of what happened "this" frame. This method is
 * necessary to advance the frame.
 */
void GameController::clearState() {
    for(auto it = _axisState.begin(); it != _axisState.end(); ++it) {
        *it = false;
    }
    for(auto it = _hatState.begin(); it != _hatState.end(); ++it) {
        *it = false;
    }
    for(auto it = _buttonState.begin(); it != _buttonState.end(); ++it) {
        *it = false;
    }
}

/**
 * Records that an {@link GameControllerAxisEvent} has occured
 *
 * @param axis  The axis index
 * @param value The axis value in [-1,1]
 * @param stamp The event timestamp
 */
void GameController::reportAxis(Uint8 axis, float value, const Timestamp& stamp) {
    if (!_axisListeners.empty()) {
        GameControllerAxisEvent event(_uid,axis,value,stamp);
        for(auto it = _axisListeners.begin(); it != _axisListeners.end(); ++it) {
            it->second(event,it->first == _focus);
        }
    }
    _axisState[axis] = true;
}

/**
 * Records that an {@link GameControllerBallEvent} has occured
 *
 * @param ball  The track ball index
 * @param xrel  The movement along the x-axis
 * @param yrel  The movement along the y-axis
 * @param stamp The event timestamp
 */
void GameController::reportBall(Uint8 ball, float xrel, float yrel,
                                const Timestamp& stamp) {
    if (!_ballListeners.empty()) {
        GameControllerBallEvent event(_uid,ball,xrel,yrel,stamp);
        for(auto it = _ballListeners.begin(); it != _ballListeners.end(); ++it) {
            it->second(event,it->first == _focus);
        }
    }
}

/**
 * Records that an {@link GameControllerHatEvent} has occured
 *
 * @param hat   The hat index
 * @param value The hat position
 * @param stamp The event timestamp
 */
void GameController::reportHat(Uint8 hat, Uint8 value, const Timestamp& stamp) {
    if (!_hatListeners.empty()) {
        GameControllerHatEvent event(_uid,hat,value,stamp);
        for(auto it = _hatListeners.begin(); it != _hatListeners.end(); ++it) {
            it->second(event,it->first == _focus);
        }
    }
    _hatState[hat] = true;
}

/**
 * Records that an {@link GameControllerButtonEvent} has occured
 *
 * @param button    The button index
 * @param down      Whether the button is down
 * @param stamp     The event timestamp
 */
void GameController::reportButton(Uint8 button, bool down, const Timestamp& stamp) {
    if (!_buttonListeners.empty()) {
        GameControllerButtonEvent event(_uid,button,down,stamp);
        for(auto it = _buttonListeners.begin(); it != _buttonListeners.end(); ++it) {
            it->second(event,it->first == _focus);
        }
    }
    _buttonState[button] = true;
}

#pragma mark Attributes
/**
 * Returns the internal SDL identifier for this game controller
 *
 * @return the internal SDL identifier for this game controller
 */
SDL_JoystickID GameController::getJoystickID() const {
    return SDL_JoystickInstanceID(_input);
}

/**
 * Returns the number of axes.
 *
 * An axis is either a single axis of a analogue joystick (which consists
 * of two perpendicular axes) or a trigger.
 *
 * Axes are refered to by their index. The index of an axis is any positive
 * integer less than the returned value.
 *
 * @return the number of axes.
 */
Uint8 GameController::numberAxes() const {
    return _axisState.size();
}

/**
 * Returns the number of track balls.
 *
 * Trackballs only measure a change in position. They do not have a absolute
 * position like axes do. Most joysticks do not have track balls.
 *
 * Track balls are refered to by their index. The index of a track ball is
 * any positive integer less than the returned value.
 *
 * @return the number of track balls.
 */
Uint8 GameController::numberBalls() const {
    return _ballCount;
}

/**
 * Returns the number of hats.
 *
 * A "hat" is the official name of a D-Pad. Its state can be centered
 * (untouched) or one of the eight cardinal directions. Note that just
 * because a game controller has a D-Pad does not mean it has a hat.
 * Some game controllers treat their D-Pad as four separate buttons
 * (one for each direction).
 *
 * Hats are refered to by their index. The index of a hat is any positive
 * integer less than the returned value.
 *
 * @return the number of hats.
 */
Uint8 GameController::numberHats() const {
    return _hatState.size();
}

/**
 * Returns the number of buttons.
 *
 * A button is simply that -- something that can be pressed.  Most game
 * controllers have the classic 4 buttons (`X`, `Y`, `A`, `B` on XBox
 * controllers). But the start and "back" buttons also count as buttons,
 * as do any bumpers above the triggers. Finally, on some controllers
 * the D-Pad is interpretted as four buttons instead of as hat.
 *
 * Buttons are refered to by their index. The index of a button is any
 * positive integer less than the returned value.
 *
 * @return the number of buttons.
 */
Uint8 GameController::numberButtons() const {
    return _buttonState.size();
}

#pragma mark Haptics
/**
 * Returns true if this controller supports general rumble effects
 *
 * @return true if this controller supports general rumble effects
 */
bool GameController::hasRumble() const {
    if (_input == NULL) {
        return false;
    }
    return SDL_JoystickHasRumble(_input) == SDL_TRUE;
}

/**
 * Returns true if this controller supports trigger rumble effects
 *
 * @return true if this controller supports trigger rumble effects
 */
bool GameController::hasRumbleTriggers() const {
    if (_input == NULL) {
        return false;
    }
    return SDL_JoystickHasRumbleTriggers(_input) == SDL_TRUE;
}

/**
 * Starts a rumble effect for this controller.
 *
 * @param low_freq  The intensity of the low frequency (left) rumble motor
 * @param high_freq The intensity of the high frequency (right) rumble motor
 * @param duration  The rumble duration in milliseconds
 */
void GameController::applyRumble(Uint16 low_freq, Uint16 high_freq, Uint32 duration) {
    if (_input == NULL) {
        CUAssertLog(_input, "Game controller has been closed");
        return ;
    }
    SDL_JoystickRumble(_input, low_freq, high_freq, duration);
}

/**
 * Starts a rumble effect for this controller.
 *
 * @param left      The intensity of the left trigger rumble motor
 * @param right     The intensity of the right trigger rumble motor
 * @param duration  The rumble duration in milliseconds
 */
void GameController::hasRumbleTriggers(Uint16 left, Uint16 right, Uint32 duration) const {
    if (_input == NULL) {
        CUAssertLog(_input, "Game controller has been closed");
        return ;
    }
    SDL_JoystickRumbleTriggers(_input, left, right, duration);
}

#pragma mark Data Polling
/**
 * Returns the current axis position.
 *
 * The axis position is a value between -1 and 1, inclusive. It is often
 * either an axis of an analogue joystick or a trigger.  If it is a
 * joystick, then its default value will be near 0. It is a trigger, then
 * its default value will be -1.
 *
 * This class does not implement any dead zones (e.g. axis values to be
 * ignored and treated as the default position). That is the responsibility
 * of the user. A standard recommendation for a deadzone is -0.8 or lower
 * for triggers and -0.2 to 0.2 for joysticks.
 *
 * @param axis  The axis index
 */
float GameController::getAxisPosition(Uint8 axis) const {
    if (_input == NULL) {
        CUAssertLog(_input, "Game controller has been closed");
        return 0.0f;
    }
    
    CUAssertLog(axis < _axisState.size(), "Axis %d is out of range",axis);
    Sint16 value = SDL_JoystickGetAxis(_input, axis);
    return Sint16tofloat(value);
}

/**
 * Returns true if the given axis changed position this frame.
 *
 * @param axis  The axis index
 *
 * @return true if the given axis changed position this frame.
 */
bool GameController::axisDidChange(Uint8 axis) const {
    if (_input == NULL) {
        CUAssertLog(_input, "Game controller has been closed");
        return false;
    }

    CUAssertLog(axis < _axisState.size(), "Axis %d is out of range",axis);
    return _axisState[axis];
}

/**
 * Returns true if the given axis is a trigger.
 *
 * Triggers are axes whose default value is -1 (so they are one-sided).
 *
 * @param axis  The axis index
 *
 * @return true if the given axis is a trigger.
 */
bool GameController::axisIsTrigger(Uint8 axis) const {
    if (_input == NULL) {
        CUAssertLog(_input, "Game controller has been closed");
        return false;
    }
    
    CUAssertLog(axis < _axisState.size(), "Axis %d is out of range",axis);
    Sint16 state;
    SDL_bool good = SDL_JoystickGetAxisInitialState(_input,axis,&state);
    return good && state == -32768;
}

/**
 * Returns the relative motion of the track ball.
 *
 * The vector coordinates are between -1 and 1, inclusive. The values 1
 * or -1 represent maximum movement in a direction. The values returned
 * are the relative movement since the last animation frame. A zero vector
 * means that the track ball did not move.
 *
 * @param ball  The track ball index
 *
 * @return the relative motion of the track ball.
 */
Vec2 GameController::getBallOffset(Uint8 ball) const {
    if (_input == NULL) {
        CUAssertLog(_input, "Game controller has been closed");
        return Vec2::ZERO;
    }
    
    CUAssertLog(ball < _ballCount, "Track ball %d is out of range",ball);
    int x, y;
    SDL_JoystickGetBall(_input, ball, &x, &y);
    return Vec2(Sint16tofloat((Sint16)x),Sint16tofloat((Sint16)y));
}
  
/**
 * Returns the hat position
 *
 * A "hat" is the official name of a D-Pad. Its state can be centered
 * (untouched) or one of the eight cardinal directions. Note that just
 * because a game controller has a D-Pad does not mean it has a hat.
 * Some game controllers treat their D-Pad as four separate buttons
 * (one for each direction).
 *
 * @param hat   The hat index
 *
 * @return the hat position
 */
GameController::Hat GameController::getHatPosition(Uint8 hat) const {
    if (_input == NULL) {
        CUAssertLog(_input, "Game controller has been closed");
        return GameController::Hat::CENTERED;
    }
    
    CUAssertLog(hat < _hatState.size(), "Hat %d is out of range",hat);
    Uint8 value = SDL_JoystickGetHat(_input, hat);
    switch (value) {
        case SDL_HAT_CENTERED:
            return GameController::Hat::CENTERED;
        case SDL_HAT_LEFTUP:
            return GameController::Hat::LEFT_UP;
        case SDL_HAT_UP:
            return GameController::Hat::UP;
        case SDL_HAT_RIGHTUP:
            return GameController::Hat::RIGHT_UP;
        case SDL_HAT_RIGHT:
            return GameController::Hat::RIGHT;
        case SDL_HAT_RIGHTDOWN:
            return GameController::Hat::RIGHT_DOWN;
        case SDL_HAT_DOWN:
            return GameController::Hat::DOWN;
        case SDL_HAT_LEFTDOWN:
            return GameController::Hat::LEFT_DOWN;
        case SDL_HAT_LEFT:
            return GameController::Hat::LEFT_DOWN;
    }

    return GameController::Hat::CENTERED;
}

/**
 * Returns true if the given hat changed position this frame.
 *
 * @param hat   The hat index
 *
 * @return true if the given hat changed position this frame.
 */
bool GameController::hatDidChange(Uint8 hat) const {
    if (_input == NULL) {
        CUAssertLog(_input, "Game controller has been closed");
        return false;
    }
    
    CUAssertLog(hat < _hatState.size(), "Hat %d is out of range",hat);
    return _hatState[hat];
}

/**
 * Returns true if the given button is currently down.
 *
 * This method does not distinguish presses or releases and will return
 * true the duration of a button hold.
 *
 * @param button    The button index
 *
 * @return true if the given button is currently down.
 */
bool GameController::isButtonDown(Uint8 button) const {
    if (_input == NULL) {
        CUAssertLog(_input, "Game controller has been closed");
        return false;
    }

    CUAssertLog(button < _buttonState.size(), "Button %d is out of range",button);
    return SDL_JoystickGetButton(_input, button) != 0;
}

/**
 * Returns true if the given button was pressed this frame.
 *
 * A press means that the button is down this animation frame, but was not
 * down the previous frame.
 *
 * @param button    The button index
 *
 * @return true if the given button was pressed this frame.
 */
bool GameController::isButtonPressed(Uint8 button) const {
    if (_input == NULL) {
        CUAssertLog(_input, "Game controller has been closed");
        return false;
    }

    CUAssertLog(button < _buttonState.size(), "Button %d is out of range",button);
    return SDL_JoystickGetButton(_input, button) != 0 && _buttonState[button];
}

/**
 * Returns true if the given button was released this frame.
 *
 * A release means that the button is up this animation frame, but was not
 * up the previous frame.
 *
 * @param button    The button index
 *
 * @return true if the given button was released this frame.
 */
bool GameController::isButtonReleased(Uint8 button) const {
    if (_input == NULL) {
        CUAssertLog(_input, "Game controller has been closed");
        return false;
    }

    CUAssertLog(button < _buttonState.size(), "Button %d is out of range",button);
    return SDL_JoystickGetButton(_input, button) == 0 && _buttonState[button];
}


#pragma mark Listener Methods
/**
 * Requests focus for the given identifier
 *
 * Only an active listener can have focus. This method returns false if
 * the key does not refer to an active listener (of any type). Note that
 * keys may be shared across listeners of different types, but must be
 * unique for each listener type.
 *
 * @param key   The identifier for the focus object
 *
 * @return false if key does not refer to an active listener
 */
bool GameController::requestFocus(Uint32 key)  {
    if (isListener(key)) {
        _focus = key;
        return true;
    }
    return false;
}

/**
 * Returns true if key represents a listener object
 *
 * An object is a listener if it is a listener for any of the four actions:
 * axis movement, ball movement, hat movement, or button press/release.
 *
 * @param key   The identifier for the listener
 *
 * @return true if key represents a listener object
 */
bool GameController::isListener(Uint32 key) const {
    bool result = _axisListeners.find(key) != _axisListeners.end();
    result = result || _ballListeners.find(key) != _ballListeners.end();
    result = result || _hatListeners.find(key) != _hatListeners.end();
    result = result || _buttonListeners.find(key) != _buttonListeners.end();
    return result;
}

/**
 * Returns the axis motion listener for the given object key
 *
 * This listener is invoked when an axis changes position.
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the axis motion listener for the given object key
 */
const GameController::AxisListener GameController::getAxisListener(Uint32 key) const {
    if (_axisListeners.find(key) != _axisListeners.end()) {
        return (_axisListeners.at(key));
    }
    return nullptr;
}

/**
 * Returns the ball motion listener for the given object key
 *
 * This listener is invoked when a track ball is moved.
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the ball motion listener for the given object key
 */
const GameController::BallListener GameController::getBallListener(Uint32 key) const {
    if (_ballListeners.find(key) != _ballListeners.end()) {
        return (_ballListeners.at(key));
    }
    return nullptr;
}

/**
 * Returns the hat listener for the given object key
 *
 * This listener is invoked when the hat changes position.
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the hat listener for the given object key
 */
const GameController::HatListener GameController::getHatListener(Uint32 key) const {
    if (_hatListeners.find(key) != _hatListeners.end()) {
        return (_hatListeners.at(key));
    }
    return nullptr;
}

/**
 * Returns the button listener for the given object key
 *
 * This listener is invoked when the button changes state. So it is
 * invoked on a press or a release, but not a hold.
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the button listener for the given object key
 */
const GameController::ButtonListener GameController::getButtonListener(Uint32 key) const {
    if (_buttonListeners.find(key) != _buttonListeners.end()) {
        return (_buttonListeners.at(key));
    }
    return nullptr;
}

/**
 * Adds an axis motion listener for the given object key
 *
 * There can only be one axis listener for a given key (though you may
 * share keys across other listener types). If a listener already exists
 * for the key, the method will fail and return false.  You must remove
 * a listener before adding a new one for the same key.
 *
 * This listener is invoked when an axis changes position.
 *
 * @param key       The identifier for the listener
 * @param listener  The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool GameController::addAxisListener(Uint32 key, AxisListener listener) {
    if (_axisListeners.find(key) == _axisListeners.end()) {
        _axisListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Adds a ball motion listener for the given object key
 *
 * There can only be one ball listener for a given key (though you may
 * share keys across other listener types). If a listener already exists
 * for the key, the method will fail and return false.  You must remove
 * a listener before adding a new one for the same key.
 *
 * This listener is invoked when a track ball is moved.
 *
 * @param key       The identifier for the listener
 * @param listener  The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool GameController::addBallListener(Uint32 key, BallListener listener) {
    if (_ballListeners.find(key) == _ballListeners.end()) {
        _ballListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Adds a hat listener for the given object key
 *
 * There can only be one hat listener for a given key (though you may
 * share keys across other listener types). If a listener already exists
 * for the key, the method will fail and return false.  You must remove
 * a listener before adding a new one for the same key.
 *
 * This listener is invoked when the hat changes position.
 *
 * @param key       The identifier for the listener
 * @param listener  The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool GameController::addHatListener(Uint32 key, HatListener listener) {
    if (_hatListeners.find(key) == _hatListeners.end()) {
        _hatListeners[key] = listener;
        return true;
    }
    return false;
}


/**
 * Adds a button listener for the given object key
 *
 * There can only be one button listener for a given key (though you may
 * share keys across other listener types). If a listener already exists
 * for the key, the method will fail and return false.  You must remove
 * a listener before adding a new one for the same key.
 *
 * This listener is invoked when the button changes state. So it is
 * invoked on a press or a release, but not a hold.
 *
 * @param key       The identifier for the listener
 * @param listener  The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool GameController::addButtonListener(Uint32 key, ButtonListener listener) {
    if (_buttonListeners.find(key) == _buttonListeners.end()) {
        _buttonListeners[key] = listener;
        return true;
    }
    return false;
}


/**
 * Removes the axis motion listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * This listener is invoked when an axis changes position.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool GameController::removeAxisListener(Uint32 key) {
    if (_axisListeners.find(key) != _axisListeners.end()) {
        _axisListeners.erase(key);
        return true;
    }
    return false;
}

/**
 * Removes the ball motion listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * This listener is invoked when a track ball is moved.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool GameController::removeBallListener(Uint32 key) {
    if (_ballListeners.find(key) != _ballListeners.end()) {
        _ballListeners.erase(key);
        return true;
    }
    return false;
}

/**
 * Removes the hat listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false. This method will succeed if there is a drag listener for
 * the given key, even if the pointer awareness if BUTTON.
 *
 * This listener is invoked when the hat changes position.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool GameController::removeHatListener(Uint32 key) {
    if (_hatListeners.find(key) != _hatListeners.end()) {
        _hatListeners.erase(key);
        return true;
    }
    return false;
}

/**
 * Removes the button listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false. This method will succeed if there is a motion listener
 * for the given key, even if the pointer awareness if BUTTON or DRAG.
 *
 * This listener is invoked when the button changes state. So it is
 * invoked on a press or a release, but not a hold.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool GameController::removeButtonListener(Uint32 key) {
    if (_buttonListeners.find(key) != _buttonListeners.end()) {
        _buttonListeners.erase(key);
        return true;
    }
    return false;
}

#pragma mark -
#pragma mark GameControllerInput

/**
 * Initializes this device, acquiring any necessary resources
 *
 * @return true if initialization was successful
 */
bool GameControllerInput::init() {
    scanDevices();
    return true;
}

/**
 * Unintializes this device, returning it to its default state
 *
 * An uninitialized device may not work without reinitialization.
 */
void GameControllerInput::dispose() {
    for (auto it = _bysdl.begin(); it != _bysdl.end(); ++it) {
        it->second->dispose();
    }
    _devices.clear();
    _names.clear();
    _joyids.clear();
    _bysdl.clear();
    _byname.clear();
    _listeners.clear();
}

#pragma mark Internals
/**
 * Adds a new device to this manager.
 *
 * This method generates the UID that we use to reference the devices.
 * It is called when the manager is first initialized (when it scans
 * all connected devices) and when a new device is connected.
 *
 * @param index The SDL device index
 *
 * @return the UID of the new device
 */
std::string  GameControllerInput::addDevice(int index) {
    // First create the hash
    std::stringstream ss;
    std::hash<std::string> hasher;
    
    std::string name = SDL_JoystickNameForIndex(index);
    SDL_JoystickID key = (Uint32)SDL_JoystickGetDeviceInstanceID(index);

    // Construct a unique identifier
    size_t data = hasher(name);
    Uint16 salt = (key >> 16) ^ (key & 0xffff);

    ss << std::uppercase << std::setfill('0') << std::setw(4) << std::hex
       << ((data >> 48) & 0xffff);
    ss << "-";
    ss << std::uppercase << std::setfill('0') << std::setw(4) << std::hex
       << ((data >> 32) & 0xffff);
    ss << "-";
    ss << std::uppercase << std::setfill('0') << std::setw(4) << std::hex
       << ((data >> 16) & 0xffff);
    ss << "-";
    ss << std::uppercase << std::setfill('0') << std::setw(4) << std::hex
       << ((data & 0xffff) ^ salt);

    std::string uuid = ss.str();

    _devices[uuid] = index;
    _names[uuid] = name;
    _joyids[key] = uuid;
    return uuid;
}

/**
 * Removes a new device to this manager.
 *
 * This method will close the associated game controller, invalidating
 * any references to it. It is called whenever a device becomes
 * disconnected.
 *
 * @param jid   The SDL instance identifier
 *
 * @return the UID of the removed device
 */
std::string  GameControllerInput::removeDevice(SDL_JoystickID jid) {
    auto jt = _joyids.find(jid);
    if (jt == _joyids.end()) {
        return "<UNKNOWN>";
    }

    std::string uid = jt->second;
    _devices.erase(jt->second);
    _names.erase(jt->second);
    _joyids.erase(jt);
    
    // See if this joystick was active
    auto kt = _bysdl.find(jid);
    if (kt != _bysdl.end()) {
        kt->second->forceClose();
        _bysdl.erase(kt);
        _byname.erase(uid);
    }
    
    // Reassign available
    for(int ii = 0; ii < SDL_NumJoysticks(); ii++) {
        SDL_JoystickID key = SDL_JoystickGetDeviceInstanceID(ii);
        jt = _joyids.find(key);
        if (jt != _joyids.end()) {
            _devices[jt->second] = ii;
        }
    }
    
    return uid;
}

/**
 * Scans all connected game controllers, adding them to this manager.
 *
 * This method depends on {@link #doesFilter} for what it considers a
 * valid game controller.
 */
void GameControllerInput::scanDevices() {
    _devices.clear();
    _name.clear();
    _joyids.clear();
    for(int ii = 0; ii < SDL_NumJoysticks(); ii++) {
        if (!_filter || SDL_IsGameController(ii)) {
            addDevice(ii);
        }
    }
}

#pragma mark Attributes
/**
 * Sets whether this game controller manager filters its devices
 *
 * Our game controller manager is an interface built on top of the SDL
 * joystick functions. However, SDL has a very broad definition of
 * joystick, and uses it to include things like an accelerometer. If
 * this value is true (which is the default), then only devices which
 * match traditional game controllers will be listed.
 *
 * @param value whether this game controller manager filters its devices
 */
void GameControllerInput::filter(bool value) {
    if (_filter == value) {
        return;
    }
    if (value) {
        _filter = true;
        scanDevices();
        
        // Make sure all open devices correspond to an instance
        for(auto it = _bysdl.begin(); it != _bysdl.end(); ) {
            auto jt = _joyids.find(it->first);
            if (jt == _joyids.end()) {
                // Expensive miss. Find device and add it.
                bool found = false;
                for(int kk = 0; kk < SDL_NumJoysticks(); kk++) {
                    SDL_JoystickID jid = SDL_JoystickGetDeviceInstanceID(kk);
                    if (jid == it->first) {
                        addDevice(kk);
                    }
                }
                if (!found) {
                    std::string name = it->second->getName();
                    _byname.erase(name);
                    it->second->dispose();
                    it = _bysdl.erase(it);
                } else {
                    it++;
                }
            } else {
                it++;
            }
        }
    } else {
        _filter = false;
        scanDevices();
    }
}

/**
 * Returns the number of connected devices.
 *
 * This value will be affected by {@link #doesFilter}.
 *
 * @return the number of connected devices.
 */
size_t GameControllerInput::size() const {
    return _devices.size();
}

#pragma mark Device Access
/**
 * Returns the list of connected devices.
 *
 * The list is a vector of unique identifiers (UID) used to identify
 * each connected controller. These identifiers are not very descriptive,
 * as they are designed to be compact and unique. For a description of
 * each device, use {@link #getName}.
 *
 * @return the list of connected devices.
 */
std::vector<std::string> GameControllerInput::devices() const {
    std::vector<std::string> result;
    for(auto it = _devices.begin(); it != _devices.end(); ++it) {
        result.push_back(it->first);
    }
    return result;
}

/**
 * Returns a descriptive name for the given device.
 *
 * The UID for the device should be one listed in {@link #devices()}. If
 * the device does not exist, it will return the empty string.
 *
 * @param uid   The device UID
 *
 * @return a descriptive name for the given device.
 */
std::string GameControllerInput::getName(const std::string key) {
    auto jt = _names.find(key);
    if (jt == _names.end()) {
        return "";
    }
    return jt->second;
}

/**
 * Returns a reference to a newly actived game controller.
 *
 * This method activates the game controller with the given UID. If the
 * game controller is already active, it simply returns a reference to
 * the existing game controller. The UID for the device should be one
 * listed in {@link #devices()}. If the device does not exist, this method
 * will return nullptr.
 *
 * It is generally a good idea to close all game controllers when you
 * are done with them. However, deactivating this game controller
 * automatically disposes of any active controllers.
 *
 * @param uid   The device UID
 *
 * @return a reference to a newly actived game controller.
 */
std::shared_ptr<GameController> GameControllerInput::open(const std::string uid) {
    // Make the device exists
    auto jt = _devices.find(uid);
    if (jt == _devices.end()) {
        return nullptr;
    }
    Uint32 index = jt->second;
    
    // Make sure we have not already opened it
    SDL_JoystickID jid = SDL_JoystickGetDeviceInstanceID(index);
    auto kt = _bysdl.find(jid);
    if (kt != _bysdl.end()) {
        return kt->second;
    }
    
    std::shared_ptr<GameController> result = std::make_shared<GameController>();
    if (result->init(index, uid)) {
        _bysdl[jid] = result;
        _byname[uid] = result;
        return result;
    }
    
    return nullptr;
}

/**
 * Returns a reference to the given game controller.
 *
 * This method assumes the game controller for this UID has already been
 * activated. The UID for the device should be one listed in
 * {@link #devices()}. If the device does not exist, or the device has
 * not been activate, this method will return nullptr.
 *
 * @param uid   The device UID
 *
 * @return a reference to the given game controller.
 */
std::shared_ptr<GameController> GameControllerInput::get(const std::string name) {
    // Make the device exists
    auto jt = _byname.find(name);
    if (jt != _byname.end()) {
        return jt->second;
    }
    return nullptr;
}

/**
 * Closes the game controller for the given UID
 *
 * This invalidates all references to the game controller, making them
 * no longer usable. The only way to access the game controller again is
 * to call {@link #open}.
 *
 * @param uid   The device UID
 */
void GameControllerInput::close(const std::string name) {
    // Make the device exists
    auto jt = _byname.find(name);
    if (jt == _byname.end()) {
        return;
    }
    SDL_JoystickID jid = jt->second->getJoystickID();
    jt->second->dispose();
    _bysdl.erase(jid);
    _byname.erase(jt);
}

#pragma mark Listener Methods
/**
 * Requests focus for the given identifier
 *
 * Only a listener can have focus.  This method returns false if key
 * does not refer to an active listener
 *
 * @param key   The identifier for the focus object
 *
 * @return false if key does not refer to an active listener
 */
bool GameControllerInput::requestFocus(Uint32 key) {
    if (isListener(key)) {
        _focus = key;
        return true;
    }
    return false;
}

/**
 * Returns true if key represents a listener object
 *
 * @param key   The identifier for the listener
 *
 * @return true if key represents a listener object
 */
bool GameControllerInput::isListener(Uint32 key) const {
    return _listeners.find(key) != _listeners.end();
}

/**
 * Returns the game controller input listener for the given object key
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the game controller input listener for the given object key
 */
const GameControllerInput::Listener GameControllerInput::getListener(Uint32 key) const {
    if (isListener(key)) {
        return (_listeners.at(key));
    }
    return nullptr;
}

/**
 * Adds an game controller input listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * @param key       The identifier for the listener
 * @param listener  The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool GameControllerInput::addListener(Uint32 key,
                                      GameControllerInput::Listener listener) {
    if (!isListener(key)) {
        _listeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Removes the game controller input listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool GameControllerInput::removeListener(Uint32 key) {
    if (isListener(key)) {
        _listeners.erase(key);
        return true;
    }
    return false;
}

#pragma mark Input Device Methods
/**
 * Clears the state of this input device, readying it for the next frame.
 *
 * Many devices keep track of what happened "this" frame.  This method is
 * necessary to advance the frame.
 */
void GameControllerInput::clearState() {
    for(auto it = _bysdl.begin(); it != _bysdl.end(); ++it) {
        it->second->clearState();
    }
}

/**
 * Processes an SDL_Event
 *
 * The dispatcher guarantees that an input device only receives events that
 * it subscribes to.
 *
 * @param event The input event to process
 * @param stamp The event timestamp in CUGL time
 *
 * @return false if the input indicates that the application should quit.
 */
bool GameControllerInput::updateState(const SDL_Event& event,
                                      const Timestamp& stamp) {
    switch (event.type) {
        case SDL_JOYAXISMOTION:
        {
            SDL_JoyAxisEvent jevt = event.jaxis;
            auto jt = _bysdl.find(jevt.which);
            if (jt != _bysdl.end()) {
                jt->second->reportAxis(jevt.axis,Sint16tofloat(jevt.value),stamp);
            }
        }
            break;
        case SDL_JOYBALLMOTION:
        {
            SDL_JoyBallEvent jevt = event.jball;
            auto jt = _bysdl.find(jevt.which);
            if (jt != _bysdl.end()) {
                jt->second->reportBall(jevt.ball,
                                       Sint16tofloat(jevt.xrel),
                                       Sint16tofloat(jevt.yrel),
                                       stamp);
            }
        }
            break;
        case SDL_JOYHATMOTION:
        {
            SDL_JoyHatEvent jevt = event.jhat;
            auto jt = _bysdl.find(jevt.which);
            if (jt != _bysdl.end()) {
                jt->second->reportHat(jevt.hat, jevt.value, stamp);
            }
        }
            break;
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
        {
            SDL_JoyButtonEvent jevt = event.jbutton;
            auto jt = _bysdl.find(jevt.which);
            if (jt != _bysdl.end()) {
                bool down = jevt.state == SDL_PRESSED;
                jt->second->reportButton(jevt.button, down, stamp);
            }
        }
            break;
        case SDL_JOYDEVICEADDED:
        {
            SDL_JoyDeviceEvent jevt = event.jdevice;
            std::string uid = addDevice(jevt.which);
            GameControllerInputEvent gevent(uid,true,stamp);
        }
            break;
        case SDL_JOYDEVICEREMOVED:
        {
            SDL_JoyDeviceEvent jevt = event.jdevice;
            std::string uid = removeDevice(jevt.which);
            GameControllerInputEvent gevent(uid,false,stamp);
        }
            break;
        default:
            return true;
    }
    return true;
}

/**
 * Determine the SDL events of relevance and store there types in eventset.
 *
 * An SDL_EventType is really Uint32.  This method stores the SDL event
 * types for this input device into the vector eventset, appending them
 * to the end. The Input dispatcher then uses this information to set up
 * subscriptions.
 *
 * @param eventset  The set to store the event types.
 */
void GameControllerInput::queryEvents(std::vector<Uint32>& eventset) {
    eventset.push_back((Uint32)SDL_JOYAXISMOTION);
    eventset.push_back((Uint32)SDL_JOYBALLMOTION);
    eventset.push_back((Uint32)SDL_JOYBUTTONDOWN);
    eventset.push_back((Uint32)SDL_JOYBUTTONUP);
    eventset.push_back((Uint32)SDL_JOYHATMOTION);
    eventset.push_back((Uint32)SDL_JOYDEVICEADDED);
    eventset.push_back((Uint32)SDL_JOYDEVICEREMOVED);
}


