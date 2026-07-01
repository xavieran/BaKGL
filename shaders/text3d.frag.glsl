#version 330 core

in vec3 uvCoords;
out vec4 color;

uniform sampler2DArray texture0;
uniform vec4 uColor;

void main() {
    float alpha = texture(texture0, uvCoords).a * uColor.a;
    if (alpha == 0) discard;
    color = vec4(uColor.rgb, alpha);
}
