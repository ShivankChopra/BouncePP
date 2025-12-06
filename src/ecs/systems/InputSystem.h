//
// Created by Shivank Chopra on 30/11/25.
//

#ifndef BOUNCEPP_INPUTSYSTEM_H
#define BOUNCEPP_INPUTSYSTEM_H

#include <entt/entt.hpp>
#include <SDL3/SDL_events.h>

#include "../components/components.hpp"

using namespace  components;

class InputSystem {

private:
    SDL_Event _inputEvent;
    // void _handleNetworkInput();
    // void _handleAiInput();
    void _handleManualInput(const MetaData &metaData) const;
    void _handleInput(const MetaData &metaData) const; // main routing function

public:
    void setSdlEvent(const SDL_Event &event);
    void processInput(const entt::registry &registry);
};


#endif //BOUNCEPP_INPUTSYSTEM_H