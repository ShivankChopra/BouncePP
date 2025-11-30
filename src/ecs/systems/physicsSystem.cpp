//
// Created by Shivank Chopra on 31/10/25.
//

#include "physicsSystem.h"

using namespace components;

const b2WorldId& PhysicsSystem::getWorldId() {
    return _worldId;
}

void PhysicsSystem::updatePhysics() {
    b2World_Step(_worldId, 1.0f/60.0f, 4);
}

void PhysicsSystem::syncPhysicsWithRendering(const entt::registry &registry) {
    auto physicsRenderingView = registry.view<PhysicsData, RenderingData, MetaData>();
    for (auto [entity, physicsData, renderingData, metaData] : physicsRenderingView.each()) {
        if (metaData._entityType == EntityType::PLATFORM)
            continue;

        // only update for balls
        auto [x, y] = b2Body_GetPosition(physicsData._physicsBodyId);
        auto [rot_x, rot_y] =  b2Body_GetRotation(physicsData._physicsBodyId);
        renderingData._rotDeg = atan2(rot_y, rot_x) * 180.0 / M_PI;
        renderingData._rect.x = m_to_px(x) - renderingData._rect.w / 2.0f;
        renderingData._rect.y = m_to_px(y) - renderingData._rect.h / 2.0f;
    }
}

void PhysicsSystem::destroyWorld() {
    b2DestroyWorld(_worldId);
}
