#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

// Uniforms
uniform int seedCount;        // Number of seeds (points)
uniform vec2 seedPositions[10]; // Array of seed positions (x,y)
uniform vec4 seedColors[10];  // Array of seed colors (r,g,b,a)

void main()
{
	vec2 pixelPos = gl_FragCoord.xy;

	// Find closest seed (Voronoi diagram computation)
	float minDistSqr = 1000000.0;
	int closestSeed = 0;

	for (int i = 0; i < seedCount; ++i) {
		float dx = pixelPos.x - seedPositions[i].x;
		float dy = pixelPos.y - seedPositions[i].y;
		float distSqr = dx * dx + dy * dy;

		if (distSqr < minDistSqr) {
			minDistSqr = distSqr;
			closestSeed = i;
		}
	}

	finalColor = seedColors[closestSeed];
}

