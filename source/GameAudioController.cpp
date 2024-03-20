//
//  GameAudioController.cpp
//  Shine
//
//  Created by Troy Moslemi on 3/18/24.
//

#include "GameAudioController.h"
using namespace cugl;


bool GameAudioController::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    
    _assets = assets;
    _gameplayMusic = _assets->get<cugl::Sound>("tower_of_dragons");

    return true;
}

void GameAudioController::playGameplayMusic() {

    AudioEngine::get()->play("tower_of_dragons", _gameplayMusic);
    AudioEngine::get()->getMusicQueue()->play(_gameplayMusic);
    AudioEngine::get()->getMusicQueue()->setLoop(true);
    
};

void GameAudioController::stopGameplayMusic() {
    AudioEngine::get()->getMusicQueue()->clear();;
};

void GameAudioController::update(bool gameplayIsActive) {
    if (!gameplayIsActive) {
        AudioEngine::get()->getMusicQueue()->clear();
    } else if (gameplayIsActive && !AudioEngine::get()->getMusicQueue()->isLoop()) {
        playGameplayMusic();
    };
};
