//
//  LobbyScene.h
//
//  This creates the lobby scene, which is handled differently for each player.
//  For example, if the player is the host, the game code is automatically
//  generated for them while the client has to enter the game code to join
//  the game. This is done per Walker White's advice. This scene also
//  generates the networkcontroller for each player. After the game is started,
//  the network controller is transfered to the gamescene.
//
//  Created by Troy Moslemi on 2/29/24.
//

#include "LobbyScene.h"
#include <cugl/cugl.h>
#include <iostream>
#include <sstream>
/** Regardless of logo, lock the height to this */
#define SCENE_HEIGHT  720

using namespace cugl;
using namespace cugl::net;
using namespace std;


/**
 * Converts a hexadecimal string to a decimal string
 *
 * This function assumes that the string is 4 hexadecimal characters
 * or less, and therefore it converts to a decimal string of five
 * characters or less (as is the case with the lobby server). We
 * pad the decimal string with leading 0s to bring it to 5 characters
 * exactly.
 *
 * @param hex the hexadecimal string to convert
 *
 * @return the decimal equivalent to hex
 */
static std::string hex2dec(const std::string hex) {
    Uint32 value = strtool::stou32(hex,0,16);
    std::string result = strtool::to_string(value);
    if (result.size() < 5) {
        size_t diff = 5-result.size();
        std::string alt(5,'0');
        for(size_t ii = 0; ii < result.size(); ii++) {
            alt[diff+ii] = result[ii];
        }
        result = alt;
    }
    return result;
}

/**
 * Converts a decimal string to a hexadecimal string
 *
 * This function assumes that the string is a decimal number less
 * than 65535, and therefore converts to a hexadecimal string of four
 * characters or less (as is the case with the lobby server). We
 * pad the hexadecimal string with leading 0s to bring it to four
 * characters exactly.
 *
 * @param dec the decimal string to convert
 *
 * @return the hexadecimal equivalent to dec
 */
static std::string dec2hex(const std::string dec) {
    Uint32 value = strtool::stou32(dec);
    if (value >= 655366) {
        value = 0;
    }
    return strtool::to_hexstring(value,4);
}

/**
 * Checks that the network connection is still active.
 *
 * Even if you are not sending messages all that often, you need to be calling
 * this method regularly. This method is used to determine the current state
 * of the scene.
 *
 * @return true if the network connection is still active.
 */
void LobbyScene::updateText(const std::shared_ptr<scene2::Button>& button, const std::string text) {
    auto label = std::dynamic_pointer_cast<scene2::Label>(button->getChildByName("up")->getChildByName("label"));
    label->setText(text);

}


bool LobbyScene::init_host(const std::shared_ptr<cugl::AssetManager>& assets) {
    // Initialize the scene to a locked width
    
    setHost(true);
    
    Size dimen = Application::get()->getDisplaySize();
    dimen *= SCENE_HEIGHT/dimen.height;
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    // Start up the input handler
    _assets = assets;
    
    std::shared_ptr<scene2::SceneNode> scene;
    

    
    // Acquire the scene built by the asset loader and resize it the scene
    scene = _assets->get<scene2::SceneNode>("host");
    scene->setContentSize(dimen);
    scene->doLayout(); // Repositions the HUD

    _select_red = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_center_col1_red"));
    _select_blue = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_center_col2_blue"));
    _select_green = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_center_col1_green"));
    _select_yellow = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_center_col2_yellow"));

    _startgame = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_start"));
    _backout = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_back"));
    _gameid_host = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("host_game_field_text"));
    _player = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("host_players_field_text"));
    _status = Status::WAIT;
    _id = 1;
    
    // Program the buttons
    _backout->addListener([this](const std::string& name, bool down) {
        if (down) {
            disconnect();
            _status = Status::ABORT;
        }
    });

    _startgame->addListener([this](const std::string& name, bool down) {
        if (down) {
            startGame();
        }
    });


    
    // Create the server configuration
    auto json = _assets->get<JsonValue>("server");
    _config.set(json);
    
    addChild(scene);
    setActive(false);
    return true;
    
    
}

