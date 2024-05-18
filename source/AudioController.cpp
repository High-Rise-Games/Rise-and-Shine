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
    _allowMusic = true;
    _allowSounds = true;
    
    
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
    if (!_allowSounds) {
        AudioEngine::get()->play("bang", _bang, false, _bang->getVolume(), true);
    } else {
        AudioEngine::get()->play("bang", _bang, false, 0, true);
    }
    
}

void AudioController::playCleanSoundHost() {
    if (!_allowSounds) {
        AudioEngine::get()->play("clean", _clean, false, 0, true);
    } else {
        AudioEngine::get()->play("clean", _clean, false, _clean->getVolume(), true);
    }
    
}

void AudioController::playCleanSoundClient() {
    _soundQueue->clear();
//    _cleanEffectIsActive = _soundQueue->isActive("clean");
    if (!_cleanEffectIsActive) {
//        AudioEngine::get()->play("clean", _clean, false, _clean->getVolume(), true);
        if (!_allowSounds) {
            _soundQueue->play(_clean, false, 0, false);
        } else {
            _soundQueue->play(_clean, false, _clean->getVolume(), false);
        }
        
    }
}

void AudioController::playBangSoundClient() {
    _soundQueue->clear();
    _bangEffectIsActive = AudioEngine::get()->isActive("bang");
    if (!_bangEffectIsActive) {
//        AudioEngine::get()->play("bang", _bang, false, _bang->getVolume(), true);
        if (!_allowSounds) {
            _soundQueue->play(_bang, false, 0, false);
        } else {
            _soundQueue->play(_bang, false, _bang->getVolume(), false);
        }
    }
}


void AudioController::stopMusic() {
    AudioEngine::get()->getMusicQueue()->clear();
    _menuMusicIsActive = false;
    _gameplayMusicIsActive = false;
};

void AudioController::allowMusic() {
    if (_allowMusic) {
        AudioEngine::get()->getMusicQueue()->setVolume(0);
        _allowMusic = false;
    } else {
        AudioEngine::get()->getMusicQueue()->setVolume(0.25);
        _allowMusic=true;
    }
};

void AudioController::allowSounds() {
    if (_allowSounds) {
        _allowSounds=false;
    } else {
        _allowSounds = true;
    }
};


