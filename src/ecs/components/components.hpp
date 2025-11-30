#pragma once

#include <box2d/box2d.h>
#include <SDL3/SDL.h>

namespace components {
    static constexpr float PPM = 50.0f; // 50 pixels = 1 meter
    inline float px_to_m(const float px) { return px / PPM; }
    inline float m_to_px(const float m) { return m * PPM; }

    enum EntityType {
        BALL = 0,
        PLATFORM = 1
    };

    enum ControlledBy {
        MANUAL = 0,
        AI = 1,
        NETWORK = 2,
        NOT_CONTROLLED = 3
    };

    enum Command {
        JUMP = 0,
        MOVE_LEFT = 1,
        MOVE_RIGHT = 2,
        EMPTY_COMMAND = 3
    };

    struct PhysicsData {
        b2BodyDef _bodyDef;
        b2ShapeDef _shapeDef;
        b2Polygon _polygon;
        b2BodyId _physicsBodyId;
    };

    struct RenderingData {
        mutable SDL_FRect _rect{};
        mutable double _rotDeg = 0;
    };

    struct MetaData {
        EntityType _entityType;
        mutable ControlledBy _controlledBy = ControlledBy::NOT_CONTROLLED;
        mutable Command _currentCommand = Command::EMPTY_COMMAND;
    };
}