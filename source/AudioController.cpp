//
//  GameAudioController.cpp
//  Shine
//
//  Created by Troy Moslemi on 3/18/24.
//

#include "AudioController.h"
using namespace cugl;


bool AudioController::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    
    _assets = assets;
    _gameplayMusic = _assets->get<cugl::Sound>("high_rising");
    _menuMusic = _assets->get<cugl::Sound>("riser_riser");
    _buttonPress = _assets->get<cugl::Sound>("button_press");
    _birdPoop = _assets->get<cugl::Sound>("button_press");
    
    return true;
}

void AudioController::playGameplayMusic() {
    
    if (!AudioEngine::get()->getMusicQueue()->isLoop()) {
        AudioEngine::get()->getMusicQueue()->play(_gameplayMusic);
        AudioEngine::get()->getMusicQueue()->setLoop(true);
    }
    
};

void AudioController::playMenuMusic() {
    if (!AudioEngine::get()->getMusicQueue()->isLoop()) {
        AudioEngine::get()->getMusicQueue()->play(_menuMusic);
        AudioEngine::get()->getMusicQueue()->setLoop(true);
    }
};


void AudioController::playButtonPressSound() {
    AudioEngine::get()->play("button", _buttonPress);
};


void AudioController::playPoopSound() {
    AudioEngine::get()->play("poop", _birdPoop);
}

void AudioController::stopMusic() {
    AudioEngine::get()->getMusicQueue()->clear();
};



