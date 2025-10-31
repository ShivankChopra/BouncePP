#include <filesystem>
#include <iostream>
#include <string>
#include <entt/entt.hpp>
#include <box2d/box2d.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb/stb_image.h"

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

constexpr int BLOCK_WIDTH = 25;
constexpr int BLOCK_HEIGHT = 25;

constexpr int PLATFORM_WIDTH = 200;
constexpr int PLATFORM_HEIGHT = 10;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static constexpr float PPM = 50.0f; // 50 pixels = 1 meter
inline float px_to_m(const float px) { return px / PPM; }
inline float m_to_px(const float m) { return m * PPM; }

// load texture from file
SDL_Texture* LoadTextureFromFile(SDL_Renderer *renderer, const char *filename)
{
    int w, h, channels;

    // Force 4 channels (RGBA8). This is simplest and well-supported.
    unsigned char *pixels = stbi_load(filename, &w, &h, &channels, STBI_rgb_alpha);
    if (!pixels) {
        SDL_Log("stb_image failed to load %s: %s", filename, stbi_failure_reason());
        return NULL;
    }

    // Describe the pixel format to SDL.
    // STBI_rgb_alpha gives us 4 bytes per pixel: R,G,B,A in 8 bits each.
    const Uint32 pitch = (Uint32)(w * 4);  // bytes per row

    SDL_Surface *surface = SDL_CreateSurfaceFrom(
        w,
        h,
        SDL_PIXELFORMAT_RGBA32, // a.k.a. 32-bit RGBA8
        pixels,
        pitch
    );

    if (!surface) {
        SDL_Log("SDL_CreateSurfaceFrom failed: %s", SDL_GetError());
        stbi_image_free(pixels);
        return NULL;
    }

    // Upload to GPU as texture
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_Log("SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
        SDL_DestroySurface(surface);
        stbi_image_free(pixels);
        return NULL;
    }

    // Cleanup CPU-side stuff:
    SDL_DestroySurface(surface); // this does NOT free 'pixels' because we created the surface *from* external memory
    stbi_image_free(pixels);     // so we still free it manually

    return texture;
}

struct PhysicsData {
    b2BodyDef _bodyDef;
    b2ShapeDef _shapeDef;
    b2Polygon _polygon;
    b2BodyId _physicsBodyId;
};

enum TextureId {
    BALL_TEXTURE = 0,
    PLATFORM_TEXTURE = 1
};

struct RenderingData {
    SDL_FRect _rect{};
    TextureId _textureId;
    double _rotDeg = 0;
};

PhysicsData createPhysicsData(const float x, const float y, const float w, const float h, const b2WorldId worldId, const bool isDynamic = false) {
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

RenderingData createRenderingData(const float x, const float y, const float w, const float h, const TextureId tid) {
    auto data = RenderingData();
    data._rect = { x, y, w, h };
    data._textureId = tid;
    return data;
}

int main(int argc, char* argv[])
{
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Physics + rendering demo!",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED
    );

    if (window == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Log("SDL3 Window created successfully yayyy!");

    // ECS registery
    entt::registry registry;

    // physics world
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2(0.0, 10.98f);
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    // prepare block texture
    std::string ballImgPath = "/Users/shivankchopra/CLionProjects/Bounce++/resources/ball_image.png"; // todo Fixed relative paths
    SDL_Texture* ballTexture = LoadTextureFromFile(renderer, ballImgPath.c_str());

    // prepare platform texture
    SDL_Texture* platformTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_UNKNOWN, SDL_TEXTUREACCESS_TARGET, PLATFORM_WIDTH, PLATFORM_HEIGHT);
    SDL_Texture* prev = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, platformTexture);
    SDL_SetRenderDrawColor(renderer, 166, 44, 8, SDL_ALPHA_OPAQUE); // dark brown
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, prev);

    // track textures
    SDL_Texture* textures[2] = { ballTexture, platformTexture };

    // create balls
    constexpr int NUM_BALLS = 10;
    for (int i = 0; i < NUM_BALLS; i++) {
        int x = 100 + i * (50 + 10);
        int y = 100;
        const auto ball = registry.create();
        registry.emplace<PhysicsData>(ball, createPhysicsData(x, y, BLOCK_WIDTH, BLOCK_HEIGHT, worldId, true));
        registry.emplace<RenderingData>(ball, createRenderingData(x, y, BLOCK_WIDTH, BLOCK_HEIGHT, TextureId::BALL_TEXTURE));
    }

    // create platforms
    constexpr int NUM_STEPS = 12;
    for (int i = 0; i < NUM_STEPS; i++) {
        int x = 10 + i * (100 + 10);
        int y = 300 + i * (30 + 10);
        const auto step = registry.create();
        registry.emplace<PhysicsData>(step, createPhysicsData(x, y, PLATFORM_WIDTH, PLATFORM_HEIGHT, worldId));
        registry.emplace<RenderingData>(step, createRenderingData(x, y, PLATFORM_WIDTH, PLATFORM_HEIGHT, PLATFORM_TEXTURE));
    }

    // Game loop
    SDL_Event event;
    bool running = true;

    while (running)
    {
        // update physics world
        b2World_Step(worldId, 1.0f/60.0f, 4);

        // process events
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (event.key.key == SDLK_ESCAPE)
                {
                    running = false;
                }
            }
        }

        // clear before rendering
        SDL_SetRenderDrawColor(renderer, 119, 182, 246, SDL_ALPHA_OPAQUE); // powder blue (light blue)
        SDL_RenderClear(renderer);

        // apply physics updates
        auto physicsRenderingView = registry.view<PhysicsData, RenderingData>();
        for (auto [entity, physicsData, renderingData] : physicsRenderingView.each()) {
            if (renderingData._textureId == TextureId::PLATFORM_TEXTURE)
                continue;

            // only update for balls
            auto [x, y] = b2Body_GetPosition(physicsData._physicsBodyId);
            auto [rot_x, rot_y] =  b2Body_GetRotation(physicsData._physicsBodyId);
            renderingData._rotDeg = atan2(rot_y, rot_x) * 180.0 / M_PI;
            renderingData._rect.x = m_to_px(x) - renderingData._rect.w / 2.0f;
            renderingData._rect.y = m_to_px(y) - renderingData._rect.h / 2.0f;
        }

        // render components
        auto renderView = registry.view<RenderingData>();
        for (auto [entity, renderData] : renderView.each()) {
            if (renderData._textureId == TextureId::BALL_TEXTURE) {
                SDL_RenderTextureRotated(renderer, textures[TextureId::BALL_TEXTURE], nullptr, &renderData._rect, renderData._rotDeg, nullptr, SDL_FLIP_NONE);
            } else {
                SDL_RenderTexture(renderer, textures[renderData._textureId], nullptr, &renderData._rect);
            }
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(16); // A small delay (approx. 60 FPS)
    }

    b2DestroyWorld(worldId);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}