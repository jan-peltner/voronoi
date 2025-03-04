#include "raylib.h"
#include "raymath.h" 
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>

// Screen
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

// Colors
#define PALETTE_N 10

// Seeds
#define SEEDS_N_START 2
#define SEEDS_N_MAX 10
#define SEED_RADIUS 4  
#define SEED_COLOR (Color){0, 0, 0, 127}
#define SEED_VELOCITY(range) (((float)rand() / RAND_MAX) * ((float)range)) - (((float)range) * 0.5f)

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Color color;
} Seed;

typedef struct {
    Seed seeds[SEEDS_N_MAX];
    unsigned int seedCount;
    float edgeThickness;
    bool isPaused;
    bool hideSeeds;
    bool hideEdges;
} State;

int spawnSeed(Seed* seeds, const Color* palette, unsigned int count) {
        seeds[count].position.x = (float)(rand() % SCREEN_WIDTH);
        seeds[count].position.y = (float)(rand() % SCREEN_HEIGHT);
        seeds[count].velocity.x = SEED_VELOCITY(1); // Velocity between -1 and 1
        seeds[count].velocity.y = SEED_VELOCITY(1);
        seeds[count].color = palette[count % 10];
        return count + 1;
}

void paletteToNormalizedFloats(const Color* palette, float* out, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        out[i * 4] = palette[i].r / 255.0f;
        out[i * 4 + 1] = palette[i].g / 255.0f;
        out[i * 4 + 2] = palette[i].b / 255.0f;
        out[i * 4 + 3] = palette[i].a / 255.0f;
    }
}

void handleKeyEvents(State* state, Color* palette) {
    if (IsKeyReleased(KEY_SPACE)) {
        state->isPaused = !state->isPaused;
    }

    if (IsKeyReleased(KEY_E)) {
        state->hideEdges = !state->hideEdges;
    }

    if (IsKeyReleased(KEY_UP)) {
        state->edgeThickness += 1.0f;
        if (state->edgeThickness > 5.0f) state->edgeThickness = 5.0f;
    }

    if (IsKeyReleased(KEY_DOWN)) {  
        state->edgeThickness -= 1.0f;
        if (state->edgeThickness < 1.0f) state->edgeThickness = 1.0f;
    }

    if (IsKeyReleased(KEY_S)) {
        if (state->seedCount < SEEDS_N_MAX) {
            state->seedCount = spawnSeed(state->seeds, palette, state->seedCount);
        } 
    }

    if (IsKeyReleased(KEY_D)) {
        if (state->seedCount > 1) {
            --state->seedCount;
        } 
    }

    if (IsKeyReleased(KEY_H)) {
        state->hideSeeds = !state->hideSeeds;
    }
}

