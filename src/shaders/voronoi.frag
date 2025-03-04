#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

// Uniforms
uniform int seedCount;
uniform vec2 seedPositions[20];
uniform vec4 seedColors[10];

void main()
{
	float minDist = 1000000000.0;
	int closestSeed = 0;

	for (int i = 0; i < seedCount; ++i) {
		float dx = gl_FragCoord.x - seedPositions[i].x;
		float dy = gl_FragCoord.y - seedPositions[i].y;
		float dist = sqrt(dx * dx + dy * dy);
		if (dist < minDist) { 
			minDist = dist;
			closestSeed = i;
		} 
	}
	finalColor = seedColors[closestSeed % 10];
}

