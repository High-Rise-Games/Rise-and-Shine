//
//  CUGameController.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for generic game controllers. Despite the name,
//  this classes are an interface built on top of SDL's Joystick interface, and
//  not its more robust GameController interface. That is because this is still
//  an experimental input device and we need a few more iterations to get this
//  one right.
//
//  This input devices are singletons and should never be allocated directly.
//  They should only be accessed via the Input dispatcher.
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
//  Version: 1/16/24
//
#ifndef __CU_GAME_CONTROLLER_H__
#define __CU_GAME_CONTROLLER_H__

#include <cugl/input/CUInput.h>
#include <cugl/math/CUVec2.h>

namespace cugl {

// Forward declarations
class GameControllerAxisEvent;
class GameControllerBallEvent;
class GameControllerHatEvent;
class GameControllerButtonEvent;

#pragma mark -
#pragma mark GameController
/**
 * This class is a reference to single game controller.
 *
 * A game controller is a collection of four types of input devices:
 *
 *  - Axes: Analogue joysticks and triggers
 *  - Balls: Track balls, which provide relative movement
 *  - Hats: Direction pads
 *  - Buttons: On/off input
 *
 * While individual game controllers (like XBox controllers or PS5 controllers)
 * have a very specific layout, this is a general class that makes no
 * assumptions about the layout of the individual controls. Instead, each
 * attached input device is referred to by an index.
 *
 * With that said, the index number an attached device is fixed for all
 * controllers of the same type. For example, and XBox controller uses axes
 * 0 (left-right) and 1 (up-down) for the left analogue stick and axes 2
 * (left-right) and 3 (up-down) for the right analogue stick. With some
 * experimentation, you should be able to figure the layout for any popular
 * controller.
 *
 * There should only be one instance of a specific controller at any given
 * time. In addition, controllers can be connected and removed while the
 * application is running. For that reason, this class does not allow you
 * to allocate an game controller object. Instead, you must access each
 * game controller through the {@link GameControllerInput} interface.
 */
class GameController {
public:
    /**
     * An enumeration of the hat positions.
     *
     * A hat is a directional pad. It has nine states -- the center and the
     * eight cardinal directions.
     */
    enum class Hat : int {
        /** A hat at rest */
        CENTERED = 0,
        /** A hat pressed upwards to the left */
        LEFT_UP = 1,
        /** A hat pressed entirely upwards */
        UP = 2,
        /** A hat pressed upwards to the right */
        RIGHT_UP = 3,
        /** A hat pressed entirely to the right */
        RIGHT = 4,
        /** A hat pressed downwards to the right */
        RIGHT_DOWN = 5,
        /** A hat pressed entirely downwards */
        DOWN = 6,
        /** A hat pressed downwards to the left */
        LEFT_DOWN = 7,
        /** A hat pressed upwards to the left */
        LEFT = 8
    };

#pragma mark Listener Types
    /**
     * @typedef AxisListener
     *
     * This type represents an axis listener for the {@link GameController} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * An event is delivered whenever an axis changes its position. See the
     * method {@link GameController#getAxisPosition} for more information.
     *
     * Listeners are guaranteed to be called at the start of an animation frame,
     * before the method {@link Application#update(float) }.
     *
     * While game controller listeners do not traditionally require focus like
     * a keyboard does, we have included that functionality. While only one
     * listener can have focus at a time, all listeners will receive input from
     * the game controller.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const GameControllerAxisEvent event, bool focus)>
     *
     * @param event     The axis event
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const GameControllerAxisEvent& event, bool focus)> AxisListener;

    /**
     * @typedef BallListener
     *
     * This type represents a track ball listener for the {@link GameController} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * An event is delivered whenever a track ball moves. See the method
     * {@link GameController#getBallOffset} for more information.
     *
     * Listeners are guaranteed to be called at the start of an animation frame,
     * before the method {@link Application#update(float) }.
     *
     * While game controller listeners do not traditionally require focus like
     * a keyboard does, we have included that functionality. While only one
     * listener can have focus at a time, all listeners will receive input from
     * the game controller.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const GameControllerBallEvent event, bool focus)>
     *
     * @param event     The track ball event
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const GameControllerBallEvent& event, bool focus)> BallListener;

    /**
     * @typedef HatListener
     *
     * This type represents a hat listener for the {@link GameController} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * A hat event is delivered whenever a hat changes position. See the method
     * {@link GameController#getHatPosition} for more information.
     *
     * Listeners are guaranteed to be called at the start of an animation frame,
     * before the method {@link Application#update(float) }.
     *
     * While game controller listeners do not traditionally require focus like
     * a keyboard does, we have included that functionality. While only one
     * listener can have focus at a time, all listeners will receive input from
     * the game controller.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const GameControllerHatEvent event, bool focus)>
     *
     * @param event     The hat event
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const GameControllerHatEvent& event, bool focus)> HatListener;

    /**
     * @typedef ButtonListener
     *
     * This type represents a button listener for the {@link GameController} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * A button event is delivered whenever a buttons changes state between up
     * and/or down. See the methods {@link GameController#isButtonPressed} and
     * {@link GameController#isButtonReleased} for more information.
     *
     * Listeners are guaranteed to be called at the start of an animation frame,
     * before the method {@link Application#update(float) }.
     *
     * While game controller listeners do not traditionally require focus like
     * a keyboard does, we have included that functionality. While only one
     * listener can have focus at a time, all listeners will receive input from
     * the game controller.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const GameControllerButtonEvent event, bool focus)>
     *
     * @param event     The button event
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const GameControllerButtonEvent& event, bool focus)> ButtonListener;
    
protected:
    /** The SDL joystick for the accelerometer */
    SDL_Joystick* _input;
    /** The joystick UID assigned by GameControllerInput */
    std::string _uid;
    /** The game controller description */
    std::string _name;
    
