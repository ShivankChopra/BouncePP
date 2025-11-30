#ifndef BOUNCEPP_PHYSICS_H
#define BOUNCEPP_PHYSICS_H

#include <box2d/box2d.h>
#include <entt/entt.hpp>

#include "../components/components.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class PhysicsSystem {

private:
    b2WorldId _worldId;
    b2WorldDef _worldDef;

public:
    explicit PhysicsSystem(const float &gravity) {
        _worldDef = b2DefaultWorldDef();
        _worldDef.gravity = b2Vec2(0.0, gravity);
        _worldId = b2CreateWorld(&_worldDef);
    }

    const b2WorldId& getWorldId();

    void updatePhysics();

    void syncPhysicsWithRendering(const entt::registry &registry);

    void destroyWorld();
};


#endif //BOUNCEPP_PHYSICS_H