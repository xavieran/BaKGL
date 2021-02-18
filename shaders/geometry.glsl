#version 330
layout(triangles) in;
layout(line_strip, max_vertices=8) out;

uniform float normal_length;
uniform mat4 gxl3d_ModelViewProjectionMatrix;

in Vertex
{
  vec4 normal;
  vec4 color;
} vertex[];

out vec4 vertex_color;

void main()
{
  int i;
  
  //------ 3 lines for the 3 vertex normals
  //
  for(i=0; i<gl_in.length(); i++)
  {
    vec3 P = gl_in[i].gl_Position.xyz;
    vec3 N = vertex[i].normal.xyz;
    
    gl_Position = gxl3d_ModelViewProjectionMatrix * vec4(P, 1.0);
    vertex_color = vertex[i].color;
    EmitVertex();
    
    gl_Position = gxl3d_ModelViewProjectionMatrix * vec4(P + N * normal_length, 1.0);
    vertex_color = vertex[i].color;
    EmitVertex();
    
    EndPrimitive();
  }
  

  //------ One line for the face normal
  //
  vec3 P0 = gl_in[0].gl_Position.xyz;
  vec3 P1 = gl_in[1].gl_Position.xyz;
  vec3 P2 = gl_in[2].gl_Position.xyz;
  
  vec3 V0 = P0 - P1;
  vec3 V1 = P2 - P1;
  
  vec3 N = cross(V1, V0);
  N = normalize(N);
  
  // Center of the triangle
  vec3 P = (P0+P1+P2) / 3.0;
  
  gl_Position = gxl3d_ModelViewProjectionMatrix * vec4(P, 1.0);
  vertex_color = vec4(1, 0, 0, 1);
  EmitVertex();
  
  gl_Position = gxl3d_ModelViewProjectionMatrix * vec4(P + N * normal_length, 1.0);
  vertex_color = vec4(1, 0, 0, 1);
  EmitVertex();
  EndPrimitive();
}
