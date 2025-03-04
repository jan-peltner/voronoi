#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0; // Texture from first pass
uniform float edgeThickness;
uniform vec4 edgeColor; 
uniform vec2 resolution;

void main()
{
    vec2 texSize = textureSize(texture0, 0);
    vec2 texelSize = 1.0 / texSize;

    vec4 centerTexColor = texture2D(texture0, fragTexCoord);

    bool isEdge = false;

    // Check surrounding pixels within the specified range
    for (float dy = -edgeThickness; dy <= edgeThickness && !isEdge; ++dy) {
        for (float dx = -edgeThickness; dx <= edgeThickness && !isEdge; ++dx) {
            // Skip the center pixel
            if (dx == 0.0 && dy == 0.0) continue;

            // Sample neighbor pixel by taking into account UV offset
            vec2 neighborTexCoord = vec2(fragTexCoord.x + dx * texelSize.x, fragTexCoord.y + dy * texelSize.y);

            // Skip if we're on the edge of the texture (prevent framing effect)
            if (neighborTexCoord.x < 0 || neighborTexCoord.x > 1 || neighborTexCoord.y < 0 || neighborTexCoord.y > 1) continue;
            vec4 neighborTexColor = texture2D(texture0, neighborTexCoord);

            if (neighborTexColor != centerTexColor) {
                isEdge = true;
            }
        }
    }
    finalColor = isEdge ? edgeColor : centerTexColor;
}
