//
//  InputController.h
//
//  Author: High Rise Games
//
#ifndef __INPUT_CONTROLLER_H__
#define __INPUT_CONTROLLER_H__

/**
 * Device-independent input manager.
 *
 * This class currently only supports the keyboard for control.  As the
 * semester progresses, you will learn how to write an input controller
 * that functions differently on mobile than it does on the desktop.
 */
class InputController {
private:
    /** How much forward are we going? */
    float _forward;
    
    /** How much are we turning? */
    float _turning;
    
    /** Did we press the fire button? */
    bool _didFire;

    /** Did we press the reset button? */
    bool _didReset;
    
    /** The unit vector of the character moving direction*/
    cugl::Vec2 _moveDir;
    
protected:
    /** Whether the input device was successfully initialized */
    bool _active;
    
    // The screen is divided into four zones: Left, Bottom, Right and Main/
    // These are all shown in the diagram below.
    //
    //   |---------------|
    //   |   |       |   |
    //   | L |   M   | R |
    //   |   |       |   |
    //   -----------------
    //
    // The meaning of any touch depends on the zone it begins in.

    /** Information representing a single "touch" (possibly multi-finger) */
    struct TouchInstance {
        /** The anchor touch position (on start) */
        cugl::Vec2 position;
        /** The current touch time */
        cugl::Timestamp timestamp;
        /** The touch id(s) for future reference */
        std::unordered_set<Uint64> touchids;
    };

    /** Enumeration identifying a zone for the current touch */
    enum class Zone {
        /** The touch was not inside the screen bounds */
        UNDEFINED,
        /** The touch was in the left zone (as shown above) */
        LEFT,
        /** The touch was in the right zone (as shown above) */
        RIGHT,
        /** The touch was in the main zone (as shown above) */
        MAIN
    };

    /** The bounds of the entire game screen (in touch coordinates) */
    cugl::Rect _tbounds;
    /** The bounds of the entire game screen (in scene coordinates) */
    cugl::Rect _sbounds;
    /** The bounds of the left touch zone */
    cugl::Rect _lzone;
    /** The bounds of the right touch zone */
    cugl::Rect _rzone;

    // Each zone can have only one touch
    /** The current touch location for the left zone */
    TouchInstance _ltouch;
    /** The current touch location for the right zone */
    TouchInstance _rtouch;
    /** The current touch location for the bottom zone */
    TouchInstance _mtouch;
    
    /** Whether the virtual joystick is active */
    bool _joystick;
    /** The position of the virtual joystick */
    cugl::Vec2 _joycenter;

#pragma mark Input Control
public:
    /**
     * Returns the amount of forward movement.
     *
     * -1 = backward, 1 = forward, 0 = still
     *
     * @return amount of forward movement.
     */
    float getForward() const {
        return _forward;
    }

    /**
     * Returns the amount to turn the ship.
     *
     * -1 = clockwise, 1 = counter-clockwise, 0 = still
     *
     * @return amount to turn the ship.
     */
    float getTurn() const {
        return _turning;
    }
    
    /**
     * Returns the unit vector of moving direction
     *
     * @return amount to turn the ship.
     */
    cugl::Vec2 getDir() const {
        return _moveDir;
    }

    /**
     * Returns whether the fire button was pressed.
     *
     * @return whether the fire button was pressed.
     */
    bool didPressFire() const {
        return _didFire;
    }
    
    /**
     * Returns whether the reset button was pressed.
     *
     * @return whether the reset button was pressed.
     */
    bool didPressReset() const {
        return _didReset;
    }
    
    /**
     * Returns true if the virtual joystick is in use (touch only)
     *
     * @return true if the virtual joystick is in use (touch only)
     */
    bool withJoystick() const { return _joystick; }

    /**
     * Returns the scene graph position of the virtual joystick
     *
     * @return the scene graph position of the virtual joystick
     */
    cugl::Vec2 getJoystick() const { return _joycenter; }

    /**
     * Creates a new input controller with the default settings
     *
     * This is a very simple class.  It only has the default settings and never
     * needs to attach any custom listeners at initialization.  Therefore, we
     * do not need an init method.  This constructor is sufficient.
     */
    InputController();

    /**
     * Disposses this input controller, releasing all resources.
     */
    ~InputController() {}
    
    /**
     * Initializes the input control for the given bounds
     *
     * The bounds are the bounds of the scene graph.  This is necessary because
     * the bounds of the scene graph do not match the bounds of the display.
     * This allows the input device to do the proper conversion for us.
     *
     * @param bounds    the scene graph bounds
     *
     * @return true if the controller was initialized successfully
     */
    bool init(const cugl::Rect bounds);
    
    /**
     * Disposes this input controller, deactivating all listeners.
     *
     * As the listeners are deactived, the user will not be able to
     * monitor input until the controller is reinitialized with the
     * {@link #init} method.
     */
    void dispose();
    
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
    void update();
    
#pragma mark Touch Management
private:
    /**
     * Call back function for touch down
     */
    void touchDownCB(const cugl::TouchEvent& event, bool focus);
    
    /**
     * Call back function for touch released
     */
    void touchUpCB(const cugl::TouchEvent& event, bool focus);
    
    /**
     * Callback for a mouse release event.
     *
     * @param event The associated event
     * @param previous The previous position of the touch
     * @param focus    Whether the listener currently has focus
     */
    void touchesMovedCB(const cugl::TouchEvent& event, const cugl::Vec2& previous, bool focus);
    
    /**
     * Defines the zone boundaries, so we can quickly categorize touches.
     */
    void createZones();
  
    /**
     * Populates the initial values of the TouchInstances
     */
    void clearTouchInstance(TouchInstance& touchInstance);

    /**
     * Returns the correct zone for the given position.
     *
     * See the comments above for a description of how zones work.
     *
     * @param  pos  a position in screen coordinates
     *
     * @return the correct zone for the given position.
     */
    Zone getZone(const cugl::Vec2 pos) const;
    
    /**
     * Returns the scene location of a touch
     *
     * Touch coordinates are inverted, with y origin in the top-left
     * corner. This method corrects for this and scales the screen
     * coordinates down on to the scene graph size.
     *
     * @return the scene location of a touch
     */
    cugl::Vec2 touch2Screen(const cugl::Vec2 pos) const;
    
    /**
     * Processes movement for the floating joystick.
     *
     * This will register movement as left or right (or neither).  It
     * will also move the joystick anchor if the touch position moves
     * too far.
     *
     * @param  pos  the current joystick position
     */
    void processJoystick(const cugl::Vec2 pos);
};

#endif /* __INPUT_CONTROLLER_H__ */
