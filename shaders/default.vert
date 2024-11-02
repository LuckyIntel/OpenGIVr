#version 430 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 texUV;

uniform mat4 pvm;
uniform vec2 size;
uniform vec2 zoomX;

out vec2 textureUV;

void main()
{
    textureUV = texUV;
    gl_Position = pvm * vec4(pos * size * zoomX, 0.0, 1.0);    
};