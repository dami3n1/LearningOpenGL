#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float mixValue;

void main()
{
    //built in mix function linerarly interpolates based on 3rd value
    FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), mixValue); //blends both textures together 80% first input color 20% second input color
}