#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
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

// rectangular movable boxes
class Ball {

    SDL_FRect _rect{};
    b2BodyDef _bodyDef;
    b2ShapeDef _shapeDef;
    b2Polygon _polygon;
    b2BodyId _physicsBodyId;

    double _rotDeg;

public:
    Ball(const float x, const float y, const b2WorldId worldId) {
        const float center_x_m = px_to_m(x + BLOCK_WIDTH / 2.0f);
        const float center_y_m = px_to_m(y + BLOCK_HEIGHT / 2.0f);

        _bodyDef = b2DefaultBodyDef();
        _bodyDef.type = b2_dynamicBody;
        _bodyDef.position = {center_x_m, center_y_m};

        _physicsBodyId = b2CreateBody(worldId, &_bodyDef);

        _shapeDef = b2DefaultShapeDef();
        _shapeDef.density = 40.0f;
        _shapeDef.material.friction = 0.4f;
        _shapeDef.material.restitution = 0.8f;

        _polygon = b2MakeBox(px_to_m(BLOCK_WIDTH / 2.0f), px_to_m(BLOCK_HEIGHT / 2.0f));

        b2CreatePolygonShape(_physicsBodyId, &_shapeDef, &_polygon);

        _rect = {x, y, BLOCK_WIDTH, BLOCK_HEIGHT};
    }

    void UpdatePosition(const b2WorldId worldId) {
        auto [x, y] = b2Body_GetPosition(_physicsBodyId);
        auto [rot_x, rot_y] =  b2Body_GetRotation(_physicsBodyId);
        _rotDeg = atan2(rot_y, rot_x) * 180.0 / M_PI;
        _rect.x = m_to_px(x) - BLOCK_WIDTH / 2.0f;
        _rect.y = m_to_px(y) - BLOCK_HEIGHT / 2.0f;
    }

    void render(SDL_Renderer* renderer, SDL_Texture* texture) {
        SDL_RenderTextureRotated(renderer, texture, nullptr, &_rect, _rotDeg, nullptr, SDL_FLIP_NONE);
    }
};

// solid static platform
class MyPlatform {

    SDL_FRect _rect{};
    b2BodyDef _bodyDef;
    b2ShapeDef _shapeDef;
    b2Polygon _polygon;
    b2BodyId _physicsBodyId;

public:
    MyPlatform(const float x, const float y, const b2WorldId worldId) {
        const float center_x_m = px_to_m(x + PLATFORM_WIDTH / 2.0f);
        const float center_y_m = px_to_m(y + PLATFORM_HEIGHT / 2.0f);

        _bodyDef = b2DefaultBodyDef();
        _bodyDef.position = {center_x_m, center_y_m};

        _physicsBodyId = b2CreateBody(worldId, &_bodyDef);

        _shapeDef = b2DefaultShapeDef();
        _shapeDef.density = 0.0f;

        _polygon = b2MakeBox(px_to_m(PLATFORM_WIDTH / 2.0f), px_to_m(PLATFORM_HEIGHT / 2.0f));

        b2CreatePolygonShape(_physicsBodyId, &_shapeDef, &_polygon);

        _rect = {x, y, PLATFORM_WIDTH, PLATFORM_HEIGHT};
    }

    void render(SDL_Renderer* renderer, SDL_Texture* texture) {
        SDL_RenderTexture(renderer, texture, nullptr, &_rect);
    }
};

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

    // physics world
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2(0.0, 10.98f);
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    // prepare block texture
    std::string ballImgPath = "/Users/shivankchopra/CLionProjects/Bounce++/resources/ball_image.png"; // todo Fixed relative paths
    SDL_Texture* boxTexture = LoadTextureFromFile(renderer, ballImgPath.c_str());

    // prepare platform texture
    SDL_Texture* platformTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_UNKNOWN, SDL_TEXTUREACCESS_TARGET, PLATFORM_WIDTH, PLATFORM_HEIGHT);
    SDL_Texture* prev = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, platformTexture);
    SDL_SetRenderDrawColor(renderer, 166, 44, 8, SDL_ALPHA_OPAQUE); // dark brown
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, prev);

    // blocks
    constexpr int NUM_BALLS = 10;
    std::vector<std::unique_ptr<Ball>> balls;
    for (int i = 0; i < NUM_BALLS; ++i) {
        balls.push_back(std::make_unique<Ball>(100 + i * (50 + 10), 100, worldId));
    }

    // solid platform
    constexpr int NUM_STEPS = 12;
    std::vector<std::unique_ptr<MyPlatform>> steps;
    for (int i = 0; i < NUM_STEPS; ++i) {
        steps.push_back(std::make_unique<MyPlatform>(10 + i * (100 + 10), 300 + i * (30 + 10), worldId));
    }

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

        // display blocks
        for (const std::unique_ptr<Ball>& block : balls) {
            block->UpdatePosition(worldId);
            block->render(renderer, boxTexture);
        }

        // display platform steps
        for (const std::unique_ptr<MyPlatform>& step : steps) {
            step->render(renderer, platformTexture);
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(16); // A small delay (approx. 60 FPS)
    }

    b2DestroyWorld(worldId);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
