#version 330 core

struct Light
{
    vec3 mDirection;
    vec3 mAmbientColor;
    vec3 mDiffuseColor;
    vec3 mSpecularColor;
};

in vec3 Position_worldspace;
in vec3 Position_lightSpace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 lightDirection_cameraspace;

in vec4 vertexColor;
in vec3 uvCoords;
in float texBlend;
in float DistanceFromCamera;

// Ouput data
out vec4 color;

uniform Light light;
uniform sampler2DArray texture0;
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = (currentDepth - .001) > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

void main()
{
    float k = .0005;
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

    vec3 materialAmbientColor = diffuseColor;
    vec3 materialSpecularColor = materialDiffuseColor;

    // Normal of the computed fragment, in camera space
    vec3 n = Normal_cameraspace;
    // Direction of the light (from the fragment to the light)
    vec3 l = lightDirection_cameraspace;

    // Cosine of the angle between the normal and the light direction, 
    // clamped above 0
    //  - light is at the vertical of the triangle -> 1
    //  - light is perpendicular to the triangle -> 0
    //  - light is behind the triangle -> 0
    float cosTheta = clamp(dot(n, l), 0, 1);
    
    // Eye vector (towards the camera)
    vec3 E = EyeDirection_cameraspace;
    // Direction in which the triangle reflects the light
    vec3 R = reflect(-l, n);
    // Cosine of the angle between the Eye vector and the Reflect vector,
    // clamped to 0
    //  - Looking into the reflection -> 1
    //  - Looking elsewhere -> < 1
    float cosAlpha = clamp(dot(E, R), 0, 1);

    float shadow = ShadowCalculation(vec4(Position_lightSpace, 1));

    vec3 litColor
        = materialAmbientColor * light.mAmbientColor
        + (1 - shadow) 
            * (cosTheta * diffuseColor * light.mDiffuseColor
            + pow(cosAlpha, 4) * materialSpecularColor * light.mSpecularColor);
    
    vec3 foggedColor = mix(fogColor, litColor, fogFactor);

    color = vec4(foggedColor, alpha);
}

