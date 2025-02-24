#include "raylib.h"
#include "raymath.h"  // Include for Vector2 operations
#include <stdlib.h>
#include <time.h>
#include <float.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define NUM_SEEDS 10
#define SEED_RADIUS 4  
#define SEED_COLOR (Color){0, 0, 0, 100}
#define SEED_VELOCITY(range) ((float)(range) * 0.5f)

typedef struct {
    Vector2 position;  // Position using Vector2
    Vector2 velocity;  // Velocity using Vector2
    Color color;       // Color from the Catppuccin palette
} Seed;

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Voronoi");
    SetTargetFPS(60);

    srand(time(NULL));

    // Define a subset of the Catppuccin Mocha palette (10 colors)
    Color palette[10] = {
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
    Seed seeds[NUM_SEEDS];
    for (int i = 0; i < NUM_SEEDS; i++) {
        seeds[i].position.x = (float)(rand() % SCREEN_WIDTH);
        seeds[i].position.y = (float)(rand() % SCREEN_HEIGHT);
        seeds[i].velocity.x = ((float)rand() / RAND_MAX) * SEED_VELOCITY(2) - SEED_VELOCITY(1); // Velocity between -1 and 1
        seeds[i].velocity.y = ((float)rand() / RAND_MAX) * SEED_VELOCITY(2) - SEED_VELOCITY(1);
        seeds[i].color = palette[i % 10];
    }

    // Allocate pixel color buffer
    Color* pixelColors = malloc(sizeof(Color) * SCREEN_WIDTH * SCREEN_HEIGHT);
    if (!pixelColors) {
        CloseWindow();
        return 1; // Exit if malloc fails
    }

    // Create a texture for rendering
    Image image = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    // Main render loop
    while (!WindowShouldClose()) {
        // Update seed positions and handle boundary collisions
        for (int i = 0; i < NUM_SEEDS; i++) {
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
        }

        // Compute the Voronoi diagram
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                int closestSeed = 0;
                float minDistSquared = FLT_MAX;

                // Find the closest seed for this pixel
                for (int i = 0; i < NUM_SEEDS; i++) {
                    float dx = (float)x - seeds[i].position.x;
                    float dy = (float)y - seeds[i].position.y;
                    float distSquared = dx * dx + dy * dy;
                    if (distSquared < minDistSquared) {
                        minDistSquared = distSquared;
                        closestSeed = i;
                    }
                }

                // Assign the color of the closest seed to this pixel
                pixelColors[y * SCREEN_WIDTH + x] = seeds[closestSeed].color;
            }
        }

        // Update the texture with the new pixel data
        UpdateTexture(texture, pixelColors);

        // Draw the frame
        BeginDrawing();
        ClearBackground(BLACK);              // Clear the screen
        DrawTexture(texture, 0, 0, WHITE);   // Draw the Voronoi diagram first
        // Draw seeds as filled circles on top
        for (int i = 0; i < NUM_SEEDS; i++) {
            DrawCircle((int)seeds[i].position.x, (int)seeds[i].position.y, SEED_RADIUS, SEED_COLOR);
        }
        EndDrawing();
    }

    // Clean up
    free(pixelColors);
    UnloadTexture(texture);
    CloseWindow();

    return 0;
}
