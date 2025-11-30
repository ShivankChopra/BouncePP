//
// Created by Shivank Chopra on 30/11/25.
//

#ifndef BOUNCEPP_GAMELOGIC_H
#define BOUNCEPP_GAMELOGIC_H

#include<entt/entt.hpp>

#include "../components/components.hpp"

using namespace components;

class GameLogic {

private:
    void jump(const PhysicsData &physicsData) const;
    void moveLeft(const PhysicsData &physicsData) const;
    void moveRight(const PhysicsData &physicsData) const;

public:
    void applyActions(const entt::registry &registry) const;
};


#endif //BOUNCEPP_GAMELOGIC_H