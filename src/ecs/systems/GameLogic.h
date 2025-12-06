//
// Created by Shivank Chopra on 30/11/25.
//

#ifndef BOUNCEPP_GAMELOGIC_H
#define BOUNCEPP_GAMELOGIC_H

#include<box2d/box2d.h>
#include<entt/entt.hpp>

#include "../components/components.hpp"

using namespace components;

class GameLogic {

private:
    bool allowJump = false;

    // collision logic functions
    void processPlatformCollision(bool isBeginCollision);
    void processBeginCollision(const MetaData &metaA, const MetaData &metaB);
    void processEndCollision(const MetaData &metaA, const MetaData &metaB);

    // action functions
    void jump(const PhysicsData &physicsData) const;
    void moveLeft(const PhysicsData &physicsData) const;
    void moveRight(const PhysicsData &physicsData) const;

public:
    void applyInputActions(const entt::registry &registry) const;
    void checkPhysicsEvents(const b2WorldId &worldId, const entt::registry &registry);
};


#endif //BOUNCEPP_GAMELOGIC_H