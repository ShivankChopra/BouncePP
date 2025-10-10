#include <iostream>
#include <vector>
#include <box2d/box2d.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

constexpr int BLOCK_WIDTH = 100;
constexpr int BLOCK_HEIGHT = 100;

constexpr int PLATFORM_WIDTH = 1000;
constexpr int PLATFORM_HEIGHT = 10;

static constexpr float PPM = 50.0f; // 50 pixels = 1 meter
inline float px_to_m(const float px) { return px / PPM; }
inline float m_to_px(const float m) { return m * PPM; }

// rectangular movable boxes
class MyBlocks {

    SDL_FRect _rect{};
    b2BodyDef _bodyDef;
    b2ShapeDef _shapeDef;
    b2Polygon _polygon;
    b2BodyId _physicsBodyId;

public:
    MyBlocks(const float x, const float y, const b2WorldId worldId) {
        const float center_x_m = px_to_m(x + BLOCK_WIDTH / 2.0f);
        const float center_y_m = px_to_m(y + BLOCK_HEIGHT / 2.0f);

        _bodyDef = b2DefaultBodyDef();
        _bodyDef.type = b2_dynamicBody;
        _bodyDef.position = {center_x_m, center_y_m};

        _physicsBodyId = b2CreateBody(worldId, &_bodyDef);

        _shapeDef = b2DefaultShapeDef();
        _shapeDef.density = 20.0f;
        _shapeDef.material.friction = 0.4f;
        _shapeDef.material.restitution = 0.7f;

        _polygon = b2MakeBox(px_to_m(BLOCK_WIDTH / 2.0f), px_to_m(BLOCK_HEIGHT / 2.0f));

        b2CreatePolygonShape(_physicsBodyId, &_shapeDef, &_polygon);

        _rect = {x, y, BLOCK_WIDTH, BLOCK_HEIGHT};
    }

    void UpdatePosition(const b2WorldId worldId) {
        auto [x, y] = b2Body_GetPosition(_physicsBodyId);
        _rect.x = m_to_px(x) - BLOCK_WIDTH / 2.0f;
        _rect.y = m_to_px(y) - BLOCK_HEIGHT / 2.0f;
    }

    [[nodiscard]] const SDL_FRect& getRect() const {
        return _rect;
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

    [[nodiscard]] const SDL_FRect& getRect() const {
        return _rect;
    }
};

int main(int argc, char* argv[])
{
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "SDL3 Experiment Window",
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

    SDL_Log("SDL3 Window created successfully!");

    // physics world
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2(0.0, 0.98f);
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    // blocks
    constexpr int NUM_RECTS = 5;
    std::vector<std::unique_ptr<MyBlocks>> myBlocks;
    for (int i = 0; i < NUM_RECTS; ++i) {
        myBlocks.push_back(std::make_unique<MyBlocks>(100 + i * (100 + 10), 100, worldId));
    }

    // solid platform
    const auto platform = std::make_unique<MyPlatform>(10, 500, worldId);

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
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); // black
        SDL_RenderClear(renderer);

        // display blocks
        SDL_SetRenderDrawColor(renderer, 53, 20, 135, SDL_ALPHA_OPAQUE); // purple
        for (const std::unique_ptr<MyBlocks>& block : myBlocks) {
            block->UpdatePosition(worldId);
            SDL_RenderRect(renderer, &block->getRect());
        }

        // display platform
        SDL_SetRenderDrawColor(renderer, 164, 96, 8, SDL_ALPHA_OPAQUE); // purple
        SDL_RenderRect(renderer, &platform->getRect());

        SDL_RenderPresent(renderer);

        SDL_Delay(16); // A small delay (approx. 60 FPS)
    }

    b2DestroyWorld(worldId);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}