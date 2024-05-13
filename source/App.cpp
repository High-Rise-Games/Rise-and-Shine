//
//  App.cpp
//
//  Author: High Rise Games
//
#include "App.h"
using namespace cugl;

#pragma mark -
#pragma mark Gameplay Control

/**
 * The method called after OpenGL is initialized, but before running the application.
 *
 * This is the method in which all user-defined program intialization should
 * take place.  You should not create a new init() method.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to FOREGROUND,
 * causing the application to run.
 */
void App::onStartup() {
    _assets = AssetManager::alloc();
    _batch  = SpriteBatch::alloc();
    auto cam = OrthographicCamera::alloc(getDisplaySize());

    
#ifdef CU_TOUCH_SCREEN
    // Start-up basic input for loading screen (MOBILE ONLY)
    Input::activate<Touchscreen>();
#else
    // Start-up basic input (DESKTOP ONLY)
    
    Input::activate<Mouse>();
    
#endif
    Input::activate<Keyboard>();
    Input::activate<TextInput>();

    _assets->attach<Font>(FontLoader::alloc()->getHook());
    _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    _assets->attach<Sound>(SoundLoader::alloc()->getHook());
    _assets->attach<JsonValue>(JsonLoader::alloc()->getHook());
    _assets->attach<WidgetValue>(WidgetLoader::alloc()->getHook());
    _assets->attach<scene2::SceneNode>(Scene2Loader::alloc()->getHook()); // Needed for loading screen

    _scene = State::LOAD;
//    _loaded = false;
    _loading.init(_assets);
    
    
    // Queue up the other assets
    _assets->loadDirectoryAsync("json/assets.json",nullptr);
    
    _displaySettings = false;
    
    net::NetworkLayer::start(net::NetworkLayer::Log::VERBOSE);
    AudioEngine::start();
    Application::onStartup(); // YOU MUST END with call to parent
}

/**
 * The method called when the application is ready to quit.
 *
 * This is the method to dispose of all resources allocated by this
 * application.  As a rule of thumb, everything created in onStartup()
 * should be deleted here.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to NONE,
 * causing the application to be deleted.
 */
void App::onShutdown() {
    _loading.dispose();
    _gamescene.dispose();
    _lobby_host.dispose();
    _lobby_client.dispose();
    _levelscene.dispose();
    _client_join_scene.dispose();
    _assets = nullptr;
    _batch = nullptr;

    // Shutdown input
    Input::deactivate<TextInput>();
    Input::deactivate<Keyboard>();
    Input::deactivate<Mouse>();
    net::NetworkLayer::stop();
    AudioEngine::stop();
    Application::onShutdown();  // YOU MUST END with call to parent
}

/**
 * The method called when the application is suspended and put in the background.
 *
 * When this method is called, you should store any state that you do not
 * want to be lost.  There is no guarantee that an application will return
 * from the background; it may be terminated instead.
 *
 * If you are using audio, it is critical that you pause it on suspension.
 * Otherwise, the audio thread may persist while the application is in
 * the background.
 */
void App::onSuspend() {
    AudioEngine::get()->pause();
}

/**
 * The method called when the application resumes and put in the foreground.
 *
 * If you saved any state before going into the background, now is the time
 * to restore it. This guarantees that the application looks the same as
 * when it was suspended.
 *
 * If you are using audio, you should use this method to resume any audio
 * paused before app suspension.
 */
void App::onResume() {
    AudioEngine::get()->resume();
}

/**
 * The method called to update the application data.
 *
 * This is your core loop and should be replaced with your custom implementation.
 * This method should contain any code that is not an OpenGL call.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void App::update(float timestep) {
    
    
    switch (_scene) {
        case LOAD:
            updateLoadingScene(timestep);
            break;
        case MENU:
            updateMenuScene(timestep);
            break;
        case LEVEL:
            updateLevelScene(timestep);
            break;
        case CLIENT_JOIN:
            updateClientJoinScene(timestep);
            break;
        case LOBBY_CLIENT:
            updateLobbyScene(timestep);
            break;
        case LOBBY_HOST:
            updateLobbyScene(timestep);
            break;
        case GAME:
            updateGameScene(timestep);
            break;
        case TUTORIAL:
            updateTutorialScene(timestep);
            break;
    }

}

/**
 * The method called to draw the application to the screen.
 *
 * This is your core loop and should be replaced with your custom implementation.
 * This method should OpenGL and related drawing calls.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 */
