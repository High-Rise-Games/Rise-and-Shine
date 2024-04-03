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

    // host only instantiates the all characters list, which stores char selections of all players in the lobby
    _all_characters = std::vector<std::string>(4);
    
    // part 1 of initializing list to keep track of invalid character selections
    _all_characters_select = std::vector<int>(4);
    
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
    
    std::shared_ptr<scene2::SceneNode> scene;
    
    // Acquire the scene built by the asset loader and resize it the scene
    scene = _assets->get<scene2::SceneNode>("host");
    scene->setContentSize(dimen);
    scene->doLayout(); // Repositions the HUD

    // Acquire the invalid texture to draw on the screen when player picks
    // an already selected player charatcer
    _invalid = _assets->get<scene2::SceneNode>("invalid");
    
    // initialize the list to keep track of selectable vs non-selectable characters
    _all_characters_select;
    for (int i=0; i<4; i++) {
        _all_characters_select[i] = 0;
    };
    
    
    _select_red = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_red"));
    _select_blue = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_blue"));
    _select_green = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_green"));
    _select_yellow = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_yellow"));
    _character_field_red = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("host_character_red"));
    _character_field_blue = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("host_character_blue"));
    _character_field_green = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("host_character_green"));
    _character_field_yellow = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("host_character_yellow"));

    _startgame = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_bottom_start"));
    _backout = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host_back"));
    _gameid_host = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("host_bottom_game_field_text"));
    _player_field = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("host_bottom_players_field_text"));
    _level_field = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("host_bottom_level_field_text"));
    _status = Status::WAIT;
    _id = 1;
    
    // Program the buttons
    _backout->addListener([this](const std::string& name, bool down) {
        if (down) {
            disconnect();
            _status = Status::ABORT;
            _quit = true;
        }
    });

    _startgame->addListener([this](const std::string& name, bool down) {
        if (down) {
            startGame();
        }
    });

    _select_red->addListener([this](const std::string& name, bool down) {
        if (down) {
            // Mushroom = 0
            character = "Mushroom";
            _character_field_red->setVisible(true);
            _character_field_blue->setVisible(false);
            _character_field_green->setVisible(false);
            _character_field_yellow->setVisible(false);
            
            _select_blue->setDown(false);
            _select_green->setDown(false);
            _select_yellow->setDown(false);
            AudioEngine::get()->play("click", _assets->get<cugl::Sound>("click"));
        }
        });
    _select_blue->addListener([this](const std::string& name, bool down) {
        if (down) {
            // Frog = 1
            character = "Frog";
            _character_field_red->setVisible(false);
            _character_field_blue->setVisible(true);
            _character_field_green->setVisible(false);
            _character_field_yellow->setVisible(false);

            _select_red->setDown(false);
            _select_green->setDown(false);
            _select_yellow->setDown(false);
            AudioEngine::get()->play("click", _assets->get<cugl::Sound>("click"));
        }
        });
    _select_green->addListener([this](const std::string& name, bool down) {
        if (down) {
            // Chameleon = 2
            character = "Chameleon";
            _character_field_red->setVisible(false);
            _character_field_blue->setVisible(false);
            _character_field_green->setVisible(true);
            _character_field_yellow->setVisible(false);

            _select_red->setDown(false);
            _select_blue->setDown(false);
            _select_yellow->setDown(false);
            AudioEngine::get()->play("click", _assets->get<cugl::Sound>("click"));
        }
        });
    _select_yellow->addListener([this](const std::string& name, bool down) {
        if (down) {
            // Flower = 3
            character = "Flower";
            _character_field_red->setVisible(false);
            _character_field_blue->setVisible(false);
            _character_field_green->setVisible(false);
            _character_field_yellow->setVisible(true);

            _select_red->setDown(false);
            _select_blue->setDown(false);
            _select_green->setDown(false);
            AudioEngine::get()->play("click", _assets->get<cugl::Sound>("click"));
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
    
    // Acquire the scene built by the asset loader and resize it the scene
    scene = _assets->get<scene2::SceneNode>("client");
    scene->setContentSize(dimen);
    scene->doLayout(); // Repositions the HUD

    _select_red = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_red"));
    _select_blue = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_blue"));
    _select_green = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_green"));
    _select_yellow = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_yellow"));
    _character_field_red = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("client_character_red"));
    _character_field_blue = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("client_character_blue"));
    _character_field_green = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("client_character_green"));
    _character_field_yellow = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("client_character_yellow"));

    _startgame = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_bottom_start"));
    _backout = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client_back"));
    _gameid_client = std::dynamic_pointer_cast<scene2::TextField>(_assets->get<scene2::SceneNode>("client_bottom_game_field_text"));
    _player_field = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("client_bottom_players_field_text"));
    _level_field = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("client_bottom_level_field_text"));
    _status = Status::IDLE;
    _id = 0;
    _level = -1;
    
    _backout->addListener([this](const std::string& name, bool down) {
        if (down) {
            disconnect();
            _status = Status::ABORT;
            _quit = true;
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

    _select_red->addListener([this](const std::string& name, bool down) {
        if (down) {
            character = "Mushroom";
            _character_field_red->setVisible(true);
            _character_field_blue->setVisible(false);
            _character_field_green->setVisible(false);
            _character_field_yellow->setVisible(false);
            
            _select_blue->setDown(false);
            _select_green->setDown(false);
            _select_yellow->setDown(false);
        }
        });
    _select_blue->addListener([this](const std::string& name, bool down) {
        if (down) {
            character = "Frog";
            _character_field_red->setVisible(false);
            _character_field_blue->setVisible(true);
            _character_field_green->setVisible(false);
            _character_field_yellow->setVisible(false);

            _select_red->setDown(false);
            _select_green->setDown(false);
            _select_yellow->setDown(false);
        }
        });
    _select_green->addListener([this](const std::string& name, bool down) {
        if (down) {
            character = "Chameleon";
            _character_field_red->setVisible(false);
            _character_field_blue->setVisible(false);
            _character_field_green->setVisible(true);
            _character_field_yellow->setVisible(false);

            _select_red->setDown(false);
            _select_blue->setDown(false);
            _select_yellow->setDown(false);
        }
        });
    _select_yellow->addListener([this](const std::string& name, bool down) {
        if (down) {
            character = "Flower";
            _character_field_red->setVisible(false);
            _character_field_blue->setVisible(false);
            _character_field_green->setVisible(false);
            _character_field_yellow->setVisible(true);

            _select_red->setDown(false);
            _select_blue->setDown(false);
            _select_green->setDown(false);
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
    _level_field->setText(std::to_string(_level));

    if (isHost()) {
        _all_characters[0] = character;
    }

    if (_network) {
        _network->receive([this](const std::string source,
            const std::vector<std::byte>& data) {
                processData(source, data);
                
            });
        if (!checkConnection()) {
            return;
        }
        
        configureStartButton();

        _player_field->setText(std::to_string(_network->getPeers().size() + 1));
        
        if (isHost()) {
            // sends level data across network
            const std::shared_ptr<JsonValue> json = std::make_shared<JsonValue>();
            json->init(JsonValue::Type::ObjectType);
            json->appendValue("level", std::to_string(_level));

            NetcodeSerializer netSerializer;
            netSerializer.writeJson(json);
            const std::vector<std::byte>& levelJSON = netSerializer.serialize();
            _network->broadcast(levelJSON);

            netSerializer.reset();
        }
        else if ((!isHost() && _status == WAIT) || isHost()) {
            // sends current character selection across network
            const std::shared_ptr<JsonValue> json = std::make_shared<JsonValue>();
            json->init(JsonValue::Type::ObjectType);
            json->appendValue("id", std::to_string(_id));
            json->appendValue("char", character);

            NetcodeSerializer netSerializer;
            netSerializer.writeJson(json);
            const std::vector<std::byte>& charJSON = netSerializer.serialize();
            _network->broadcast(charJSON);
            netSerializer.reset();
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
    if (isHost() && _status == START) {
        return;
    }
    if (!isHost() && data.at(0) == std::byte{ 0xff } && _status != START) {
        // read game start message sent from host
        _status = START;
        return;
    }

    NetcodeDeserializer netDeserializer;
    netDeserializer.receive(data);
    std::shared_ptr<JsonValue> jsonData = netDeserializer.readJson();

    if (jsonData->has("level") && !isHost()) {
        // read level message sent from host and update level
        _level = std::stoi(jsonData->getString("level"));
    }
    else if (jsonData->has("char") && isHost()) {
        // read character selection message sent from clients and update internal state
        std::string char_selection = jsonData->getString("char");
        int player_id = std::stoi(jsonData->getString("id"));
        if (_all_characters_select[mapToSelectList(char_selection)] == 0) {
            _all_characters[player_id - 1] = char_selection;
            _all_characters_select[mapToSelectList(char_selection)] = 1;
        } else {
            _invalid->setVisible(true);
        }
    }

    netDeserializer.reset();
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
            case cugl::net::NetcodeConnection::State::NEGOTIATING:
                _status = WAIT;
                break;
            case cugl::net::NetcodeConnection::State::CONNECTED:
                if (_status == WAIT) {
                    _status = IDLE;
                    _gameid_host->setText(hex2dec(_network->getRoom()));
                }
                break;
            case cugl::net::NetcodeConnection::State::MISMATCHED:
            case cugl::net::NetcodeConnection::State::INVALID:
            case cugl::net::NetcodeConnection::State::FAILED:
            case cugl::net::NetcodeConnection::State::DENIED:
            case cugl::net::NetcodeConnection::State::DISCONNECTED:
                // code block
                disconnect();
                _status = WAIT;
            default:
                return false;
        }
        return true;
    }
     else if (!isHost()) {
        switch(_network->getState()) {
            case cugl::net::NetcodeConnection::State::NEGOTIATING:
            // code block
                _status = JOIN;
                return true;
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
                disconnect();
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
        if (_status == IDLE && !_gameid_client->getText().empty() && !_network) {
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
            _select_red->setDown(true);
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
                connect();
            }
        }
    } else if (!isHost()) {
        if (isActive() != value) {
            Scene2::setActive(value);
            if (value) {
                _status = IDLE;
                _gameid_client->activate();
                _network = nullptr;
                _player_field->setText("1");
                _level_field->setText("1");
                configureStartButton();
                // Don't reset the room id
            }
        }

    }
}

