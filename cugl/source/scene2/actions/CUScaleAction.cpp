//
//  CUScaleAction.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for the scaling actions.  Scaling can specified
//  as either the final magnification or multiplicative factor.
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
//  Version: 3/12/17
//
#include <cugl/scene2/actions/CUScaleAction.h>
#include <string>
#include <sstream>

using namespace cugl;
using namespace cugl::scene2;

#pragma mark -
#pragma mark MoveBy

/**
 * Initializes a scaling animation by the given factor
 *
 * When animated, this action will adjust the scale of the node so that it
 * is multiplied by the given factor. The animation will take place over
 * the given number of seconds.
 *
 * @param factor    The amount to scale the target node
 * @param time      The animation duration
 *
 * @return true if initialization was successful.
 */
bool ScaleBy::init(const Vec2 factor, float time) {
    _delta = factor;
    _duration = time;
    return true;
}

/**
 * Returns a newly allocated copy of this Action.
 *
 * @return a newly allocated copy of this Action.
 */
std::shared_ptr<Action> ScaleBy::clone() {
    auto action = ScaleBy::alloc();
    action->setFactor(_delta);
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
void ScaleBy::start(const std::shared_ptr<SceneNode>& target, void** state) {
    // We need both scale AND anchor
    Vec2* data = (Vec2*)malloc(sizeof(Vec2)*3);
    data[0] = target->getAnchor();
    data[1] = target->getScale();
    data[2] = data[1]*_delta;
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
void ScaleBy::stop(const std::shared_ptr<SceneNode>& target, void** state) {
    Vec2* data = (Vec2*)(*state);
    target->setAnchor(data[0]);
    target->setScale(data[2]);
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
void ScaleBy::update(const std::shared_ptr<scene2::SceneNode>& target, void* state, float dt) {
    Vec2* data = (Vec2*)state;
    target->setAnchor(data[0]);
    Vec2  scale = target->getScale();
    Vec2  diff  = data[2]-data[1];
    target->setScale(scale+diff*dt);
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
std::string ScaleBy::toString(bool verbose) const {
    std::stringstream ss;
    ss << "ScaleBy{";
    ss << _delta.toString();
    ss << "}'";
    return ss.str();
}


#pragma mark -
#pragma mark MoveTo

/**
 * Initializes a scaling action towards the given scale amount
 *
 * The animation will take place over the given number of seconds.
 *
 * @param scale The target scaling amount
 * @param time  The animation duration
 *
 * @return true if initialization was successful.
 */
bool ScaleTo::init(const Vec2 scale, float time) {
    _scale = scale;
    _duration = time;
    return true;
}

/**
 * Returns a newly allocated copy of this Action.
 *
 * @return a newly allocated copy of this Action.
 */
std::shared_ptr<Action> ScaleTo::clone() {
    auto action = ScaleTo::alloc();
    action->setScale(_scale);
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
void ScaleTo::start(const std::shared_ptr<SceneNode>& target, void** state) {
    // We need both scale AND anchor
    Vec2* data = (Vec2*)malloc(sizeof(Vec2)*2);
    data[0] = target->getAnchor();
    data[1] = target->getScale();
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
void ScaleTo::stop(const std::shared_ptr<SceneNode>& target, void** state) {
    Vec2* data = (Vec2*)(*state);
    target->setAnchor(data[0]);
    target->setScale(_scale);
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
void ScaleTo::update(const std::shared_ptr<scene2::SceneNode>& target, void* state, float dt) {
    Vec2* data = (Vec2*)state;
    Vec2 scale = target->getScale();
    Vec2  diff  = _scale-data[1];
    target->setScale(scale+diff*dt);
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
std::string ScaleTo::toString(bool verbose) const {
    std::stringstream ss;
    ss << "ScaleTo{";
    ss << _scale.toString();
    ss << "}'";
    return ss.str();
}
