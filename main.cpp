#include <SDL3/SDL.h>
#include <box2d/box2d.h>

#include <array>
#include <vector>
#include <algorithm>
#include <iostream>

struct SimBox {
    b2BodyId bodyId{};
    float width{};
    float height{};
    SDL_FColor color{1.0f, 1.0f, 1.0f, 1.0f};
};

namespace {

constexpr float pixelsPerMeter = 100.0f;

void drawBody(SDL_Renderer *renderer, b2BodyId bodyId, float width, float height, SDL_FColor color, int canvasHeight) {
    if (!b2Body_IsValid(bodyId)) {
        return;
    }

    const float halfWidth = width * 0.5f;
    const float halfHeight = height * 0.5f;
    const std::array<b2Vec2, 4> localCorners{
        b2Vec2{-halfWidth, -halfHeight},
        b2Vec2{halfWidth, -halfHeight},
        b2Vec2{halfWidth, halfHeight},
        b2Vec2{-halfWidth, halfHeight},
    };

    std::array<SDL_Vertex, 4> vertices{};
    for (std::size_t i = 0; i < localCorners.size(); ++i) {
        const b2Vec2 worldPoint = b2Body_GetWorldPoint(bodyId, localCorners[i]);
        vertices[i].position.x = worldPoint.x * pixelsPerMeter;
        vertices[i].position.y = static_cast<float>(canvasHeight) - worldPoint.y * pixelsPerMeter;
        vertices[i].color = color;
        vertices[i].tex_coord = {0.0f, 0.0f};
    }

    const int indices[6] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(renderer, nullptr, vertices.data(), static_cast<int>(vertices.size()), indices, 6);
}

} // namespace

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Bounce++ Demo", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderVSync(renderer, 1);

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, -9.81f};
    b2WorldId worldId = b2CreateWorld(&worldDef);

    constexpr float platformWidth = 6.0f;
    constexpr float platformHeight = 0.5f;

    b2BodyDef groundDef = b2DefaultBodyDef();
    groundDef.position = {4.0f, 1.0f};
    b2BodyId groundId = b2CreateBody(worldId, &groundDef);

    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    groundShapeDef.density = 0.0f;
    groundShapeDef.material.friction = 0.8f;
    groundShapeDef.material.restitution = 0.1f;
    groundShapeDef.updateBodyMass = false;
    const b2Polygon groundPolygon = b2MakeBox(platformWidth * 0.5f, platformHeight * 0.5f);
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundPolygon);

    std::vector<SimBox> boxes;
    boxes.reserve(5);

    for (int i = 0; i < 5; ++i) {
        const float width = 0.5f;
        const float height = 0.5f + 0.1f * (i % 2);

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {2.5f + 0.6f * i, 4.0f + 0.3f * i};
        bodyDef.angularDamping = 0.15f;

        b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.material.friction = 0.4f;
        shapeDef.material.restitution = 0.3f;
        const b2Polygon boxPolygon = b2MakeBox(width * 0.5f, height * 0.5f);
        b2CreatePolygonShape(bodyId, &shapeDef, &boxPolygon);

        const float red = std::clamp((160.0f + i * 15.0f) / 255.0f, 0.0f, 1.0f);
        const float green = std::clamp((80.0f + i * 20.0f) / 255.0f, 0.0f, 1.0f);
        const float blue = std::clamp((200.0f - i * 20.0f) / 255.0f, 0.0f, 1.0f);
        const SDL_FColor color{red, green, blue, 1.0f};
        boxes.push_back(SimBox{bodyId, width, height, color});
    }

    bool running = true;
    Uint64 previousTicks = SDL_GetTicks();
    float accumulator = 0.0f;
    const float timeStep = 1.0f / 60.0f;
    int windowWidth = 800;
    int windowHeight = 600;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                windowWidth = event.window.data1;
                windowHeight = event.window.data2;
            }
        }

        SDL_GetWindowSizeInPixels(window, &windowWidth, &windowHeight);

        const Uint64 currentTicks = SDL_GetTicks();
        float frameTime = static_cast<float>(currentTicks - previousTicks) / 1000.0f;
        if (frameTime > 0.25f) {
            frameTime = 0.25f;
        }
        previousTicks = currentTicks;
        accumulator += frameTime;

        while (accumulator >= timeStep) {
            b2World_Step(worldId, timeStep, 4);
            accumulator -= timeStep;
        }

        SDL_SetRenderDrawColor(renderer, 20, 24, 35, 255);
        SDL_RenderClear(renderer);

        drawBody(renderer, groundId, platformWidth, platformHeight,
                 SDL_FColor{90.0f / 255.0f, 90.0f / 255.0f, 120.0f / 255.0f, 1.0f}, windowHeight);

        for (const auto& box : boxes) {
            drawBody(renderer, box.bodyId, box.width, box.height, box.color, windowHeight);
        }

        SDL_RenderPresent(renderer);
    }

    b2DestroyWorld(worldId);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
