#version 330 core

in vec3 VertexColour;
out vec4 FragColour;

void main()
{
    FragColour = vec4(VertexColour / 0xff, 1.0f);
}
