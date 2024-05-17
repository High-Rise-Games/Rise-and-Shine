//
//  App.h
//
//  Author: Walker White
//  Version: 1/20/22
//
#ifndef __APP__
#define __APP__
#include <cugl/cugl.h>
#include "GameScene.h"
#include "GameplayController.h"
#include "LoadingScene.h"
#include "LobbyScene.h"
#include "LevelScene.h"
#include "ClientJoinScene.h"
#include "MenuScene.h"
#include "AudioController.h"
#include "TutorialController.h"
#include "SettingsScene.h"

/**
 * This class represents the application root for the ship demo.
 */
class App : public cugl::Application {
protected:
    
    /**
     * The current active scene
     */
    enum State {
        /** The loading scene */
        LOAD,
        /** The main menu scene */
        MENU,
        /** The tutorial scene */
        TUTORIAL,
        /** The level select scene*/
        LEVEL,
        /** The client joint scene*/
        CLIENT_JOIN,
        /** The scene to host or join a game */
        LOBBY_CLIENT,
        LOBBY_HOST,
        /** The scene to play the game */
        GAME,
        SETTINGS
    };
    
    /* Whether settings is displayed or not*/
    bool _displaySettings;
    

    /** The audio controller initiated by the app */
    std::shared_ptr<AudioController> _audioController;
    /** The global sprite batch for drawing (only want one of these) */
    std::shared_ptr<cugl::SpriteBatch> _batch;
    /** The global asset manager */
    std::shared_ptr<cugl::AssetManager> _assets;

    // Player modes
    
    /** The controller for the loading screen */
    LoadingScene _loading;
    /** The menu scene to choose what to do */
    MenuScene _mainmenu;
    /** The level select scene to choose the level */
    LevelScene _levelscene;
    /** The client join scene */
    ClientJoinScene _client_join_scene;
    /** The scene for hosting or joining a game */
    LobbyScene _lobby_host;
    
    /** The scene for hosting or joining a game */
    LobbyScene _lobby_client;
    
    
    /** The scene for hosting or joining a game */
    LobbyScene _lobby;
    
    /** The scene for settings */
    SettingsScene _settings;

    /** The primary controller for the tutorial world */
    std::shared_ptr<GameplayController> _tutorialController;
    
    /** The scene for the game world */
    GameScene _gamescene;
    /** The primary controller for the game world */
    std::shared_ptr<GameplayController> _gameplay;

    /** The controller for network during gameplay */
    // std::shared_ptr<NetworkController> _network;
    
    
    /** The current active scene */
    State _scene;
    

    /** Whether or not we have finished loading all assets */
    bool _loaded;
    
    // Audio Controller

    
public:
    /**
     * Creates, but does not initialized a new application.
     *
     * This constructor is called by main.cpp.  You will notice that, like
     * most of the classes in CUGL, we do not do any initialization in the
     * constructor.  That is the purpose of the init() method.  Separation
     * of initialization from the constructor allows main.cpp to perform
     * advanced configuration of the application before it starts.
     */
    App() : cugl::Application(), _loaded(false) {}
    
    /**
     * Disposes of this application, releasing all resources.
     *
     * This destructor is called by main.cpp when the application quits.
     * It simply calls the dispose() method in Application.  There is nothing
     * special to do here.
     */
    ~App() { }
    
#pragma mark Application State
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
    virtual void onStartup() override;
    
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
    virtual void onShutdown() override;
 
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
    virtual void onSuspend() override;
    
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
    virtual void onResume()  override;
    
#pragma mark Application Loop
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
    virtual void update(float timestep) override;
    
    /**
     * The method called to draw the application to the screen.
     *
     * This is your core loop and should be replaced with your custom implementation.
     * This method should OpenGL and related drawing calls.
     *
     * When overriding this method, you do not need to call the parent method
     * at all. The default implmentation does nothing.
     */
    virtual void draw() override;
    
    
private:
    /**
     * Inidividualized update method for the loading scene.
     *
     * This method keeps the primary {@link #update} from being a mess of switch
     * statements. It also handles the transition logic from the loading scene.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void updateLoadingScene(float timestep);

    /**
     * Inidividualized update method for the menu scene.
     *
     * This method keeps the primary {@link #update} from being a mess of switch
     * statements. It also handles the transition logic from the menu scene.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void updateMenuScene(float timestep);
    
    /**
     * Inidividualized update method for the level select scene.
     *
     * This method keeps the primary {@link #update} from being a mess of switch
     * statements. It also handles the transition logic from the level select scene.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void updateLevelScene(float timestep);
    
    /**
     * Inidividualized update method for the client join scene.
     *
     * This method keeps the primary {@link #update} from being a mess of switch
     * statements. It also handles the transition logic from the level select scene.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void updateClientJoinScene(float timestep);

    /**
     * Inidividualized update method for the lobby scene.
     *
     * This method keeps the primary {@link #update} from being a mess of switch
     * statements. It also handles the transition logic from the host scene.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void updateLobbyScene(float timestep);

    /**
     * Inidividualized update method for the game scene.
     *
     * This method keeps the primary {@link #update} from being a mess of switch
     * statements. It also handles the transition logic from the game scene.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void updateGameScene(float timestep);

    /**
     * Inidividualized update method for the tutorial scene.
     *
     * This method keeps the primary {@link #update} from being a mess of switch
     * statements. It also handles the transition logic from the tutorial scene.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void updateTutorialScene(float timestep);
    
};

#endif /* __APP__ */
