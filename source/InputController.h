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
    
    /** The position of the touch down event*/
    cugl::Vec2 _startPos;
    
    /** The distance of the last swipe*/
    cugl::Vec2 _moveDis;
    
protected:
    /** Whether the input device was successfully initialized */
    bool _active;
    /** Whether the finger is down*/
    bool _touchDown;
    /** Whether the finger is released in this animation frame*/
    bool _touchReleased;
    /** The key for the touch listeners*/
    Uint32 _touchKey;
    /** The current touch id of the finger*/
    cugl::TouchID _touchID;

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
    bool init();
    
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
    
#pragma mark Touch Callbacks
private:
    /**
     * Call back function for touch down
     */
    void touchDownCB(const cugl::TouchEvent& event, bool focus);
    
    /**
     * Call back function for touch released
     */
    void touchUpCB(const cugl::TouchEvent& event, bool focus);
};

#endif /* __INPUT_CONTROLLER_H__ */
