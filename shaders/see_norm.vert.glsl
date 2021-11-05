#version 150

// Input vertex data, different for all executions of this shader.
in vec3 vertexPosition_modelspace;
in vec3 vertexNormal_modelspace;
in vec4 vertexColor_modelspace;
in vec3 textureCoords;
in float texBlendVec;


in vec4 gxl3d_Position;
in vec4 gxl3d_Normal;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPosition_worldspace;
uniform vec3 cameraPosition_worldspace;

out Vertex
{
  vec4 normal;
  vec4 color;
} vertex;

void main()
{
  gl_Position =  vec4(vertexPosition_modelspace,1);
  vertex.normal = vec4(vertexNormal_modelspace, 1);
  vertex.color  =  vec4(1.0, 1.0, 0.0, 1.0);
}
