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
    
    net::NetworkLayer::start(net::NetworkLayer::Log::INFO);
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
    
// OLD CODE IN CASE THINGS BREAK
//    if (!_loaded && _loading.isActive()) {
//        _loading.update(0.01f);
//    } else if (!_loaded) {
//        _loading.dispose(); // Disables the input listeners in this mode
//        _gameplay.init(_assets);
//        _loaded = true;
//    } else {
//        _gameplay.update(timestep);
//    }
    

// NEW CODE
    
    switch (_scene) {
        case LOAD:
            updateLoadingScene(timestep);
            break;
        case MENU:
            updateMenuScene(timestep);
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
    
// OLD CODE IN CASE THINGS BREAK
//    if (!_loaded) {
//        _loading.render(_batch);
//    } else {
//        _gameplay.render(_batch);
//    }
    
    
// NEW CODE
    
    switch (_scene) {
        case LOAD:
            _loading.render(_batch);
            break;
        case MENU:
            _mainmenu.render(_batch);
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
        _lobby_host.init_host(_assets);
        _lobby_client.init_client(_assets);
        _gamescene.init(_assets, getFPS());
        _gameplay = std::make_shared<GameplayController>();
        _gameplay->init(_assets, getFPS(), _gamescene.getBounds(), _gamescene.getSize());
        _gamescene.setController(_gameplay);
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
    switch (_mainmenu.getChoice()) {
        case MenuScene::Choice::HOST:
            _mainmenu.setActive(false);
            _lobby_host.setActive(true);
            _lobby_host.setHost(true);
            _lobby_client.setActive(false);
            _scene = State::LOBBY_HOST;
            break;
        case MenuScene::Choice::JOIN:
            _mainmenu.setActive(false);
            _lobby_client.setActive(true);
            _lobby_client.setHost(false);
            _lobby_host.setActive(false);
            _scene = State::LOBBY_CLIENT;
            break;
        case MenuScene::Choice::NONE:
            // DO NOTHING
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
                _gameplay->setConnection(_lobby_host.getConnection());
                _lobby_host.disconnect();
                _gameplay->setHost(true);
                _gameplay->setId(_lobby_host.getId());
                _gameplay->initHost(_assets);
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
                _mainmenu.setActive(true);
                _scene = State::MENU;
                break;
            case LobbyScene::Status::START:
                _lobby_client.setActive(false);
                _gamescene.setActive(true);
                _scene = State::GAME;
                // Transfer connection ownership
                _gameplay->setConnection(_lobby_client.getConnection());
                _lobby_client.disconnect();
                _gameplay->setHost(false);
                _gameplay->setId(_lobby_client.getId());
                _gameplay->initPlayers(_assets);
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
    if (_gamescene.didQuit()) {
        _gamescene.setActive(false);
        _mainmenu.setActive(true);
        _gameplay->disconnect();
        _scene = State::MENU;
    }
}


