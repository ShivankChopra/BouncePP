#include <filesystem>
#include <iostream>
#include <string>
#include <entt/entt.hpp>
#include <box2d/box2d.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define STB_IMAGE_IMPLEMENTATION
#include "ecs/factories/factory.h"
#include "ecs/systems/physicsSystem.h"
#include "vendor/stb/stb_image.h"

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

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

    // ECS registery
    entt::registry registry;

    // physics
    auto ps = new PhysicsSystem(10.98f);

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
    const std::vector<SDL_Texture*> textures = { ballTexture, platformTexture };

    // factory
    auto factory = new Factory(textures, ps->getWorldId());

    // create balls
    constexpr int NUM_BALLS = 5;
    for (int i = 0; i < NUM_BALLS; i++) {
        int x = 500 + i * (BALL_WIDTH + 10);
        int y = 400;
        factory->createBall(registry, x, y);
    }

    // create platforms
    constexpr int NUM_STEPS = 15;

    // upper steps
    for (int i = 0; i < NUM_STEPS; i++) {
        int y_offset = i * (30 + 10);

        if (i > floor(NUM_STEPS/2)) {
            y_offset = (NUM_STEPS - i) * (30 + 10);
        }

        int x = 10 + i * (100 + 10);
        int y = 500 - y_offset;
        factory->createStep(registry, x, y);
    }

    // lower steps
    for (int i = 0; i < NUM_STEPS; i++) {
        int y_offset = i * (30 + 10);

        if (i > floor(NUM_STEPS/2)) {
            y_offset = (NUM_STEPS - i) * (30 + 10);
        }

        int x = 10 + i * (100 + 10);
        int y = 500 + y_offset;
        factory->createStep(registry, x, y);
    }

    // Game loop
    SDL_Event event;
    bool running = true;

    while (running)
    {
        // update physics world
        ps->updatePhysics();

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

        // apply physics updates
        ps->syncPhysicsWithRendering(registry);

        // clear before rendering
        SDL_SetRenderDrawColor(renderer, 56, 180, 248, SDL_ALPHA_OPAQUE); // light blue
        SDL_RenderClear(renderer);

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

    ps->destroyWorld();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}