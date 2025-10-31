//
// Created by Shivank Chopra on 31/10/25.
//

#include "factory.h"

// private

PhysicsData Factory::createPhysicsData(const float x, const float y, const float w, const float h, const b2WorldId worldId, const bool isDynamic) {
    auto data = PhysicsData();

    const float center_x_m = px_to_m(x + w / 2.0f);
    const float center_y_m = px_to_m(y + h / 2.0f);

    data._bodyDef = b2DefaultBodyDef();

    if (isDynamic) {
        data._bodyDef.type = b2_dynamicBody;
    }

    data._bodyDef.position = { center_x_m, center_y_m };
    data._physicsBodyId = b2CreateBody(worldId, &data._bodyDef);
    data._shapeDef = b2DefaultShapeDef();

    if (isDynamic) {
        data._shapeDef.density = 40.0f;
        data._shapeDef.material.friction = 0.4f;
        data._shapeDef.material.restitution = 0.8f;
    } else {
        data._shapeDef.density = 0.0f;
    }

    data._polygon = b2MakeBox(px_to_m(w / 2.0f), px_to_m(h / 2.0f));

    b2CreatePolygonShape(data._physicsBodyId, &data._shapeDef, &data._polygon);

    return data;
}

RenderingData Factory::createRenderingData(const float x, const float y, const float w, const float h, const components::TextureId tid) {
    auto data = RenderingData();
    data._rect = { x, y, w, h };
    data._textureId = tid;
    return data;
}

// public

const entt::entity& Factory::createBall(entt::registry &registry, const int &x, const int &y) {
    const auto ball = registry.create();
    registry.emplace<PhysicsData>(ball, createPhysicsData(x, y, BLOCK_WIDTH, BLOCK_HEIGHT, _worldId, true));
    registry.emplace<RenderingData>(ball, createRenderingData(x, y, BLOCK_WIDTH, BLOCK_HEIGHT, TextureId::BALL_TEXTURE));
    return ball;
}

const entt::entity& Factory::createStep(entt::registry &registry, const int &x, const int &y) {
    const auto step = registry.create();
    registry.emplace<PhysicsData>(step, createPhysicsData(x, y, PLATFORM_WIDTH, PLATFORM_HEIGHT, _worldId));
    registry.emplace<RenderingData>(step, createRenderingData(x, y, PLATFORM_WIDTH, PLATFORM_HEIGHT, PLATFORM_TEXTURE));
    return step;
}