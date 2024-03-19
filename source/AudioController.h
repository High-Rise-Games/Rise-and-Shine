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


class AudioController {
    
private:
    
    /** The global asset manager */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    /** The background music */
    std::shared_ptr<cugl::Sound> _backgroundMusic;

    
    /** Whether menu msuic is playing */
    bool _menuMusic;
    
    /** Whether gameplay music  is playing */
    bool _gampeplayMusic;
    
    /** Whether poop colllision sound effect is playing */
    bool _poopCollisionEffect;
    
    
public:
    
    /** Initialize the audiocontroller */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    
    void playBackgroundMusic();
    
    void stopBackgroundMusic();
    
    void playPoopCollisionSoundEffect();
    
};

#endif /* AudioController_hpp */
