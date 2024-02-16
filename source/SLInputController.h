//
//  GLInputController.h
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
#ifndef __SL_INPUT_CONTROLLER_H__
#define __SL_INPUT_CONTROLLER_H__

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
     * Reads the input for this player and converts the result into game logic.
     *
     * This is an example of polling input.  Instead of registering a listener,
     * we ask the controller about its current state.  When the game is running,
     * it is typically best to poll input instead of using listeners.  Listeners
     * are more appropriate for menus and buttons (like the loading screen).
     */
    void readInput();
};

#endif /* __GL_INPUT_CONTROLLER_H__ */