    /** Whether an axis changed state this animation frame */
    std::vector<bool> _axisState;
    /** The number of track balls */
    Uint32 _ballCount;
    /** Whether a hat changed state this animation frame */
    std::vector<bool> _hatState;
    /** Whether a button changed state this animation frame */
    std::vector<bool> _buttonState;
    
    /** The listener with focus */
   	Uint32 _focus;
   	 
    /** The set of listeners called on axis movement */
    std::unordered_map<Uint32, AxisListener> _axisListeners;
    /** The set of listeners called on track ball movement */
    std::unordered_map<Uint32, BallListener> _ballListeners;
    /** The set of listeners called on hat movement */
    std::unordered_map<Uint32, HatListener> _hatListeners;
    /** The set of listeners called on button state changes */
    std::unordered_map<Uint32, ButtonListener> _buttonListeners;

#pragma mark Constructors
public:
    /**
     * Creates a degnerate game controller
     *
     * This game controller is not actually attached to any devices. To
     * activate a game controller, use {@link GameControllerInput#open}
     * instead.
     */
    GameController();
    
    /**
     * Deletes this input device, disposing of all resources
     */
    ~GameController() { dispose(); }

private:
    /**
     * Initializes this device, acquiring any necessary resources
     *
     * @param index The device index
     * @param uid   The device uid (assigned by {@link GameControllerInput})
     *
     * @return true if initialization was successful
     */
    bool init(int index, const std::string uid);
    
    /**
     * Unintializes this device, returning it to its default state
     *
     * An uninitialized device may not work without reinitialization.
     */
    void dispose();
    
#pragma mark Input Methods
	/**
     * Cleans up a game controller after it has been disconnected.
     *
     * This method is similar to {@link dispose} except that it is aware that
     * SDL has closed the game controller already.
     */
    void forceClose();
    
    /**
     * Clears the state of this input device, readying it for the next frame.
     *
     * Many devices keep track of what happened "this" frame.  This method is
     * necessary to advance the frame.
     */
    void clearState();
    
    /**
     * Returns the internal SDL identifier for this game controller
     *
     * @return the internal SDL identifier for this game controller
     */
    SDL_JoystickID getJoystickID() const;
    
