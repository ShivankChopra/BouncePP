//
// Created by Shivank Chopra on 30/11/25.
//

#include "GameLogic.h"

void GameLogic::processPlatformCollision(bool isBeginCollision) {
    if (isBeginCollision) {
        allowJump = true;
    } else {
        allowJump = false;
    }
}

void GameLogic::processBeginCollision(const MetaData &metaA, const MetaData &metaB) {
    if ((metaA._entityType == EntityType::BALL && metaB._entityType == EntityType::PLATFORM) ||
        (metaB._entityType == EntityType::BALL && metaA._entityType == EntityType::PLATFORM)) {
        processPlatformCollision(true);
    }
}

void GameLogic::processEndCollision(const MetaData &metaA, const MetaData &metaB) {
    if ((metaA._entityType == EntityType::BALL && metaB._entityType == EntityType::PLATFORM) ||
        (metaB._entityType == EntityType::BALL && metaA._entityType == EntityType::PLATFORM)) {
        processPlatformCollision(false);
    }
}

void GameLogic::jump(const PhysicsData &physicsData) const {
    if (allowJump) {
        constexpr b2Vec2 impulse = {0.0f, -80.0f};
        b2Body_ApplyLinearImpulseToCenter(physicsData._physicsBodyId, impulse, true);
    }
}

void GameLogic::moveLeft(const PhysicsData &physicsData) const {
    constexpr b2Vec2 impulse = {-25.0f, 0.0f};
    b2Body_ApplyLinearImpulseToCenter(physicsData._physicsBodyId, impulse, true);
}

void GameLogic::moveRight(const PhysicsData &physicsData) const {
    constexpr b2Vec2 impulse = {25.0f, 0.0f};
    b2Body_ApplyLinearImpulseToCenter(physicsData._physicsBodyId, impulse, true);
}

void GameLogic::applyInputActions(const entt::registry &registry) const {
    auto view = registry.view<PhysicsData, MetaData>();
    for (auto [entity, physicsData, metaData] : view.each()) {
        switch (metaData._currentCommand) {
            case Command::JUMP:
                jump(physicsData);
                break;
            case Command::MOVE_LEFT:
                moveLeft(physicsData);
                break;
            case Command::MOVE_RIGHT:
                moveRight(physicsData);
                break;
        }

        metaData._currentCommand = Command::EMPTY_COMMAND; // reset
    }
}

void GameLogic::checkPhysicsEvents(const b2WorldId &worldId, const entt::registry &registry) {
    const b2ContactEvents contactEvents = b2World_GetContactEvents(worldId);

    // begin events
    for (int i = 0; i < contactEvents.beginCount; i++) {
        const b2ContactBeginTouchEvent& ev = contactEvents.beginEvents[i];

        const b2ShapeId shapeA = ev.shapeIdA;
        const b2ShapeId shapeB = ev.shapeIdB;

        b2BodyId bodyA = b2Shape_GetBody(shapeA);
        b2BodyId bodyB = b2Shape_GetBody(shapeB);

        entt::entity eA = static_cast<entt::entity>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyA)));
        entt::entity eB = static_cast<entt::entity>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyB)));

        auto dataA = registry.get<MetaData>(eA);
        auto dataB = registry.get<MetaData>(eB);

        processBeginCollision(dataA, dataB);
    }

    // end events
    for (int i = 0; i < contactEvents.endCount; i++) {
        const b2ContactEndTouchEvent& ev = contactEvents.endEvents[i];

        const b2ShapeId shapeA = ev.shapeIdA;
        const b2ShapeId shapeB = ev.shapeIdB;

        b2BodyId bodyA = b2Shape_GetBody(shapeA);
        b2BodyId bodyB = b2Shape_GetBody(shapeB);

        entt::entity eA = static_cast<entt::entity>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyA)));
        entt::entity eB = static_cast<entt::entity>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyB)));

        auto dataA = registry.get<MetaData>(eA);
        auto dataB = registry.get<MetaData>(eB);

        processEndCollision(dataA, dataB);
    }
}
