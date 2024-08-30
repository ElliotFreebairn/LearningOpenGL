#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform float horizontalOffset;

out vec3 ourColor;
out vec4 ourPosition;

void main()
{
    float flippedY = aPos.y * -1.0f;
    gl_Position = vec4(aPos.x + horizontalOffset, flippedY, aPos.z, 1.0);
    ourPosition = gl_Position;
    ourColor = aColor;
}