//
// Created by Shivank Chopra on 30/11/25.
//

#include "InputSystem.h"

#include <entt/entity/entity.hpp>

void InputSystem::_handleManualInput(const MetaData &metaData) const {
    switch (_event.key.key) {
        case SDLK_UP :
            metaData._currentCommand = Command::JUMP;
            break;
        case SDLK_LEFT:
            metaData._currentCommand = Command::MOVE_LEFT;
            break;
        case SDLK_RIGHT:
            metaData._currentCommand = Command::MOVE_RIGHT;
            break;
        default:
            metaData._currentCommand = Command::EMPTY_COMMAND;
    }
}

void InputSystem::_handleInput(const MetaData &metaData) const {
    switch (metaData._controlledBy) {
        case ControlledBy::MANUAL:
            _handleManualInput(metaData);
            break;
        // add more input support (AI, network)
        default: ;
    }
}

void InputSystem::setSdlEvent(const SDL_Event &event) {
    _event = event;
}

void InputSystem::processInput(const entt::registry &registry) {
    auto view = registry.view<MetaData>();
    for (auto [entity, metaData] : view.each()) {
        _handleInput(metaData);
    }
    _event = {}; // reset event
}