bool LobbyScene::init_client(const std::shared_ptr<cugl::AssetManager>& assets) {
    // Initialize the scene to a locked width
    
    setHost(false);
    
    Size dimen = Application::get()->getDisplaySize();
    dimen *= SCENE_HEIGHT/dimen.height;
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    // Start up the input handler
    _assets = assets;
    
    std::shared_ptr<scene2::SceneNode> scene;
    
    // Acquire the scene built by the asset loader and resize it the scene
    scene = _assets->get<scene2::SceneNode>("client");
    scene->setContentSize(dimen);
    scene->doLayout(); // Repositions the HUD
    
    _startgame = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_center_start"));
    _backout = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_back"));
    _gameid_client = std::dynamic_pointer_cast<scene2::TextField>(_assets->get<scene2::SceneNode>("client_center_game_field_text"));
    _player = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("client_center_players_field_text"));
    _status = Status::IDLE;
    _id = 0;
    
    _backout->addListener([this](const std::string& name, bool down) {
        if (down) {
            disconnect();
            _status = Status::ABORT;
        }
    });

    _startgame->addListener([=](const std::string& name, bool down) {
        if (down) {
            // This will call the _gameid listener
            _gameid_client->releaseFocus();
        }
    });
    
    _gameid_client->addExitListener([this](const std::string& name, const std::string& value) {
        connect(value);
    });


    
    // Create the server configuration
    auto json = _assets->get<JsonValue>("server");
    _config.set(json);
    
    addChild(scene);
    setActive(false);
    return true;
    
}


/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void LobbyScene::dispose() {
    if (_active) {
        removeAllChildren();
        _network = nullptr;
        _active = false;
    }
}


/**
 * The method called to update the player's NetworkConfig.
 *
 * We need to update this method to constantly talk to the server
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void LobbyScene::update(float timestep) {
    // We have written this for you this time
    if (_network) {
        _network->receive([this](const std::string source,
                                 const std::vector<std::byte>& data) {
            processData(source,data);
        });
        checkConnection();
        
        if (isHost()) {
            configureStartButton();
        }
        
        _player->setText(std::to_string(_network->getPeers().size()+1));

    }
}

/**
 * Processes data sent over the network.
 *
 * Once connection is established, all data sent over the network consistes of
 * byte vectors. This function is a call back function to process that data.
 * Note that this function may be called *multiple times* per animation frame,
 * as the messages can come from several sources.
 *
 * @param source    The UUID of the sender
 * @param data      The data received
 */
void LobbyScene::processData(const std::string source,
                            const std::vector<std::byte>& data) {
    
    if (isHost()) {
        
    } else {
        if (data.at(0) == std::byte{0xff} && _status != START) {
            _status = START;
        }
    }
    
    
}


/**
 * FUNCTION FOR HOST ONLY
 *
 * Connects to the game server as specified in the assets file for a player with this Network
 * Config object.
 *
 * The {@link #init} method set the configuration data. This method simply uses
 * this to create a new {@Link NetworkConnection}. It also immediately calls
 * {@link #checkConnection} to determine the scene state.
 *
 * @return true if the connection was successful
 */
bool LobbyScene::connect() {
    // IMPLEMENT ME
    
    _network = cugl::net::NetcodeConnection().alloc(_config);
    _network->open();
    
    if (checkConnection()) {
        return true;
    }
    
    return false;
    
}


/**
 * FUNCTION FOR CLIENT ONLY
 *
 * Connects to the game server as specified in the assets file
 *
 * The {@link #init} method set the configuration data. This method simply uses
 * this to create a new {@Link NetworkConnection}. It also immediately calls
 * {@link #checkConnection} to determine the scene state.
 *
 * @param room  The room ID to use
 *
 * @return true if the connection was successful
 */
bool LobbyScene::connect(const std::string room) {

    _network = cugl::net::NetcodeConnection().alloc(_config, dec2hex(room));
    _network->open();
    
    if (checkConnection()) {
        return true;
    }
    
    return false;
    
    
}



/**
 * Checks that the network connection is still active for a player with this
 * NetworkConfig object.
 *
 * Even if you are not sending messages all that often, you need to be calling
 * this method regularly. This method is used to determine the current state
 * of the scene.
 *
 * @return true if the network connection is still active.
 */
