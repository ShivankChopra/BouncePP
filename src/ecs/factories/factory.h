//
// Created by Shivank Chopra on 31/10/25.
//

#ifndef BOUNCEPP_FACTORY_H
#define BOUNCEPP_FACTORY_H

#include <entt/entt.hpp>
#include "../components/components.hpp"

using namespace components;

constexpr int BALL_WIDTH = 25;
constexpr int BALL_HEIGHT = 25;

constexpr int PLATFORM_WIDTH = 200;
constexpr int PLATFORM_HEIGHT = 10;

class Factory {

private:
    std::vector<SDL_Texture*> _textures;

    b2WorldId _worldId;

    PhysicsData createPhysicsData(const float x, const float y, const float w, const float h, const b2WorldId worldId, const bool isDynamic = false);

    RenderingData createRenderingData(const float x, const float y, const float w, const float h);

    MetaData createMetaData(EntityType entityType, ControlledBy controlled_by);

public:
    explicit Factory(const std::vector<SDL_Texture*> &textures, const b2WorldId &worldId) : _textures(textures), _worldId(worldId) {};

    const entt::entity& createBall(entt::registry &registry, const int &x, const int &y, bool isPlayer = false);

    const entt::entity& createStep(entt::registry &registry, const int &x, const int &y);
};

#endif //BOUNCEPP_FACTORY_H