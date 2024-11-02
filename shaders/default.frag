#version 430 core

out vec4 FragColor;
in vec2 textureUV;

uniform int applyTexture;
uniform sampler2D photoTexture;

void main()
{
    if (applyTexture == 1)
    {
        FragColor = texture(photoTexture, textureUV);
    } else {
        FragColor = vec4(1.0f);
    };
};