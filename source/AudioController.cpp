//
//  GameAudioController.cpp
//  Shine
//
//  Created by Troy Moslemi on 3/18/24.
//

#include "AudioController.h"
using namespace cugl;


bool AudioController::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    
    
    _soundQueue = AudioEngine::get()->allocQueue();
    _assets = assets;
    _gameplayMusicIsActive = false;
    _menuMusicIsActive = false;
    _gameplayMusic = _assets->get<cugl::Sound>("high_rising");
    _menuMusic = _assets->get<cugl::Sound>("riser_riser");
    _goPress = _assets->get<cugl::Sound>("UI_go");
    _backPress = _assets->get<cugl::Sound>("UI_back");;
    _movePress = _assets->get<cugl::Sound>("UI_move");;
    _bang = _assets->get<cugl::Sound>("bang");
    _clean = _assets->get<cugl::Sound>("clean");
    
    
    return true;
}

void AudioController::playGameplayMusic() {
    
    if (!_gameplayMusicIsActive & _menuMusicIsActive || (!_menuMusicIsActive && !_gameplayIsActive) ) {
        stopMusic();
        AudioEngine::get()->getMusicQueue()->enqueue(_gameplayMusic, true);
        _gameplayMusicIsActive = true;
    }
    
};

void AudioController::playMenuMusic() {
    
    if ((!_menuMusicIsActive && !_gameplayMusicIsActive) || (!_menuMusicIsActive && _gameplayMusicIsActive)) {
        stopMusic();
        AudioEngine::get()->getMusicQueue()->enqueue(_menuMusic, true);
        _menuMusicIsActive = true;
    }
};


void AudioController::playGoPress() {
    _soundQueue->clear();
    _soundQueue->play(_goPress);
};

void AudioController::playBackPress() {
    _soundQueue->clear();
    _soundQueue->play(_backPress);
};

void AudioController::playMovePress() {
    _soundQueue->clear();
    _soundQueue->play(_movePress);
};


void AudioController::playBangSoundHost() {
    AudioEngine::get()->play("bang", _bang, false, _bang->getVolume(), true);
}

void AudioController::playCleanSoundHost() {
    AudioEngine::get()->play("clean", _clean, false, _clean->getVolume(), true);
}

void AudioController::playCleanSoundClient() {
    _soundQueue->clear();
    _cleanEffectIsActive = _soundQueue->isActive("clean");
    if (!_cleanEffectIsActive) {
//        AudioEngine::get()->play("clean", _clean, false, _clean->getVolume(), true);
        _soundQueue->play(_clean);
    }
}

void AudioController::playBangSoundClient() {
    _soundQueue->clear();
    _bangEffectIsActive = AudioEngine::get()->isActive("bang");
    if (!_bangEffectIsActive) {
//        AudioEngine::get()->play("bang", _bang, false, _bang->getVolume(), true);
        _soundQueue->play(_bang);
    }
}


void AudioController::stopMusic() {
    AudioEngine::get()->getMusicQueue()->clear();
    _menuMusicIsActive = false;
    _gameplayMusicIsActive = false;
};