    /**
     * Records that an {@link GameControllerAxisEvent} has occured
     *
     * @param axis  The axis index
     * @param value The axis value in [-1,1]
     * @param stamp The event timestamp
     */
    void reportAxis(Uint8 axis, float value, const Timestamp& stamp);

    /**
     * Records that an {@link GameControllerBallEvent} has occured
     *
     * @param ball  The track ball index
     * @param xrel  The movement along the x-axis
     * @param yrel  The movement along the y-axis
     * @param stamp The event timestamp
     */
    void reportBall(Uint8 ball, float xrel, float yrel, const Timestamp& stamp);

    /**
     * Records that an {@link GameControllerHatEvent} has occured
     *
     * @param hat   The hat index
     * @param value The hat position
     * @param stamp The event timestamp
     */
    void reportHat(Uint8 hat, Uint8 value, const Timestamp& stamp);
    
    /**
     * Records that an {@link GameControllerButtonEvent} has occured
     *
     * @param button    The button index
     * @param down      Whether the button is down
     * @param stamp     The event timestamp
     */
    void reportButton(Uint8 button, bool down, const Timestamp& stamp);
    
    
    friend class GameControllerInput;

public:
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
    void close();

#pragma mark Attributes
    /**
     * Returns the name of this game controller.
     *
     * Names are descriptions provided by the vendor. They are not unique, and
     * it is possible to have multiple devices with the same name connected at
     * once.
     *
     * @return the name of this game controller.
     */
    std::string getName() const { return _name; }

    /**
     * Returns the UID of this game controller.
     *
     * UIDs are unique identifiers assigned by {@link GameControllerInput}.
     *
     * @return the UID of this game controller.
     */
    std::string getUID() const { return _uid; }
    
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
    Uint8 numberAxes() const;

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
    Uint8 numberBalls() const;

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
    Uint8 numberHats() const;

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
    Uint8 numberButtons() const;

#pragma mark Haptics
    /**
     * Returns true if this controller supports general rumble effects
     *
     * @return true if this controller supports general rumble effects
     */
    bool hasRumble() const;

    /**
     * Returns true if this controller supports trigger rumble effects
     *
     * @return true if this controller supports trigger rumble effects
     */
    bool hasRumbleTriggers() const;
    
    /**
     * Starts a rumble effect for this controller.
     *
     * @param low_freq  The intensity of the low frequency (left) rumble motor
     * @param high_freq The intensity of the high frequency (right) rumble motor
     * @param duration  The rumble duration in milliseconds
     */
    void applyRumble(Uint16 low_freq, Uint16 high_freq, Uint32 duration);
    
    /**
     * Starts a rumble effect for this controller.
     *
     * @param left      The intensity of the left trigger rumble motor
     * @param right     The intensity of the right trigger rumble motor
     * @param duration  The rumble duration in milliseconds
     */
    void hasRumbleTriggers(Uint16 left, Uint16 right, Uint32 duration) const;

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
    float getAxisPosition(Uint8 axis) const;
    
    /**
     * Returns true if the given axis changed position this frame.
     *
     * @param axis  The axis index
     *
     * @return true if the given axis changed position this frame.
     */
    bool axisDidChange(Uint8 axis) const;
    
    /**
     * Returns true if the given axis is a trigger.
     *
     * Triggers are axes whose default value is -1 (so they are one-sided).
     *
     * @param axis  The axis index
     *
     * @return true if the given axis is a trigger.
     */
    bool axisIsTrigger(Uint8 axis) const;

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
    Vec2 getBallOffset(Uint8 ball) const;
        
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
    Hat getHatPosition(Uint8 hat) const;
    