bool LobbyScene::checkConnection() {
        
   
    if (isHost()) {
        switch(_network->getState()) {
            case cugl::net::NetcodeConnection::State::CONNECTED:
                if (_status == WAIT) {
                    _status = IDLE;
                    _gameid_host->setText(hex2dec(_network->getRoom()));
                } else {
                    //                _status = START;
                    
                }
                return true;
            case cugl::net::NetcodeConnection::State::MISMATCHED:
                // code block
                _status = WAIT;
                return false;
            case cugl::net::NetcodeConnection::State::INVALID:
                // code block
                _status = WAIT;
                return false;
            case cugl::net::NetcodeConnection::State::FAILED:
                // code block
                _status = WAIT;
                return false;
            case cugl::net::NetcodeConnection::State::DENIED:
                // code block
                _status = WAIT;
                return false;
            case cugl::net::NetcodeConnection::State::DISCONNECTED:
                // code block
                _status = WAIT;
                return false;
            default:
                return false;
        }
    }
     else if (!isHost()) {
        switch(_network->getState()) {
            case cugl::net::NetcodeConnection::State::CONNECTED:
                if (_status != START) {
                    _status = WAIT;
                }
                if (_id == 0) {
                    _id = _network->getPeers().size() + 1;
                }
               return true;
            case cugl::net::NetcodeConnection::State::MISMATCHED:
            // code block
                _status = WAIT;
                return false;
            case cugl::net::NetcodeConnection::State::NEGOTIATING:
            // code block
                _status = JOIN;
                return true;
            case cugl::net::NetcodeConnection::State::INVALID:
            // code block
                _status = IDLE;
                return false;
            case cugl::net::NetcodeConnection::State::FAILED:
            // code block
                _status = IDLE;
                return false;
            case cugl::net::NetcodeConnection::State::DENIED:
            // code block
                _status = IDLE;
                return false;
            case cugl::net::NetcodeConnection::State::DISCONNECTED:
            // code block
                _status = IDLE;
                return false;
          default:
            return true;
        }
    }
    
    return false;
    
}



/**
 * Starts the games (method only for host)
 *
 * This method is called once the requisite number of players have connected.
 * It locks down the room and sends a "start game" message to all other
 * players.
 */
void LobbyScene::startGame() {
    
    
    if (isHost()) {
        _status = Status::START;
        
        std::vector<std::byte> byteVec;

        // sends data indicating game has started
        byteVec.push_back(std::byte{0xff});
        _network->broadcast(byteVec);
    }
    
}

/**
 * Reconfigures the start button for this scene
 *
 * This is necessary because what the buttons do depends on the state of the
 * networking.
 */
void LobbyScene::configureStartButton() {
    
    if (isHost()) {
        if (_status == WAIT) {
            updateText(_startgame,"Waiting...");
            _startgame->deactivate();
        } else if (_status == IDLE) {
            updateText(_startgame,"Start Game");
            _startgame->activate();
        }
    } else if (!isHost()) {
        if (_gameid_client->getText().size() != 0 && !_network) {
            connect(_gameid_client->getText());
        }

        if (_status == WAIT) {
            updateText(_startgame,"Waiting...");
            _startgame->setDown(false);
        } else if (_status == JOIN) {
            _startgame->deactivate();
            updateText(_startgame, "Connecting...");
            _startgame->setDown(false);
        } else if (_status == IDLE) {
            updateText(_startgame, "Start Game");
            _startgame->setDown(false);
        }
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
void LobbyScene::setActive(bool value) {
    if (isHost()) {
        if (isActive() != value) {
            Scene2::setActive(value);
            if (value) {
                _status = WAIT;
                configureStartButton();
                _backout->activate();
                connect();
            } else {
                _startgame->deactivate();
                _backout->deactivate();
                // If any were pressed, reset them
                _startgame->setDown(false);
                _backout->setDown(false);
            }
        }
    } else if (!isHost()) {
        if (isActive() != value) {
            Scene2::setActive(value);
            if (value) {
                _status = IDLE;
                _gameid_client->activate();
                _backout->activate();
                _network = nullptr;
                _player->setText("1");
                configureStartButton();
                // Don't reset the room id
            } else {
                _gameid_client->deactivate();
                _startgame->deactivate();
                _backout->deactivate();
                // If any were pressed, reset them
                _startgame->setDown(false);
                _backout->setDown(false);
            }
        }

    }
}

