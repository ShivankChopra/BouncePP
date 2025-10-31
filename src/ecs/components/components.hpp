#pragma once

#include <box2d/box2d.h>
#include <SDL3/SDL.h>

namespace components {
    static constexpr float PPM = 50.0f; // 50 pixels = 1 meter
    inline float px_to_m(const float px) { return px / PPM; }
    inline float m_to_px(const float m) { return m * PPM; }

    enum TextureId {
        BALL_TEXTURE = 0,
        PLATFORM_TEXTURE = 1
    };

    struct PhysicsData {
        b2BodyDef _bodyDef;
        b2ShapeDef _shapeDef;
        b2Polygon _polygon;
        b2BodyId _physicsBodyId;
    };

    struct RenderingData {
        mutable SDL_FRect _rect{};
        TextureId _textureId;
        mutable double _rotDeg = 0;
    };
}