    /**
     * Returns true if the given hat changed position this frame.
     *
     * @param hat   The hat index
     *
     * @return true if the given hat changed position this frame.
     */
    bool hatDidChange(Uint8 hat) const;
    
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
    bool isButtonDown(Uint8 button) const;

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
    bool isButtonPressed(Uint8 button) const;
    
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
    bool isButtonReleased(Uint8 button) const;

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
    bool requestFocus(Uint32 key);
    
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
    bool isListener(Uint32 key) const;
    
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
    const AxisListener getAxisListener(Uint32 key) const;
    
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
    const BallListener getBallListener(Uint32 key) const;

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
    const HatListener getHatListener(Uint32 key) const;

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
    const ButtonListener getButtonListener(Uint32 key) const;

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
    bool addAxisListener(Uint32 key, AxisListener listener);

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
    bool addBallListener(Uint32 key, BallListener listener);

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
    bool addHatListener(Uint32 key, HatListener listener);

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
    bool addButtonListener(Uint32 key, ButtonListener listener);

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
    bool removeAxisListener(Uint32 key);

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
    bool removeBallListener(Uint32 key);

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
    bool removeHatListener(Uint32 key);

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
    bool removeButtonListener(Uint32 key);
    
};

#pragma mark -
#pragma mark Game Controller Events
/**
 * This simple class is a struct to hold information about an axis movement.
 *
 * This event is generated when an axis changes position. Remember that
 * analogue joysticks are composed of two axes, and thuse will have two
 * events associated with them.
 */
class GameControllerAxisEvent {
public:
    /** The time of the input event */
    Timestamp timestamp;
    /** The UID of the relevant device */
    std::string uuid;
    /** The axis index */
    Uint8 index;
    /** The value in the range [-1,1] */
    float value;
    
    /**
     * Constructs a new axis event with the default values
     */
    GameControllerAxisEvent() : index(0), value(0) {}
    
    /**
     * Constructs a new axis event with the given values
     *
     * @param key   The device UID
     * @param ind   The axis index
     * @param pos   The axis position in [-1,1]
     * @param stamp The timestamp for the event
     */
    GameControllerAxisEvent(const std::string key, Uint8 ind, float pos,
                            const Timestamp& stamp) {
        uuid = key; index = ind; value = pos; timestamp = stamp;
    }
    
};

/**
 * This simple class is a struct to hold information about track ball movement
 *
 * All track ball movement is relative. Unlike the other input devices there is
 * no absolute state for a track ball. As such this even will be continuously
 * generated as long as a track ball is moving, and will not fire when it is
 * still.
 */
class GameControllerBallEvent {
public:
    /** The time of the input event */
    Timestamp timestamp;
    /** The UID of the relevant device */
    std::string uuid;
    /** The track ball index */
    Uint8 index;
    /** The relative change in the track ball position */
    Vec2 offset;
    
    /**
     * Constructs a new track ball event with the default values
     */
    GameControllerBallEvent() : index(0) {}
    
    /**
     * Constructs a new track ball event with the given values
     *
     * @param key   The device UID
     * @param ind   The track ball index
     * @param dx    The change in x position
     * @param dy    The change in y position
     * @param stamp The timestamp for the event
     */
    GameControllerBallEvent(const std::string key, Uint8 ind, float dx, float dy,
                             const Timestamp& stamp) {
        uuid = key; index = ind;
        offset.set(dx,dy); timestamp = stamp;
    }
    
};


/**
 * This simple class is a struct to hold information about hat movement
 *
 * A hat is a directional pad with 9 different states. This event will fire
 * only when this state changes.
 */
class GameControllerHatEvent {
public:
    /** The time of the input event */
    Timestamp timestamp;
    /** The UID of the relevant device */
    std::string uuid;
    /** The hat index */
    Uint8 index;
    /** The new hat position */
    GameController::Hat state;
    
    /**
     * Constructs a new acceleration event with the default values
     */
    GameControllerHatEvent() : index(0), state(GameController::Hat::CENTERED) {}
    
