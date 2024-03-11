//
//  CUMoveAction.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for the movement actions.  Movement can specified
//  as either the end target or the movement amount.
//
//  These classes use our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
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
//  Author: Sophie Huang and Walker White
//  Version: 12/12/22
//
#include <cugl/scene2/actions/CUMoveAction.h>
#include <string>
#include <sstream>

using namespace cugl;
using namespace cugl::scene2;

#pragma mark -
#pragma mark MoveBy

/**
 * Initializes a movement animation over the given vector.
 *
 * When animated, this action will move its target by the given delta. The
 * animation will take place over the given number of seconds.
 *
 * @param delta The amount to move the target node
 * @param time  The animation duration
 *
 * @return true if initialization was successful.
 */
bool MoveBy::init(const Vec2 delta, float time) {
    _delta = delta;
    _duration = time;
    return true;
}

/**
 * Returns a newly allocated copy of this Action.
 *
 * @return a newly allocated copy of this Action.
 */
std::shared_ptr<Action> MoveBy::clone() {
    auto action = MoveBy::alloc();
    action->setDelta(_delta);
    return action;
}

/**
 * Prepares a target for action
 *
 * The important state of the target should be allocated and stored in the given
 * state reference. The semantics of this state is action-dependent.
 *
 * @param target    The node to act on
 * @param state     The address to store the node state
 */
void MoveBy::start(const std::shared_ptr<SceneNode>& target, void** state) {
    // We need both position AND anchor
    Vec2* data = (Vec2*)malloc(sizeof(Vec2)*3);
    data[0] = target->getAnchor();
    data[1] = target->getPosition();
    data[2] = data[1];
    *state = data;
}

/**
 * Cleans up a target after an action
 *
 * The target state in the given address should be deallocated, just as it was
 * allocated in {@link #start}. The semantics of this state is action-dependent.
 *
 * @param target    The node to act on
 * @param state     The address to deallocate the node state
 */
void MoveBy::stop(const std::shared_ptr<SceneNode>& target, void** state) {
    Vec2* data = (Vec2*)(*state);
    target->setAnchor(data[0]);
    target->setPosition(data[1]+_delta);
    free(*state);
    *state = NULL;
}


/**
 * Executes an action on the given target node.
 *
 * The important state of the target is stored in the given state parameter.
 * The semantics of this state is action-dependent.
 *
 * @param target    The node to act on
 * @param state     The relevant node state
 * @param dt        The elapsed time to animate.
 */
void MoveBy::update(const std::shared_ptr<scene2::SceneNode>& target, void* state, float dt) {
    Vec2* data = (Vec2*)state;
    target->setAnchor(data[0]);
    data[2] += _delta*dt;
    target->setPosition(data[2]);
}

/**
 * Returns a string representation of the action for debugging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this action for debuggging purposes.
 */
std::string MoveBy::toString(bool verbose) const {
    std::stringstream ss;
    ss << "MoveBy{";
    ss << _delta.toString();
    ss << "}'";
    return ss.str();
}


#pragma mark -
#pragma mark MoveTo

/**
 * Initializes a movement animation towards towards the given position.
 *
 * The animation will take place over the given number of seconds.
 *
 * @param target    The target position
 * @param time      The animation duration
 *
 * @return true if initialization was successful.
 */
bool MoveTo::init(const Vec2 target, float time) {
    _target = target;
    _duration = time;
    return true;
}

/**
 * Returns a newly allocated copy of this Action.
 *
 * @return a newly allocated copy of this Action.
 */
std::shared_ptr<Action> MoveTo::clone() {
    auto action = MoveTo::alloc();
    action->setTarget(_target);
    return action;
}

/**
 * Prepares a target for action
 *
 * The important state of the target should be allocated and stored in the given
 * state reference. The semantics of this state is action-dependent.
 *
 * @param target    The node to act on
 * @param state     The address to store the node state
 */
void MoveTo::start(const std::shared_ptr<SceneNode>& target, void** state) {
    // We need both position AND anchor
    Vec2* data = (Vec2*)malloc(sizeof(Vec2)*2);
    data[0] = target->getAnchor();
    data[1] = target->getPosition();
    *state = data;
}

/**
 * Cleans up a target after an action
 *
 * The target state in the given address should be deallocated, just as it was
 * allocated in {@link #start}. The semantics of this state is action-dependent.
 *
 * @param target    The node to act on
 * @param state     The address to deallocate the node state
 */
void MoveTo::stop(const std::shared_ptr<SceneNode>& target, void** state) {
    Vec2* data = (Vec2*)(*state);
    target->setAnchor(data[0]);
    target->setPosition(_target);
    free(*state);
    *state = NULL;
}

/**
 * Executes an action on the given target node.
 *
 * The important state of the target is stored in the given state parameter.
 * The semantics of this state is action-dependent.
 *
 * @param target    The node to act on
 * @param state     The relevant node state
 * @param dt        The elapsed time to animate.
 */
void MoveTo::update(const std::shared_ptr<scene2::SceneNode>& target, void* state, float dt) {
    Vec2* data = (Vec2*)state;
    target->setAnchor(data[0]);
    Vec2  pos  = target->getPosition();
    Vec2  diff = _target-data[1];
    target->setPosition(pos+diff*dt);
}

/**
 * Returns a string representation of the action for debugging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this action for debuggging purposes.
 */
std::string MoveTo::toString(bool verbose) const {
    std::stringstream ss;
    ss << "MoveTo{";
    ss << _target.toString();
    ss << "}'";
    return ss.str();
}
