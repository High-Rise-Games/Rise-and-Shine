//
//  AudioController.hpp
//  Shine
//
//  Created by Troy Moslemi on 3/18/24.
//

#ifndef GameAudioController_h
#define GameAudioController_h

#include <stdio.h>
#include <cugl/cugl.h>

/** The audio controller used for gameplay **/
class GameAudioController {
    
private:
    
    /** The global asset manager */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    /** The background music */
    std::shared_ptr<cugl::Sound> _gameplayMusic;
    
    /** Whether gameplay music is playing */
    bool _gampeplayMusicIsActive;
    
    /** Whether poop colllision sound effect is playing */
    bool _poopCollisionEffectIsActive;
    
    /* Whether the gamplay controller is active */
    bool _gameplayIsActive;
    
    
public:
    
    /** Initialize the audiocontroller */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    
    /** Plays the gameplay music */
    void playGameplayMusic();
    
    /** Stops the gameplay music */
    void stopGameplayMusic();
    
    /** Plays the poop collision sound effect */
    void playPoopCollisionSoundEffect();
    
    /** Sets whether the gameplay msuic is active **/
    void setGameplayMusicActive(bool f) {
        _gampeplayMusicIsActive = f;
    }
    
    /** Returns whether the gameplayer music is currently active or not **/
    bool getGameplayMusicStatus() {
        return _gampeplayMusicIsActive;
    }
    
    /** Updates the audio controller, letting us know ot stop all music if the game is no longer active */
    void update(bool gameplayIsActive);
    
};

#endif /* GameAudioController_h */