int main(void) {
    // App state
    State state = {{}, 0, 1.0f, false, false, false};

    // Define a subset of the Catppuccin Mocha palette (10 colors)
    Color palette[PALETTE_N] = {
        {245, 224, 220, 255}, // Rosewater
        {242, 205, 205, 255}, // Flamingo
        {245, 194, 231, 255}, // Pink
        {203, 166, 247, 255}, // Mauve
        {243, 139, 168, 255}, // Red
        {250, 179, 135, 255}, // Peach
        {249, 226, 175, 255}, // Yellow
        {166, 227, 161, 255}, // Green
        {148, 226, 213, 255}, // Teal
        {137, 180, 250, 255}  // Blue
    };

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Voronoi");
    SetTargetFPS(60);

    srand(time(NULL));

    // Initialize seeds with random positions, velocities, and colors
    for (int i = 0; i < SEEDS_N_START; ++i) {
        state.seedCount = spawnSeed(state.seeds, palette, state.seedCount);
    }

    // Load voronoi shader (first pass)
    Shader voronoiShader = LoadShader("voronoi.vert", "voronoi.frag");

    // Get handles for the voronoi shader uniforms
    int shdr_seed_count_loc = GetShaderLocation(voronoiShader, "seedCount");
    int shdr_seed_positions_loc = GetShaderLocation(voronoiShader, "seedPositions");
    int shdr_colors_loc = GetShaderLocation(voronoiShader, "seedColors");

    // Load edge detection shader (second pass)
    Shader edgeDetectionShader = LoadShader("voronoi.vert", "edge.frag");
    
    // Get handles for the edge detection shader uniforms
    int edge_thickness_loc = GetShaderLocation(edgeDetectionShader, "edgeThickness");
    int edge_color_loc = GetShaderLocation(edgeDetectionShader, "edgeColor");
    int edge_resolution_loc = GetShaderLocation(edgeDetectionShader, "resolution");

    float resolution[2] = { (float) SCREEN_WIDTH, (float) SCREEN_HEIGHT };
    SetShaderValue(edgeDetectionShader, edge_resolution_loc, (float[2]){(float)SCREEN_WIDTH, (float)SCREEN_HEIGHT}, SHADER_UNIFORM_VEC2 );

    // Black edge color (default)
    Vector4 edgeColor = {0.0f, 0.0f, 0.0f, 1.0f};
    SetShaderValue(edgeDetectionShader, edge_color_loc, &edgeColor, SHADER_UNIFORM_VEC4);

    // Spread Seed and Color struct arrays
    float shdr_colors[PALETTE_N * 4];
    paletteToNormalizedFloats(palette, shdr_colors, PALETTE_N);
    SetShaderValueV(voronoiShader, shdr_colors_loc, shdr_colors, SHADER_UNIFORM_VEC4, PALETTE_N);
    
    // We only need x and y coord fields, omit Color field
    float shdr_seed_positions[SEEDS_N_MAX * 2];

    // Create two render textures for ping-pong rendering
    RenderTexture2D voronoiTarget = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    RenderTexture2D edgeTarget = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Main render loop
    while (!WindowShouldClose()) {
        handleKeyEvents(&state, palette);
        SetShaderValue(voronoiShader, shdr_seed_count_loc, &state.seedCount, SHADER_UNIFORM_INT);
        
        // Update edge thickness in shader
        SetShaderValue(edgeDetectionShader, edge_thickness_loc, &state.edgeThickness, SHADER_UNIFORM_FLOAT);

        if (!state.isPaused) {
            // Update seed positions and handle boundary collisions
            for (unsigned int i = 0; i < state.seedCount; ++i) {
                // Update position with current velocity
                state.seeds[i].position = Vector2Add(state.seeds[i].position, state.seeds[i].velocity);

                // Bounce off left or right edges using radius
                if (state.seeds[i].position.x - SEED_RADIUS < 0) {
                    state.seeds[i].position.x = SEED_RADIUS; // Position seed at edge plus radius
                    state.seeds[i].velocity.x = -state.seeds[i].velocity.x;
                } else if (state.seeds[i].position.x + SEED_RADIUS >= SCREEN_WIDTH) {
                    state.seeds[i].position.x = SCREEN_WIDTH - SEED_RADIUS; // Position seed at edge minus radius
                    state.seeds[i].velocity.x = -state.seeds[i].velocity.x;
                }

                // Bounce off top or bottom edges using radius
                if (state.seeds[i].position.y - SEED_RADIUS < 0) {
                    state.seeds[i].position.y = SEED_RADIUS; // Position seed at edge plus radius
                    state.seeds[i].velocity.y = -state.seeds[i].velocity.y;
                } else if (state.seeds[i].position.y + SEED_RADIUS >= SCREEN_HEIGHT) {
                    state.seeds[i].position.y = SCREEN_HEIGHT - SEED_RADIUS; // Position seed at edge minus radius
                    state.seeds[i].velocity.y = -state.seeds[i].velocity.y;
                }

                shdr_seed_positions[i * 2] = state.seeds[i].position.x;
                shdr_seed_positions[i * 2 + 1] = state.seeds[i].position.y;
            }
        }

        SetShaderValueV(voronoiShader, shdr_seed_positions_loc, shdr_seed_positions, SHADER_UNIFORM_VEC2, state.seedCount);

        // FIRST PASS: Render Voronoi diagram using shader
        BeginTextureMode(voronoiTarget);
            ClearBackground(BLACK);
            BeginShaderMode(voronoiShader);
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
            EndShaderMode();
        EndTextureMode();
        
        // SECOND PASS: Apply edge detection if enabled
        if (!state.hideEdges) {
            BeginTextureMode(edgeTarget);
                ClearBackground(BLACK);
                BeginShaderMode(edgeDetectionShader);
                DrawTexturePro(
                    voronoiTarget.texture,
                    (Rectangle){ 0, 0, (float)voronoiTarget.texture.width, -(float)voronoiTarget.texture.height },
                    (Rectangle){ 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT },
                    (Vector2){ 0, 0 },
                    0.0f,
                    WHITE
                );
                EndShaderMode();
            EndTextureMode();
        }

        // Draw the final frame
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw the appropriate texture based on whether edge detection is enabled
        if (state.hideEdges) {
            DrawTexturePro(
                voronoiTarget.texture,
                (Rectangle){ 0, 0, (float)voronoiTarget.texture.width, (float)voronoiTarget.texture.height },
                (Rectangle){ 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT },
                (Vector2){ 0, 0 },
                0.0f,
                WHITE
            );
        } else {
            DrawTexturePro(
                edgeTarget.texture,
                (Rectangle){ 0, 0, (float)edgeTarget.texture.width, (float)edgeTarget.texture.height },
                (Rectangle){ 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT },
                (Vector2){ 0, 0 },
                0.0f,
                WHITE
            );
        }

        // Draw seeds as filled circles on top
        if (!state.hideSeeds) {
            for (unsigned int i = 0; i < state.seedCount; ++i) {
                DrawCircle((int)state.seeds[i].position.x, (int)state.seeds[i].position.y, SEED_RADIUS, SEED_COLOR);
            }
        }

        // Draw UI text
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, BLACK);
        DrawText(TextFormat("Edge Detection: %s (E)", state.hideEdges ? "OFF" : "ON"), 10, 40, 20, BLACK);
        if (!state.hideEdges) {
            DrawText(TextFormat("Edge Thickness: %.1f (Up/Down)", state.edgeThickness), 10, 70, 20, BLACK);
        }

        EndDrawing();
    }

    // Clean up
    UnloadRenderTexture(voronoiTarget);
    UnloadRenderTexture(edgeTarget);
    UnloadShader(voronoiShader);
    UnloadShader(edgeDetectionShader);
    CloseWindow();

    return 0;
}