    /**
     * Constructs a new hat event with the given values
     *
     * @param key   The device UID
     * @param ind   The hat index
     * @param pos   The hat position (in SDL encoding)
     * @param stamp The timestamp for the event
     */
    GameControllerHatEvent(const std::string key, Uint8 ind, Uint8 pos,
                           const Timestamp& stamp);
    
};

/**
 * This simple class is a struct to hold information about button presses
 *
 * A button has only two states: up and down. This event will fire only when
 * this state changes.
 */
class GameControllerButtonEvent {
public:
    /** The time of the input event */
    Timestamp timestamp;
    /** The UID of the relevant device */
    std::string uuid;
    /** The button index */
    Uint8 index;
    /** Whether the button event is from a press (not a release) */
    bool down;
    
    /**
     * Constructs a new button event with the default values
     */
    GameControllerButtonEvent() : index(0), down(false) {}
    
    /**
     * Constructs a new button event with the given values
     *
     * @param key   The device UID
     * @param ind   The button index
     * @param dwn   Whether the button is pressed
     * @param stamp The timestamp for the event
     */
    GameControllerButtonEvent(const std::string key, Uint8 ind, bool dwn,
                             const Timestamp& stamp) {
        uuid = key; index = ind; down = dwn; timestamp = stamp;
    }
};

    
#pragma mark -
#pragma mark GameControllerInputEvent

/**
 * This simple class is a struct to hold information about a device change.
 *
 * This event is generated when new devices are added to the device list, or
 * when an existing device is removed.
 */
class GameControllerInputEvent {
public:
    /** The time of the device event */
    Timestamp timestamp;
    /** The UID of the relevant device */
    std::string uuid;
    /** Whether this device is newly added (false means it was removed) */
    bool added;
    
    /**
     * Constructs a new device change event with the default values
     */
    GameControllerInputEvent() { }
    
    /**
     * Constructs a new device change event with the given values
     *
     * @param key   The device UID
     * @param add   Whether the device was added (not removed)
     * @param stamp The timestamp for the event
     */
    GameControllerInputEvent(const std::string key, bool add,
                             const Timestamp& stamp) {
        uuid = key; added = add; timestamp = stamp;
    }
    
};

#pragma mark -
#pragma mark GameControllerInput
/**
 * This class is an input manager for a collection of game controllers
 *
 * While it is possible to have more than one game controller attached at any
 * time, SDL broadcasts all controller state changes. Therefore, it is useful
 * to have a central hub that manages controllers and dispatches events to the
 * appropriate controller. In addition, this particular input device monitors
 * when controllers are added and removed.
 *
 * Game controllers only receive events when they are activated.  See the
 * methods {@link GameControllerInput#open} and {@link GameControllerInput#close}
 * for how to activate and deactivate controllers.
 */
class GameControllerInput : public InputDevice {
public:
    /**
     * @typedef Listener
     *
     * This type represents a listener for the {@link GameControllerInput} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * An event is delivered whenever a new game controller is either added to
     * or removed from the list of devices. This can happen when a device loses
     * power, or it connected during a play session.
     *
     * Listeners are guaranteed to be called at the start of an animation frame,
     * before the method {@link Application#update(float) }.
     *
     * While game controller input listeners do not traditionally require focus
     * like a keyboard does, we have included that functionality. While only one
     * listener can have focus at a time, all listeners will receive input from
     * the Game Controller input device.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const GameControllerInputEvent event, bool focus)>
     *
     * @param event     The game controller input event
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const GameControllerInputEvent event,
                               bool focus)> Listener;

private:
    /** The list of all devices connected (identified by UIDs) */
    std::unordered_map<std::string, int> _devices;
    /** The descriptive names for these devices */
    std::unordered_map<std::string, std::string> _names;
    /** A map from the SDL identifiers to our UIDs */
    std::unordered_map<SDL_JoystickID, std::string> _joyids;
    /** The active game controllers, identified by SDL id */
    std::unordered_map<SDL_JoystickID, std::shared_ptr<GameController>> _bysdl;
    /** The active game controllers, identified by UID */
    std::unordered_map<std::string, std::shared_ptr<GameController>> _byname;

    /** The set of listeners called whenever we update the device list */
    std::unordered_map<Uint32, Listener> _listeners;
    
    /** Whether to filter out non-gamepad joysticks */
    bool _filter;

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
    std::string addDevice(int index);
    
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
    std::string removeDevice(SDL_JoystickID jid);

