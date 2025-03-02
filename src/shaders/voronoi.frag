#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

// Uniforms
uniform int seedCount;        // Number of seeds (points)
uniform vec2 seedPositions[20]; // Array of seed positions (x,y)
uniform vec4 seedColors[10];  // Array of seed colors (r,g,b,a)

void main()
{
	float minDist = 1000000000.0;
	float minDistSec = 1000000001.0;
	int closestSeed = 0;

	for (int i = 0; i < seedCount; ++i) {
		float dx = gl_FragCoord.x - seedPositions[i].x;
		float dy = gl_FragCoord.y - seedPositions[i].y;
		float dist = sqrt(dx * dx + dy * dy);
		if (dist < minDist) { 
			minDistSec = minDist;
			minDist = dist;
			closestSeed = i;
		}
	}
	if (minDistSec - minDist < 2) {
		// finalColor = vec4(76.0 / 255.0, 79.0 / 255.0, 105.0 / 255.0, 1.0);
		finalColor = vec4(0);
	} else {
		finalColor = seedColors[closestSeed % 10];
	}
}

