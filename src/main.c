#include "raylib.h"
#include "raymath.h" 
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

int spawnSeed(Seed* seeds, const Color* palette, int count) {
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

int main(void) {
    bool is_paused = false; 
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Voronoi");
    SetTargetFPS(60);

    srand(time(NULL));

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
    

    // Initialize seeds with random positions, velocities, and colors
    Seed seeds[SEEDS_N_MAX];
    int seed_count = 0;
    for (int i = 0; i < SEEDS_N_START; ++i) {
        seed_count = spawnSeed(seeds, palette, seed_count);
    }

    Shader shdr = LoadShader("voronoi.vert", "voronoi.frag");

    // Get handles for the shader uniforms
    int shdr_seed_count_loc = GetShaderLocation(shdr, "seedCount");
    int shdr_seed_positions_loc = GetShaderLocation(shdr, "seedPositions");
    int shdr_colors_loc = GetShaderLocation(shdr, "seedColors");
    

    // Spread Seed and Color struct arrays
    float shdr_colors[PALETTE_N * 4];
    paletteToNormalizedFloats(palette, shdr_colors, PALETTE_N);
    SetShaderValueV(shdr, shdr_colors_loc, shdr_colors, SHADER_UNIFORM_VEC4, PALETTE_N);
    // We only need x and y coord fields, omit Color field
    float shdr_seed_positions[SEEDS_N_MAX * 2];

    RenderTexture2D target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);


    // Main render loop
    while (!WindowShouldClose()) {

        if (IsKeyReleased(KEY_S)) {
            if (seed_count < SEEDS_N_MAX) {
                seed_count = spawnSeed(seeds, palette, seed_count);
            } 
        }

        if (IsKeyReleased(KEY_D)) {
            if (seed_count > 1) {
                --seed_count;
            } 
        }

        if (IsKeyReleased(KEY_SPACE)) {
            is_paused = !is_paused;
        }


        SetShaderValue(shdr, shdr_seed_count_loc, &seed_count, SHADER_UNIFORM_INT);

        if (!is_paused) {
            // Update seed positions and handle boundary collisions
            for (int i = 0; i < seed_count ; ++i) {
                // Update position with current velocity
                seeds[i].position = Vector2Add(seeds[i].position, seeds[i].velocity);

                // Bounce off left or right edges using radius
                if (seeds[i].position.x - SEED_RADIUS < 0) {
                    seeds[i].position.x = SEED_RADIUS; // Position seed at edge plus radius
                    seeds[i].velocity.x = -seeds[i].velocity.x;
                } else if (seeds[i].position.x + SEED_RADIUS >= SCREEN_WIDTH) {
                    seeds[i].position.x = SCREEN_WIDTH - SEED_RADIUS; // Position seed at edge minus radius
                    seeds[i].velocity.x = -seeds[i].velocity.x;
                }

                // Bounce off top or bottom edges using radius
                if (seeds[i].position.y - SEED_RADIUS < 0) {
                    seeds[i].position.y = SEED_RADIUS; // Position seed at edge plus radius
                    seeds[i].velocity.y = -seeds[i].velocity.y;
                } else if (seeds[i].position.y + SEED_RADIUS >= SCREEN_HEIGHT) {
                    seeds[i].position.y = SCREEN_HEIGHT - SEED_RADIUS; // Position seed at edge minus radius
                    seeds[i].velocity.y = -seeds[i].velocity.y;
                }

                shdr_seed_positions[i * 2] = seeds[i].position.x;
                shdr_seed_positions[i * 2 + 1] = seeds[i].position.y;
            }
        }

        SetShaderValueV(shdr, shdr_seed_positions_loc, shdr_seed_positions, SHADER_UNIFORM_VEC2, seed_count);

        // Render Voronoi diagram using shader
        BeginTextureMode(target);
            ClearBackground(BLACK);
            BeginShaderMode(shdr);
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
            EndShaderMode();

        EndTextureMode();

        // Draw the frame
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw diagram previously rendered by shader
        DrawTextureRec(target.texture, 
            (Rectangle){ 0, 0, (float)target.texture.width, (float)target.texture.height }, 
            (Vector2){ 0, 0 }, 
            WHITE);
        // Draw seeds as filled circles on top
        for (int i = 0; i < seed_count; ++i) {
                DrawCircle((int)seeds[i].position.x, (int)seeds[i].position.y, SEED_RADIUS, SEED_COLOR);
        }

        EndDrawing();
    }

    // Clean up
    UnloadRenderTexture(target);
    UnloadShader(shdr);
    CloseWindow();

    return 0;
}
