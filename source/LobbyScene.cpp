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
#include <mutex>
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
    
    _hostIDcounter = 1;
    
    _UUIDisProcessed = false;
    
    _numAssignedPlayers = 0;

    _chosenChars = { "", "", "", "" };
    
    setHost(true);

    // host only instantiates the all characters list, which stores char selections of all players in the lobby
    _all_characters = std::vector<std::string>(4);
        
    Size dimen = Application::get()->getDisplaySize();
    dimen *= SCENE_HEIGHT/dimen.height;
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }

    _quit = false;
    
    // Start up the input handler
    _assets = assets;
    assets->loadDirectory(assets->get<JsonValue>("lobby"));
    
    std::shared_ptr<scene2::SceneNode> scene;
    
    // Acquire the scene built by the asset loader and resize it the scene
    scene = _assets->get<scene2::SceneNode>("host");
    scene->setContentSize(dimen);
    scene->doLayout(); // Repositions the HUD

    _p1 = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("host_p1"));
    _p2 = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("host_p2"));
    _p3 = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("host_p3"));
    _p4 = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("host_p4"));
    
    _select_red = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_red"));
    _select_blue = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_blue"));
    _select_green = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_green"));
    _select_yellow = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_yellow"));
    _character_field_red = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("host_character_red"));
    _character_field_blue = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("host_character_blue"));
    _character_field_green = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("host_character_green"));
    _character_field_yellow = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("host_character_yellow"));

    _startgame = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_start"));
    _backout = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_back"));
    _gameid_host = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("host_bottom_game_field_text"));
    _player_field = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("host_bottom_players_field_text"));
    _level_field = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("host_bottom_level_field_text"));
    _status = Status::WAIT;
    _id = 1;
    
    // Program the buttons
    _backout->addListener([this](const std::string& name, bool down) {
        if (down) {
            _audioController->playBackPress();
            const std::shared_ptr<JsonValue> json = std::make_shared<JsonValue>();
            json->init(JsonValue::Type::ObjectType);
            json->appendValue("host quit", "host quit");
            _network.transmitMessage(json);
            _network.disconnect();
            _status = Status::ABORT;
            _quit = true;
        }
    });

    _startgame->addListener([this](const std::string& name, bool down) {
        if (down) {
            _audioController->playGoPress();
            startGame();
        }
    });

    _select_red->addListener([this](const std::string& name, bool down) {
        if (down) {
            _audioController->playMovePress();
            character = "Mushroom";
            _character_field_red->setVisible(true);
            _character_field_blue->setVisible(false);
            _character_field_green->setVisible(false);
            _character_field_yellow->setVisible(false);

            _select_blue->setDown(false);
            _select_green->setDown(false);
            _select_yellow->setDown(false);
        }
        else if (character == "Mushroom") {
            character = "";
            _character_field_red->setVisible(false);
        }
        });
    _select_blue->addListener([this](const std::string& name, bool down) {
        if (down) {
            _audioController->playMovePress();
            character = "Frog";
            _character_field_red->setVisible(false);
            _character_field_blue->setVisible(true);
            _character_field_green->setVisible(false);
            _character_field_yellow->setVisible(false);

            _select_red->setDown(false);
            _select_green->setDown(false);
            _select_yellow->setDown(false);
        }
        else if (character == "Frog") {
            character = "";
            _character_field_blue->setVisible(false);
        }
        });
    _select_green->addListener([this](const std::string& name, bool down) {
        if (down) {
            _audioController->playMovePress();
            character = "Chameleon";
            _character_field_red->setVisible(false);
            _character_field_blue->setVisible(false);
            _character_field_green->setVisible(true);
            _character_field_yellow->setVisible(false);

            _select_red->setDown(false);
            _select_blue->setDown(false);
            _select_yellow->setDown(false);
        }
        else if (character == "Chameleon") {
            character = "";
            _character_field_green->setVisible(false);
        }
        });
    _select_yellow->addListener([this](const std::string& name, bool down) {
        if (down) {
            _audioController->playMovePress();
            character = "Flower";
            _character_field_red->setVisible(false);
            _character_field_blue->setVisible(false);
            _character_field_green->setVisible(false);
            _character_field_yellow->setVisible(true);

            _select_red->setDown(false);
            _select_blue->setDown(false);
            _select_green->setDown(false);
        }
        else if (character == "Flower") {
            character = "";
            _character_field_yellow->setVisible(false);
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
    _quit = false;
    
    std::shared_ptr<scene2::SceneNode> scene;

    _chosenChars = { "", "", "", "" };
    
    // Acquire the scene built by the asset loader and resize it the scene
    scene = _assets->get<scene2::SceneNode>("client");
    scene->setContentSize(dimen);
    scene->doLayout(); // Repositions the HUD

    _p1 = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("client_p1"));
    _p2 = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("client_p2"));
    _p3 = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("client_p3"));
    _p4 = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("client_p4"));

    _select_red = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_red"));
    _select_blue = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_blue"));
    _select_green = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_green"));
    _select_yellow = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_yellow"));
    _character_field_red = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("client_character_red"));
    _character_field_blue = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("client_character_blue"));
    _character_field_green = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("client_character_green"));
    _character_field_yellow = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("client_character_yellow"));

    _startgame = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_start"));
    _backout = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_back"));
    _gameid_client = "";
    _clientField = std::dynamic_pointer_cast<scene2::TextField>(_assets->get<scene2::SceneNode>("client_bottom_game_field_text"));

    _player_field = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("client_bottom_players_field_text"));
    _level_field = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("client_bottom_level_field_text"));
    _status = Status::IDLE;
    _id = 0;
    _level = -1;
    
    _backout->addListener([this](const std::string& name, bool down) {
        if (down) {
            _audioController->playBackPress();
            _network.disconnect();
            _status = Status::ABORT;
            _quit = true;
        }
    });

    _select_red->addListener([this](const std::string& name, bool down) {
        if (down) {
            _audioController->playMovePress();
            character = "Mushroom";
            _character_field_red->setVisible(true);
            _character_field_blue->setVisible(false);
            _character_field_green->setVisible(false);
            _character_field_yellow->setVisible(false);
            
            _select_blue->setDown(false);
            _select_green->setDown(false);
            _select_yellow->setDown(false);
        }
        else if (character == "Mushroom") {
            character = "";
            _character_field_red->setVisible(false);
        }
        });
    _select_blue->addListener([this](const std::string& name, bool down) {
        if (down) {
            _audioController->playMovePress();
            character = "Frog";
            _character_field_red->setVisible(false);
            _character_field_blue->setVisible(true);
            _character_field_green->setVisible(false);
            _character_field_yellow->setVisible(false);

            _select_red->setDown(false);
            _select_green->setDown(false);
            _select_yellow->setDown(false);
        }
        else if (character == "Frog") {
            character = "";
            _character_field_blue->setVisible(false);
        }
        });
    _select_green->addListener([this](const std::string& name, bool down) {
        if (down) {
            _audioController->playMovePress();
            character = "Chameleon";
            _character_field_red->setVisible(false);
            _character_field_blue->setVisible(false);
            _character_field_green->setVisible(true);
            _character_field_yellow->setVisible(false);

            _select_red->setDown(false);
            _select_blue->setDown(false);
            _select_yellow->setDown(false);
        }
        else if (character == "Chameleon") {
            character = "";
            _character_field_green->setVisible(false);
        }
        });
    _select_yellow->addListener([this](const std::string& name, bool down) {
        if (down) {
            _audioController->playMovePress();
            character = "Flower";
            _character_field_red->setVisible(false);
            _character_field_blue->setVisible(false);
            _character_field_green->setVisible(false);
            _character_field_yellow->setVisible(true);

            _select_red->setDown(false);
            _select_blue->setDown(false);
            _select_green->setDown(false);
        }
        else if (character == "Flower") {
            character = "";
            _character_field_yellow->setVisible(false);
        }
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
        _network.setConnection(nullptr);
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
    _level_field->setText(std::to_string(_level));
    
    if (isHost()) {
        _all_characters[0] = character;
    }
    
    requestID();

    if (_network.getConnection()) {
        if (!checkConnection()) {
            return;
        }

        _network.getConnection()->receive([this](const std::string source,
            const std::vector<std::byte>& data) {
            std::shared_ptr<JsonValue> jsonData = _network.processMessage(source, data);
            processData(source, data);
            if (isHost() && jsonData->has("id request") && source!="") {
                _UUIDmap[jsonData->getString("id request")] = -1;
            }
            if (jsonData->has("char")) {
                _chosenChars[stoi(jsonData->getString("id"))-1] = jsonData->getString("char");
            }        
        });
        
        configureStartButton();
        
        _player_field->setText(std::to_string(_network.getNumPlayers()));
    
        if (isHost()) {
            int i = 1;
            for (auto it = _UUIDmap.begin(); it != _UUIDmap.end();) {
                auto& pair = *it;
                const auto& peer = pair.first;
                i++;
                if (_network.getConnection()->isPlayerActive(peer) && _network.getConnection()->getPeers().count(peer) > 0) {
                    const std::shared_ptr<JsonValue> json = std::make_shared<JsonValue>();
                    json->init(JsonValue::Type::ObjectType);
                    json->appendValue("level", std::to_string(_level));
                    _network.transmitMessage(peer, json);
                    ++it;
                } else {
                    it = _UUIDmap.erase(it);
                    _chosenChars[i-1] = "";
                }
            }
        }

            
        if ((!isHost() && _status == WAIT && _id != 0) || isHost()) {
            // sends current character selection across network
            if (_network.getConnection()->isOpen()) {
                const std::shared_ptr<JsonValue> json = std::make_shared<JsonValue>();
                json->init(JsonValue::Type::ObjectType);
                json->appendValue("id", std::to_string(_id));
                json->appendValue("char", character);
                _network.transmitMessage(json);
            }
        }
        
        if (_network.getConnection()->isOpen()) {
            if (isHost()) {
                int i = 1;
                for (auto it = _UUIDmap.begin(); it != _UUIDmap.end();) {
                    auto& pair = *it;
                    const auto& peer = pair.first;
                    i++;
                    if (_network.getConnection()->isPlayerActive(peer) && _network.getConnection()->getPeers().count(peer) > 0) {
                        const std::shared_ptr<JsonValue> json = std::make_shared<JsonValue>();
                        json->init(JsonValue::Type::ObjectType);
                        json->appendValue(peer, std::to_string(i));
                        _network.transmitMessage(json);
                        _UUIDmap[peer] = i;
                        ++it;
                    } else {
                        it = _UUIDmap.erase(it);
                    }
                }
            }
        }
    }

    _select_blue->activate();
    _select_yellow->activate();
    _select_green->activate();
    _select_red->activate();
    _p1->setVisible(false);
    _p2->setVisible(false);
    _p3->setVisible(false);
    _p4->setVisible(false);
    std::vector< std::shared_ptr<cugl::scene2::SceneNode>> pVec = { _p1, _p2, _p3, _p4 };
    for (int i = 0; i < _chosenChars.size(); i++) {
        int id = i + 1;
        string chosenChar = _chosenChars[i];
        if (chosenChar == "Frog") {
            if (id != _id) _select_blue->deactivate();
            // TODO: position player id marker
            pVec[i]->setVisible(true);
            pVec[i]->setPositionX(_select_blue->getPositionX());
            pVec[i]->setPositionY(_select_blue->getPositionY());
        }
        else if (chosenChar == "Flower") {
            if (id != _id) _select_yellow->deactivate();
            pVec[i]->setVisible(true);
            pVec[i]->setPositionX(_select_yellow->getPositionX());
            pVec[i]->setPositionY(_select_yellow->getPositionY());
        }
        else if (chosenChar == "Chameleon") {
            if (id != _id) _select_green->deactivate();
            pVec[i]->setVisible(true);
            pVec[i]->setPositionX(_select_green->getPositionX());
            pVec[i]->setPositionY(_select_green->getPositionY());
        }
        else if (chosenChar == "Mushroom") {
            if (id != _id) _select_red->deactivate();
            pVec[i]->setVisible(true);
            pVec[i]->setPositionX(_select_red->getPositionX());
            pVec[i]->setPositionY(_select_red->getPositionY());
        }
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
    
    std::shared_ptr<JsonValue> jsonData = _network.processMessage(source, data);
    
    if (_network.getConnection()){
        if (isHost() && _status == START) {
            return;
        }
        if (!isHost() && jsonData->has("start") && _status != START) {
            // read game start message sent from host
            _status = START;
            return;
        }

        if (jsonData->has("level") && !isHost()) {
            // read level message sent from host and update level
            _level = std::stoi(jsonData->getString("level"));
        }

        if (jsonData->has(_network.getConnection()->getUUID()) && !isHost()) {
            _id = std::stoi(jsonData->getString(_network.getConnection()->getUUID()));
        }
    }
    
    
    if (!_network.getConnection() || jsonData->has("host quit")) {
        _status = Status::ABORT;
        _quit = true;
    }
    

   
    

//    if (jsonData->has("invalid") == (std::stoi(jsonData->getString("invalid"))) && !isHost()) {
//        // read level message sent from host and update level
//        setInvalidCharacterChoice(true);
//    }
//    
    if (jsonData->has("char") && isHost()) {
        // read character selection message sent from clients and update internal state
        std::string char_selection = jsonData->getString("char");
        int player_id = std::stoi(jsonData->getString("id"));
        _all_characters[player_id - 1] = char_selection;
    }

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
        switch(_network.getConnection()->getState()) {
            case cugl::net::NetcodeConnection::State::NEGOTIATING:
                _status = WAIT;
                break;
            case cugl::net::NetcodeConnection::State::CONNECTED:
                if (_status == WAIT) {
                    _status = IDLE;
                    _gameid_host->setText(hex2dec(_network.getConnection()->getRoom()));
                }
                break;
            case cugl::net::NetcodeConnection::State::MISMATCHED:
            case cugl::net::NetcodeConnection::State::INVALID:
            case cugl::net::NetcodeConnection::State::FAILED:
            case cugl::net::NetcodeConnection::State::DENIED:
            case cugl::net::NetcodeConnection::State::DISCONNECTED:
                // code block
                _network.disconnect();
                _status = WAIT;
            default:
                return false;
        }
        return true;
    }
     else if (!isHost()) {
        switch(_network.getConnection()->getState()) {
            case cugl::net::NetcodeConnection::State::NEGOTIATING:
            // code block
                _status = JOIN;
                return true;
            case cugl::net::NetcodeConnection::State::CONNECTED:
                if (_status != START) {
                    _status = WAIT;
                }
                return true;
            case cugl::net::NetcodeConnection::State::MISMATCHED:
            // code block
                _network.disconnect();
                _status = WAIT;
                return false;
            case cugl::net::NetcodeConnection::State::INVALID:
            case cugl::net::NetcodeConnection::State::FAILED:
            case cugl::net::NetcodeConnection::State::DENIED:
            case cugl::net::NetcodeConnection::State::DISCONNECTED:
            // code block
//                disconnect();
//                _status = IDLE;
//                return false;
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
        
        const std::shared_ptr<JsonValue> json = std::make_shared<JsonValue>();
        json->init(JsonValue::Type::ObjectType);
        json->appendValue("start", "start");
        _network.transmitMessage(json);

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
        if (_status == IDLE && !_gameid_client.empty() && !_network.getConnection()) {
            _network.connect(_gameid_client,_config);
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
    if (isActive() != value) {
        if (value) {
            _quit = false;

            _backout->activate();
            _select_red->activate();
            _select_blue->activate();
            _select_green->activate();
            _select_yellow->activate();
            _select_red->setToggle(true);
            _select_blue->setToggle(true);
            _select_green->setToggle(true);
            _select_yellow->setToggle(true);
//            _select_red->setDown(true);
        }
        else {
            _startgame->deactivate();
            _backout->deactivate();
            _select_red->deactivate();
            _select_blue->deactivate();
            _select_green->deactivate();
            _select_yellow->deactivate();
            // If any were pressed, reset them
            _startgame->setDown(false);
            _backout->setDown(false);
            _select_red->setDown(false);
            _select_blue->setDown(false);
            _select_green->setDown(false);
            _select_yellow->setDown(false);
        }
    }
    
    if (isHost()) {
        if (isActive() != value) {
            Scene2::setActive(value);
            if (value) {
                _status = WAIT;
                configureStartButton();
                _player_field->setText("1");
                _level_field->setText("1");
                _network.connect(_config);
            }
        }
    } else if (!isHost()) {
        if (isActive() != value) {
            Scene2::setActive(value);
            if (value) {
                _status = IDLE;
                _clientField->setText(_gameid_client);
                _network.setConnection(nullptr);
                _player_field->setText("1");
                _level_field->setText("1");
                configureStartButton();
                // Don't reset the room id
            }
        }

    }
}

void LobbyScene::requestID() {

    
    if (_status == WAIT && !_network.didQuit()) {
        const std::shared_ptr<JsonValue> json = std::make_shared<JsonValue>();
        json->init(JsonValue::Type::ObjectType);
        json->appendValue("id request", _network.getConnection()->getUUID());
        _network.transmitMessage(json);
        
    }

    
}

