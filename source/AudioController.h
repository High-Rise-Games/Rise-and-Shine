//
//  AudioController.hpp
//  Shine
//
//  Created by Troy Moslemi on 3/18/24.
//

#ifndef AudioController_h
#define AudioController_h

#include <stdio.h>
#include <cugl/cugl.h>

/** The audio controller used for gameplay **/
class AudioController {
    
private:
    
    /** The global asset manager */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    /** The gameplay  music */
    std::shared_ptr<cugl::Sound> _gameplayMusic;
    
    /** The menu music */
    std::shared_ptr<cugl::Sound> _menuMusic;
    
    /** The gameplay  music */
    std::shared_ptr<cugl::Sound> _goPress;
    
    /** The gameplay  music */
    std::shared_ptr<cugl::Sound> _movePress;
    
    /** The gameplay  music */
    std::shared_ptr<cugl::Sound> _backPress;
    
    /** The window cleaning sound effect */
    std::shared_ptr<cugl::Sound> _clean;
    
    /** The bird poop collision sound effect */
    std::shared_ptr<cugl::Sound> _bang;
    
    /** Whether gameplay music is playing */
    bool _gameplayMusicIsActive;
    
    /** Whether menu music is playing */
    bool _menuMusicIsActive;
    
    /** Whether poop colllision sound effect is playing for the client */
    bool _bangEffectIsActive;
    
    /** Whether clean sound effect is playing for the host */
    bool _cleanEffectIsActive;
    
    /* Whether the gamplay controller is active */
    bool _gameplayIsActive;
    
    /** The queue for sounds so we dont abrutly stop sounds */
    std::shared_ptr<cugl::AudioQueue> _soundQueue;
    
    
public:
    
    /** Initialize the audiocontroller */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    
    /** Plays the gameplay music */
    void playGameplayMusic();
    
    /** Plays the menu music */
    void playMenuMusic();
    
    /** Plays the go sound effect when pressing a go button */
    void playGoPress();

    /** Plays the back sound effect when pressing a back button */
    void playBackPress();

    /** Plays the normal button press sound effect when pressing a normal button */
    void playMovePress();
    
    /** Plays the poop collision sound effec for the host */
    void playBangSoundHost();
    
    /** Plays the cleaning sound effect for the host */
    void playCleanSoundHost();
    
    /** Plays the poop collision sound effec for the client */
    void playBangSoundClient();
    
    /** Plays the cleaning sound effect for the client */
    void playCleanSoundClient();
    
    /** Stops all music */
    void stopMusic();
    
    /** Updates the audio controller, letting us know ot stop all music if the game is no longer active */
    void update(bool f);
    
};

#endif /* AudioController_h */
