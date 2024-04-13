//
//  ClientJoinScene.cpp
//  Shine
//
//

#include "ClientJoinScene.h"
#include <cugl/cugl.h>
#include <iostream>
#include <sstream>

/** Regardless of logo, lock the height to this */
#define SCENE_HEIGHT  720

using namespace cugl;
using namespace std;

/**
 * Initializes the controller contents
 *
 * In previous labs, this method "started" the scene.  But in this
 * case, we only use to initialize the scene user interface.  We
 * do not activate the user interface yet, as an active user
 * interface will still receive input EVEN WHEN IT IS HIDDEN.
 *
 * That is why we have the method {@link #setActive}.
 *
 * @param assets    The (loaded) assets for this game mode
 *
 * @return true if the controller is initialized properly, false otherwise.
 */
bool ClientJoinScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    dimen *= SCENE_HEIGHT/dimen.height;
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    // Acquire the scene built by the asset loader and resize it the scene
    _assets = assets;
    
    std::shared_ptr<scene2::SceneNode> scene = assets->get<scene2::SceneNode>("client_join");
    scene->setContentSize(dimen);
    scene->doLayout(); // Repositions the HUD
    _choice = Choice::NONE;
    _selectedlevel = -1;
    _placeholderText = _assets->get<scene2::SceneNode>("client_join_inputPlaceholder");
    _clientRoomTextfield = std::dynamic_pointer_cast<scene2::TextField>(_assets->get<scene2::SceneNode>("client_join_client_id_field"));
    _backbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_join_back"));
    _nextbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_join_next"));
    _keypadbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_join_b0")));
    _keypadbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_join_b1")));
    _keypadbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_join_b2")));
    _keypadbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_join_b3")));
    _keypadbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_join_b4")));
    _keypadbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_join_b5")));
    _keypadbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_join_b6")));
    _keypadbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_join_b7")));
    _keypadbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_join_b8")));
    _keypadbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_join_b9")));
    _keypadbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_join_bc")));
    _keypadbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_join_b-1")));
    
    // Program the buttons
    _backbutton->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = BACK;
        }
    });
    _nextbutton->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = NEXT;
        }
    });
    
    for (int i = 0; i < _keypadbuttons.size(); ++i) {
        _keypadbuttons[i]->addListener([this, i](const std::string& name, bool down) {
            if (down) {
                if (i < 10) {
                    if (_clientRoomTextfield->getText().length() < 5) {
                        _clientRoomTextfield->setText(_clientRoomTextfield->getText() + std::to_string(i).c_str());
                    }
                } else if (i == 10) {
                    _clientRoomTextfield->setText("");
                } else {
                    if (_clientRoomTextfield->isVisible()) {
                        std::string txt = _clientRoomTextfield->getText();
                        txt.pop_back();
                        _clientRoomTextfield->setText(txt);
                    }
                }
                if (!_clientRoomTextfield->getText().empty()) {
                    _clientRoomTextfield->setVisible(true);
                    _placeholderText->setVisible(false);
                } else {
                    _clientRoomTextfield->setVisible(false);
                    _placeholderText->setVisible(true);
                }
            }
        });
    }
    
    _clientRoomTextfield->setText("");
    _clientRoomTextfield->setVisible(false);

    addChild(scene);
    setActive(false);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void ClientJoinScene::dispose() {
    if (_active) {
        removeAllChildren();
        _active = false;
    }
}

/**
 * Sets whether the scene is currently active
 *
 * This method should be used to toggle all the UI elements.  Buttons
 * should be activated when it is made active and deactivated when
 * it is not.
 *
 * @param value whether the scene is currently active
 */
void ClientJoinScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        if (value) {
            _choice = NONE;
            _backbutton->activate();
            _nextbutton->activate();
            for (const auto& button: _keypadbuttons) {
                button->activate();
                button->setVisible(true);
            }
            _clientRoomTextfield->setText("");
            _placeholderText->setVisible(true);
        } else {
            _backbutton->deactivate();
            _nextbutton->deactivate();
            // If any were pressed, reset them
            _backbutton->setDown(false);
            _nextbutton->setDown(false);
            for (const auto& button: _keypadbuttons) {
                button->deactivate();
                button->setDown(false);
            }
        }
    }
}
