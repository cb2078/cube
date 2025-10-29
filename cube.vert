#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec3 cubie_translation;
layout(location = 2) in vec3 facelet_colour;

out vec3 VertexColour;

float scale = 1.07;
uniform mat4 facelet_transforms[6];
uniform mat4 cubie_transforms[27];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = scale * vec4(cubie_translation, 1) + facelet_transforms[gl_InstanceID % 6] * vec4(pos, 0, 1);
    gl_Position = cubie_transforms[gl_InstanceID / 6] * gl_Position;
    gl_Position = projection * view * model * gl_Position;
    VertexColour = facelet_colour;
}