void App::draw() {
    

// NEW CODE
    
    
    
    switch (_scene) {
        case LOAD:
            _loading.render(_batch);
            break;
        case MENU:
            _mainmenu.render(_batch);
            break;
        case LEVEL:
            _levelscene.render(_batch);
            break;
        case CLIENT_JOIN:
            _client_join_scene.render(_batch);
            break;
        case LOBBY_HOST:
            _lobby_host.render(_batch);
            break;
        case LOBBY_CLIENT:
            _lobby_client.render(_batch);
            break;
        case GAME:
            _gamescene.render(_batch);
            break;
        case TUTORIAL:
            _gamescene.render(_batch);
            break;
    }
    
    if (_displaySettings) {
        _settings._settingsUI->setVisible(true);
        _settings._settingsUI->render(_batch);
        CULog("Rendering Settings: %b", _displaySettings);
    }
    
}

/**
 * Inidividualized update method for the loading scene.
 *
 * This method keeps the primary {@link #update} from being a mess of switch
 * statements. It also handles the transition logic from the loading scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void App::updateLoadingScene(float timestep) {
    if (_loading.isActive()) {
        _loading.update(timestep);
    } else {
        _loading.dispose(); // Permanently disables the input listeners in this mode
        _mainmenu.init(_assets);

        // game scene, gameplay and tutorial controller
        _gamescene.init(_assets, getFPS());
        _gameplay = std::make_shared<GameplayController>();
        _gameplay->init(_assets, getFPS(), _gamescene.getBounds(), _gamescene.getSize());
        _tutorialController = std::make_shared<TutorialController>();
        _tutorialController->init(_assets, getFPS(), _gamescene.getBounds(), _gamescene.getSize());

        // level select and lobby scenes
        _levelscene.init(_assets);
        _client_join_scene.init(_assets);
        _lobby_host.init_host(_assets);
        _lobby_client.init_client(_assets);
        _settings.init(_assets);


        // audio controller
        _audioController = std::make_shared<AudioController>();
        _audioController->init(_assets);
        _tutorialController->setAudioController(_audioController);
        _gameplay->setAudioController(_audioController);
        _mainmenu.setAudioController(_audioController);
        _lobby_host.setAudioController(_audioController);
        _lobby_client.setAudioController(_audioController);
        _client_join_scene.setAudioController(_audioController);
        _levelscene.setAudioController(_audioController);
        _mainmenu.setActive(true);
        _scene = State::MENU;
    }
}

/**
 * Inidividualized update method for the menu scene.
 *
 * This method keeps the primary {@link #update} from being a mess of switch
 * statements. It also handles the transition logic from the menu scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void App::updateMenuScene(float timestep) {
    _mainmenu.update(timestep);
    std::vector<std::string> h;
    switch (_mainmenu.getChoice()) {
    case MenuScene::Choice::HOST:
        // play the click soud
        _mainmenu.setActive(false);
        _levelscene.setActive(true);
        _gamescene.setController(_gameplay);
        _scene = State::LEVEL;
        break;
    case MenuScene::Choice::JOIN:
        _mainmenu.setActive(false);
        _client_join_scene.setActive(true);
        _gamescene.setController(_gameplay);
        _scene = State::CLIENT_JOIN;
        break;
    case MenuScene::Choice::TUTORIAL:
        CULog("update menu scene to tutorial");
        _mainmenu.setActive(false);
        _gamescene.setActive(true);
        _gamescene.setController(_tutorialController);
        _tutorialController->initLevel(1);
        _tutorialController->setActive(true);
        _tutorialController->setId(1);
        _tutorialController->initHost(_assets);
        _tutorialController->setCharacters(h);
        _scene = State::TUTORIAL;
        break;
    case MenuScene::Choice::NONE:
        // DO NOTHING
        break;
    case MenuScene::Choice::SETTINGS:
        _displaySettings = true;
        _settings.cugl::Scene2::setActive(true);
        break;
    }
}

/**
 * Inidividualized update method for the level select scene.
 *
 * This method keeps the primary {@link #update} from being a mess of switch
 * statements. It also handles the transition logic from the level select scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void App::updateLevelScene(float timestep) {
    _levelscene.update(timestep);
    switch (_levelscene.getChoice()) {
        case LevelScene::Choice::NEXT:
            _levelscene.setActive(false);
            _lobby_host.setActive(true);
            _lobby_host.setHost(true);
            _lobby_host.setLevel(_levelscene.getLevel() + 1);
            _lobby_client.setActive(false);
            _scene = State::LOBBY_HOST;
            break;
        case LevelScene::Choice::BACK:
            _levelscene.setActive(false);
            _mainmenu.setActive(true);
            _scene = State::MENU;
            break;
        case LevelScene::Choice::NONE:
            break;
    }
}

/**
 * Inidividualized update method for the client join scene.
 *
 * This method keeps the primary {@link #update} from being a mess of switch
 * statements. It also handles the transition logic from the level select scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void App::updateClientJoinScene(float timestep) {
    _client_join_scene.update(timestep);
    switch (_client_join_scene.getChoice()) {
        case ClientJoinScene::Choice::NEXT:
            _client_join_scene.setActive(false);
            _lobby_client.setGameidClient(_client_join_scene.getClientID());
            _lobby_client.setActive(true);
            _lobby_client.setHost(false);
            _lobby_host.setActive(false);
            _scene = State::LOBBY_CLIENT;
            break;
        case ClientJoinScene::Choice::BACK:
            _client_join_scene.setActive(false);
            _mainmenu.setActive(true);
            _scene = State::MENU;
            break;
        case ClientJoinScene::Choice::NONE:
            break;
    }
}

/**
 * Inidividualized update method for the host scene.
 *
 * This method keeps the primary {@link #update} from being a mess of switch
 * statements. It also handles the transition logic from the host scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void App::updateLobbyScene(float timestep) {
    if (_mainmenu.getChoice() == MenuScene::HOST) {
        _lobby_host.update(timestep);

        switch (_lobby_host.getStatus()) {
            case LobbyScene::Status::ABORT:
                _lobby_host.setActive(false);
                _mainmenu.setActive(true);
                _scene = State::MENU;
                break;
            case LobbyScene::Status::START:
                _lobby_host.setActive(false);
                _gamescene.setActive(true);
                _scene = State::GAME;
                // Transfer connection ownership
                _gameplay->setConnection(_lobby_host.getNetworkController().getConnection());
                _lobby_host.getNetworkController().disconnect();
                _gameplay->setHost(true);
                _gameplay->setUUIDMap(_lobby_host.getUUIDMap());
                _gameplay->initLevel(_lobby_host.getLevel());
                _gamescene.loadBackgroundTextures();
                _gameplay->setActive(true);
                _gameplay->setId(_lobby_host.getId());
                _gameplay->initHost(_assets);
                _gameplay->setCharacters(_lobby_host.getAllCharacters());
                CULog("my id: %d", _gameplay->getId());
                break;
            case LobbyScene::Status::WAIT:
                break;
            case LobbyScene::Status::IDLE:
                // DO NOTHING
                break;
            case LobbyScene::JOIN:
                break;
        }
    } else {
        _lobby_client.update(timestep);

        switch (_lobby_client.getStatus()) {
            case LobbyScene::Status::ABORT:
                _lobby_client.setActive(false);
                _client_join_scene.setActive(true);
                _scene = State::CLIENT_JOIN;
                break;
            case LobbyScene::Status::START:
                _lobby_client.setActive(false);
                _gamescene.setActive(true);
                _scene = State::GAME;
                // Transfer connection ownership
                _gameplay->setConnection(_lobby_client.getNetworkController().getConnection());
                _lobby_client.getNetworkController().disconnect();
                _gameplay->setHost(false);
                _gameplay->initLevel(_lobby_client.getLevel());
                _gamescene.loadBackgroundTextures();
                _gameplay->setActive(true);
                _gameplay->setId(_lobby_client.getId());
                _gameplay->initClient(_assets);
                CULog("my id: %d", _gameplay->getId());
               break;
            case LobbyScene::Status::WAIT:
            case LobbyScene::Status::IDLE:
            case LobbyScene::Status::JOIN:
                // DO NOTHING
                break;
        }
    }
}

/**
 * Inidividualized update method for the game scene.
 *
 * This method keeps the primary {@link #update} from being a mess of switch
 * statements. It also handles the transition logic from the game scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void App::updateGameScene(float timestep) {
    _gamescene.update(timestep);
    if (_gamescene.didQuit() || _gameplay->isThereARequestForMenu()) {

        _gamescene.setActive(false);
        _gameplay->setActive(false);
        _gameplay->disconnect();
        _mainmenu.setActive(true);
        _scene = State::MENU;
    }
}


/**
 * Inidividualized update method for the tutorial scene.
 *
 * This method keeps the primary {@link #update} from being a mess of switch
 * statements. It also handles the transition logic from the tutorial scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void App::updateTutorialScene(float timestep) {
    _gamescene.update(timestep);
    if (_gamescene.didQuit() || _tutorialController->isThereARequestForMenu()) {
        _gamescene.setActive(false);
        _tutorialController->setActive(false);
        _mainmenu.setActive(true);
        _scene = State::MENU;
    }
}
