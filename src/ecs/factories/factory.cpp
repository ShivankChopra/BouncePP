//
// Created by Shivank Chopra on 31/10/25.
//

#include "factory.h"

// private

PhysicsData Factory::createPhysicsData(const float x, const float y, const float w, const float h, const b2WorldId worldId, const entt::entity entity, const bool isDynamic) {
    auto data = PhysicsData();

    const float center_x_m = px_to_m(x + w / 2.0f);
    const float center_y_m = px_to_m(y + h / 2.0f);

    data._bodyDef = b2DefaultBodyDef();

    if (isDynamic) {
        data._bodyDef.type = b2_dynamicBody;
    }

    data._bodyDef.position = { center_x_m, center_y_m };
    data._bodyDef.userData = reinterpret_cast<void*>(entity);
    data._physicsBodyId = b2CreateBody(worldId, &data._bodyDef);

    data._shapeDef = b2DefaultShapeDef();

    if (isDynamic) {
        data._shapeDef.density = 40.0f;
        data._shapeDef.material.friction = 1.2f;
        data._shapeDef.material.restitution = 0.5f;
        data._shapeDef.enableContactEvents = true;
    } else {
        data._shapeDef.density = 0.0f;
    }

    data._polygon = b2MakeBox(px_to_m(w / 2.0f), px_to_m(h / 2.0f));

    b2CreatePolygonShape(data._physicsBodyId, &data._shapeDef, &data._polygon);

    return data;
}

RenderingData Factory::createRenderingData(const float x, const float y, const float w, const float h) {
    auto data = RenderingData();
    data._rect = { x, y, w, h };
    return data;
}

MetaData Factory::createMetaData(EntityType entityType, ControlledBy controlled_by) {
    auto data = MetaData();
    data._entityType = entityType;
    data._controlledBy = controlled_by;
    return data;
}

// public

const entt::entity& Factory::createBall(entt::registry &registry, const int &x, const int &y) {
    const auto ball = registry.create();
    registry.emplace<PhysicsData>(ball, createPhysicsData(x, y, BALL_WIDTH, BALL_HEIGHT, _worldId, ball, true));
    registry.emplace<RenderingData>(ball, createRenderingData(x, y, BALL_WIDTH, BALL_HEIGHT));

    ControlledBy controlledBy = ControlledBy::MANUAL;
    registry.emplace<MetaData>(ball, createMetaData(EntityType::BALL, controlledBy));

    return ball;
}

const entt::entity& Factory::createStep(entt::registry &registry, const int &x, const int &y) {
    const auto step = registry.create();
    registry.emplace<PhysicsData>(step, createPhysicsData(x, y, PLATFORM_WIDTH, PLATFORM_HEIGHT, _worldId, step));
    registry.emplace<RenderingData>(step, createRenderingData(x, y, PLATFORM_WIDTH, PLATFORM_HEIGHT));
    registry.emplace<MetaData>(step, createMetaData(EntityType::PLATFORM, ControlledBy::NOT_CONTROLLED));
    return step;
}