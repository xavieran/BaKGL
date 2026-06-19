#version 330 core

in vec3 Position_worldspace;
in vec3 Position_screenspace;
in vec3 uvCoords;
// Apple's GLSL linker requires all vertex outputs to be read by the fragment shader.
// Guard against infinite positions so the variable is considered "used".
#define CONSUME_WORLDSPACE if (any(isinf(Position_worldspace))) discard;
flat in int  colorMode;
in vec4 blockColor;

// Ouput data
out vec4 color;

uniform sampler2DArray texture0;

// colorMode
// 0 :: Use Texture
// 1 :: Use Solid Color
// 2 :: Use mix of texture and solid color (TintColor)
// 3 :: Use solid color with texture alpha (ReplaceColor)

void main()
{
    CONSUME_WORLDSPACE
    vec4 textureSample = texture(texture0, uvCoords);
    vec3 textureColor  = textureSample.xyz;
    vec3 blockColorB   = blockColor.xyz;
    if (colorMode == 1) // block mode
        color = blockColor;
    else if (colorMode == 2) // tint mode
        color = vec4(
            mix(blockColorB, textureColor, .5),
            textureSample.a);
    else if (colorMode == 3) // replace mode
        color = vec4(
            blockColorB,
            textureSample.a);
    else
        color = vec4(textureColor, textureSample.a);

    if (color.a == 0) discard;
}
