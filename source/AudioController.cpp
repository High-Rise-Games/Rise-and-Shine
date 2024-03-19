//
//  AudioController.cpp
//  Shine
//
//  Created by Troy Moslemi on 3/18/24.
//

#include "AudioController.h"
using namespace cugl;


bool AudioController::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    
    _assets = assets;
    
    _backgroundMusic = _assets->get<cugl::Sound>("tower_of_dragons");

    return true;
}

void AudioController::playBackgroundMusic() {

    AudioEngine::get()->play("tower_of_dragons", _backgroundMusic);
    
}