    /**
     * Scans all connected game controllers, adding them to this manager.
     *
     * This method depends on {@link #doesFilter} for what it considers a
     * valid game controller.
     */
    void scanDevices();
    
#pragma mark Constructors
    /**
     * Creates and initializes a new game controller manager.
     *
     * WARNING: Never allocate a game controller manager directly. Always
     * use the {@link Input#activate()} method instead.
     */
    GameControllerInput() : _filter(true) { }
    
    /**
     * Deletes this input device, disposing of all resources
     */
    virtual ~GameControllerInput() { dispose(); }
    
    /**
     * Initializes this device, acquiring any necessary resources
     *
     * @return true if initialization was successful
     */
    bool init();
    
    /**
     * Unintializes this device, returning it to its default state
     *
     * An uninitialized device may not work without reinitialization.
     */
    virtual void dispose() override;

#pragma mark Attributes
public:
    /**
     * Returns true if this game controller manager filters its devices
     *
     * Our game controller manager is an interface built on top of the SDL
     * joystick functions. However, SDL has a very broad definition of
     * joystick, and uses it to include things like an accelerometer. If
     * this value is true (which is the default), then only devices which
     * match traditional game controllers will be listed.
     *
     * @return true if this game controller manager filters its devices
     */
    bool doesFilter() const { return _filter; }
    
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
    void filter(bool value);
    
    /**
     * Returns the number of connected devices.
     *
     * This value will be affected by {@link #doesFilter}.
     *
     * @return the number of connected devices.
     */
    size_t size() const;
    
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
	std::vector<std::string> devices() const;

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
    std::string getName(const std::string uid);

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
    std::shared_ptr<GameController> open(const std::string uid);
    
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
    std::shared_ptr<GameController> get(const std::string uid);
    
    /**
     * Closes the game controller for the given UID
     *
     * This invalidates all references to the game controller, making them
     * no longer usable. The only way to access the game controller again is
     * to call {@link #open}.
     *
     * @param uid   The device UID
     */
    void close(const std::string uid);

#pragma mark Listener Methods
    /**
     * Requests focus for the given identifier
     *
     * Only a listener can have focus. This method returns false if key
     * does not refer to an active listener
     *
     * @param key   The identifier for the focus object
     *
     * @return false if key does not refer to an active listener
     */
    virtual bool requestFocus(Uint32 key) override;
    
    /**
     * Returns true if key represents a listener object
     *
     * @param key   The identifier for the listener
     *
     * @return true if key represents a listener object
     */
    bool isListener(Uint32 key) const;
    
    /**
     * Returns the game controller manager listener for the given object key
     *
     * If there is no listener for the given key, it returns nullptr.
     *
     * @param key   The identifier for the listener
     *
     * @return the game controller manager listener for the given object key
     */
    const Listener getListener(Uint32 key) const;
    
    /**
     * Adds an game controller manager listener for the given object key
     *
     * There can only be one listener for a given key. If there is already
     * a listener for the key, the method will fail and return false. You
     * must remove a listener before adding a new one for the same key.
     *
     * @param key       The identifier for the listener
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addListener(Uint32 key, Listener listener);
    
    /**
     * Removes the game controller manager listener for the given object key
     *
     * If there is no active listener for the given key, this method fails and
     * returns false.
     *
     * @param key   The identifier for the listener
     *
     * @return true if the listener was succesfully removed
     */
    bool removeListener(Uint32 key);
    
#pragma mark Input Device Methods
    /**
     * Clears the state of this input device, readying it for the next frame.
     *
     * Many devices keep track of what happened "this" frame.  This method is
     * necessary to advance the frame.
     */
    virtual void clearState() override;
    
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
    virtual bool updateState(const SDL_Event& event, const Timestamp& stamp) override;
    
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
    virtual void queryEvents(std::vector<Uint32>& eventset) override;
    
    // Apparently friends are not inherited
    friend class Input;
};

}

#endif /* __CU_GAME_CONTROLLER_H__ */
