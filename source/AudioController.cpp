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
    _gameplayMusicIsActive = false;
    _menuMusicIsActive = false;
    _gameplayMusic = _assets->get<cugl::Sound>("high_rising");
    _menuMusic = _assets->get<cugl::Sound>("riser_riser");
    _goPress = _assets->get<cugl::Sound>("button_press");
    _backPress = _assets->get<cugl::Sound>("button_press");;
    _movePress = _assets->get<cugl::Sound>("button_press");;
    _birdPoop = _assets->get<cugl::Sound>("button_press");
    
    return true;
}

void AudioController::playGameplayMusic() {
    
    if (!_gameplayMusicIsActive & _menuMusicIsActive || (!_menuMusicIsActive && !_gameplayIsActive) ) {
        stopMusic();
        AudioEngine::get()->getMusicQueue()->play(_gameplayMusic);
        AudioEngine::get()->getMusicQueue()->setLoop(true);
        _gameplayMusicIsActive = true;
    }
    
};

void AudioController::playMenuMusic() {
    
    if ((!_menuMusicIsActive && !_gameplayMusicIsActive) || (!_menuMusicIsActive && _gameplayMusicIsActive)) {
        stopMusic();
        AudioEngine::get()->getMusicQueue()->play(_menuMusic);
        AudioEngine::get()->getMusicQueue()->setLoop(true);
        _menuMusicIsActive = true;
    }
};


void AudioController::playGoPress() {
    AudioEngine::get()->play("go", _goPress);
};

void AudioController::playBackPress() {
    AudioEngine::get()->play("back", _backPress);
};

void AudioController::playMovePress() {
    AudioEngine::get()->play("move", _movePress);
};




void AudioController::playPoopSound() {
    AudioEngine::get()->play("poop", _birdPoop);
}

void AudioController::stopMusic() {
    AudioEngine::get()->getMusicQueue()->clear();
    _menuMusicIsActive = false;
    _gameplayMusicIsActive = false;
};





