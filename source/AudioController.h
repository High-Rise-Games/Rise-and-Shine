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
    
    /** The bird poop sound effect */
    std::shared_ptr<cugl::Sound> _birdPoop;
    
    /** Whether gameplay music is playing */
    bool _gameplayMusicIsActive;
    
    /** Whether menu music is playing */
    bool _menuMusicIsActive;
    
    /** Whether poop colllision sound effect is playing */
    bool _poopCollisionEffectIsActive;
    
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
    
    void playGoPress();

    void playBackPress();

    void playMovePress();

    
    /** Plays the poop collision sound effect */
    void playPoopSound();
    
    /** Stops all music */
    void stopMusic();
    
    /** Updates the audio controller, letting us know ot stop all music if the game is no longer active */
    void update(bool f);
    
};

#endif /* AudioController_h */
