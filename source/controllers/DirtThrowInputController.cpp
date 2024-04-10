//
//  DirtThrowInputController.cpp
//  Shine
//

#include <cugl/cugl.h>
#include "DirtThrowInputController.h"

using namespace cugl;

#pragma mark Input Control
/**
 * Creates a new input controller.
 *
 * This constructor DOES NOT attach any listeners, as we are not
 * ready to do so until the scene is created. You should call
 * the {@link #init} method to initialize the scene.
 */
DirtThrowInputController::DirtThrowInputController() :
_active(false),
_currDown(false),
_prevDown(false),
_mouseDown(false),
_mouseKey(0),
_touchDown(false),
_touchKey(0) {
}


/**
 * Initializes the control to support mouse or touch.
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
bool DirtThrowInputController::init() {
#ifdef CU_TOUCH_SCREEN
    Touchscreen* touch = Input::get<Touchscreen>();
    // ADD CODE HERE FOR EXTRA CREDIT
    if (touch) {
        _touchKey = touch->acquireKey();
        touch->addBeginListener(_touchKey, [=](const TouchEvent& event, bool focus) {
            this->touchDownCB(event, focus);
        });
        touch->addEndListener(_touchKey, [=](const TouchEvent& event, bool focus) {
            this->touchUpCB(event, focus);
        });
        touch->addMotionListener(_touchKey, [=](const TouchEvent& event, const Vec2 previous, bool focus) {
            this->touchDragCB(event, previous, focus);
        });
        _active = true;
    }
#else
    Mouse* mouse = Input::get<Mouse>();
    if (mouse) {
        mouse->setPointerAwareness(Mouse::PointerAwareness::DRAG);
        _mouseKey = mouse->acquireKey();
        mouse->addPressListener(_mouseKey,[=](const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
            this->buttonDownCB(event,clicks,focus);
        });
        mouse->addReleaseListener(_mouseKey,[=](const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
            this->buttonUpCB(event,clicks,focus);
        });
        mouse->addDragListener(_mouseKey,[=](const cugl::MouseEvent& event, const Vec2 previous, bool focus) {
            this->motionCB(event,previous,focus);
        });
        _active = true;
    }
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
void DirtThrowInputController::dispose() {
#ifdef CU_TOUCH_SCREEN
    if (_active) {
        Touchscreen* touch = Input::get<Touchscreen>();
        touch->removeBeginListener(_touchKey);
        touch->removeEndListener(_touchKey);
        touch->removeMotionListener(_touchKey);
        _active = false;
    }
#else
    if (_active) {
        Mouse* mouse = Input::get<Mouse>();
        mouse->removePressListener(_mouseKey);
        mouse->removeReleaseListener(_mouseKey);
        mouse->removeDragListener(_mouseKey);
        mouse->setPointerAwareness(Mouse::PointerAwareness::BUTTON);
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
void DirtThrowInputController::update() {
    _prevDown = _currDown;
#ifdef CU_TOUCH_SCREEN
    _currDown = _touchDown;
#else
    _currDown = _mouseDown;
#endif
    _prevPos = _currPos;
#ifdef CU_TOUCH_SCREEN
    _currPos = _touchPos;
#else
    _currPos = _mousePos;
#endif
}

#pragma mark -
#pragma mark Mouse Callbacks
/**
 * Call back to execute when a mouse button is first pressed.
 *
 * This function will record a press only if the left button is pressed.
 *
 * @param event     The event with the mouse information
 * @param clicks    The number of clicks (for double clicking)
 * @param focus     Whether this device has focus (UNUSED)
 */
void DirtThrowInputController::buttonDownCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
    // Only recognize the left mouse button
    if (!_mouseDown && event.buttons.hasLeft()) {
        _mouseDown = true;
        _mousePos = event.position;
    }
}

/**
 * Call back to execute when a mouse button is first released.
 *
 * This function will record a release for the left mouse button.
 *
 * @param event     The event with the mouse information
 * @param clicks    The number of clicks (for double clicking)
 * @param focus     Whether this device has focus (UNUSED)
 */
void DirtThrowInputController::buttonUpCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
    // Only recognize the left mouse button
    if (_mouseDown && event.buttons.hasLeft()) {
        _mouseDown = false;
    }
}

/**
 * Call back to execute when the mouse moves.
 *
 * This input controller sets the pointer awareness only to monitor a mouse
 * when it is dragged (moved with button down), not when it is moved. This
 * cuts down on spurious inputs. In addition, this method only pays attention
 * to drags initiated with the left mouse button.
 *
 * @param event     The event with the mouse information
 * @param previous  The previously reported mouse location
 * @param focus     Whether this device has focus (UNUSED)
 */
void DirtThrowInputController::motionCB(const cugl::MouseEvent& event, const Vec2 previous, bool focus) {
    if (_mouseDown) {
        _mousePos = event.position;
    }
}

#pragma mark -
#pragma mark Touch Callbacks

void DirtThrowInputController::touchDownCB(const cugl::TouchEvent& event, bool focus) {
    if (!_touchDown) {
        _touchDown = true;
        _touchID = event.touch;
        _touchPos = event.position;
    }
}

void DirtThrowInputController::touchUpCB(const cugl::TouchEvent& event, bool focus) {
    if (_touchDown && _touchID == event.touch) {
        _touchDown = false;
    }
}

void DirtThrowInputController::touchDragCB(const cugl::TouchEvent& event, const Vec2 previous, bool focus) {
    if (_touchDown && _touchID == event.touch) {
        _touchPos = event.position;
    }
}
