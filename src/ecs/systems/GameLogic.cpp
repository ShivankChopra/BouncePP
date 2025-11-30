//
// Created by Shivank Chopra on 30/11/25.
//

#include "GameLogic.h"

void GameLogic::jump(const PhysicsData &physicsData) const {
    constexpr b2Vec2 impulse = {0.0f, -50.0f};
    b2Body_ApplyLinearImpulseToCenter(physicsData._physicsBodyId, impulse, true);
}

void GameLogic::moveLeft(const PhysicsData &physicsData) const {
    constexpr b2Vec2 impulse = {-20.0f, 0.0f};
    b2Body_ApplyLinearImpulseToCenter(physicsData._physicsBodyId, impulse, true);
}

void GameLogic::moveRight(const PhysicsData &physicsData) const {
    constexpr b2Vec2 impulse = {20.0f, 0.0f};
    b2Body_ApplyLinearImpulseToCenter(physicsData._physicsBodyId, impulse, true);
}

void GameLogic::applyActions(const entt::registry &registry) const {
    auto view = registry.view<PhysicsData, MetaData>();
    for (auto [entity, physicsData, metaData] : view.each()) {
        switch (metaData._currentCommand) {
            case Command::JUMP:
                jump(physicsData);
                SDL_Log("Game logic : Current command is : JUMP");
                break;
            case Command::MOVE_LEFT:
                moveLeft(physicsData);
                SDL_Log("Game logic : Current command is : MOVE_LEFT");
                break;
            case Command::MOVE_RIGHT:
                moveRight(physicsData);
                SDL_Log("Game logic : Current command is : MOVE_RIGHT");
                break;
        }

        metaData._currentCommand = Command::EMPTY_COMMAND; // reset
    }
}
