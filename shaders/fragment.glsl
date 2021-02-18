#version 330 core

in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 lightDirection_cameraspace;
in vec4 vertexColor;
in vec3 uvCoords;
in float texBlend;
in float DistanceFromCamera;

// Ouput data
out vec4 color;

uniform mat4 MV;
uniform vec3 lightPosition_worldspace;
uniform sampler2DArray texture0;

void main(){

    // light emission properties
    // You probably want to put them as uniforms
    vec3 lightColor = vec3(1,1,1);
    float lightPower = 100000.0f;
    
    float k = .0001;
    vec3 fogColor   = vec3(0.15, 0.31, 0.36);
    float fogFactor = exp(-DistanceFromCamera * k);
    
    vec3 materialDiffuseColor = vertexColor.xyz;
    float materialAlpha = vertexColor.a;

    vec4 textureSample = texture(texture0, uvCoords);
    vec3 textureColor  = textureSample.xyz;
    float textureAlpha = textureSample.a;

    // Choose either vertex color or texture color with the texBlend
    vec3 diffuseColor = mix(materialDiffuseColor, textureColor, texBlend);
    float alpha       = mix(materialAlpha, textureAlpha, texBlend);

    if (alpha == 0) discard;

    //vec3 materialAmbientColor = vec3(0.1,0.1,0.1) * diffuseColor;
    vec3 materialAmbientColor = .1 * diffuseColor;
    vec3 materialSpecularColor = vec3(0.1);

    // Distance to the light
    float distance = length(lightPosition_worldspace - Position_worldspace);

    // Normal of the computed fragment, in camera space
    vec3 n = normalize(Normal_cameraspace);
    // Direction of the light (from the fragment to the light)
    vec3 l = normalize(lightDirection_cameraspace);
    // Cosine of the angle between the normal and the light direction, 
    // clamped above 0
    //  - light is at the vertical of the triangle -> 1
    //  - light is perpendicular to the triangle -> 0
    //  - light is behind the triangle -> 0
    float cosTheta = clamp(dot(n, l), 0, 1);
    
    // Eye vector (towards the camera)
    vec3 E = normalize(EyeDirection_cameraspace);
    // Direction in which the triangle reflects the light
    vec3 R = reflect(-l, n);
    // Cosine of the angle between the Eye vector and the Reflect vector,
    // clamped to 0
    //  - Looking into the reflection -> 1
    //  - Looking elsewhere -> < 1
    float cosAlpha = clamp(dot(E, R), 0, 1);

    vec3 litColor = 
          materialAmbientColor 
        + diffuseColor * lightColor * lightPower * cosTheta
            / (distance * distance)
        + materialSpecularColor * lightColor * lightPower * pow(cosAlpha, 4) 
            / (distance * distance);
    
    vec3 foggedColor = mix(fogColor, litColor, fogFactor);

    color = vec4(foggedColor, alpha);
}

