#version 330 core

out vec4 FragColor;
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    //built in mix function linerarly interpolates based on 3rd value
    FragColor = mix((texture(texture1, TexCoord) * vec4(ourColor, 1.0)), texture(texture2, TexCoord), 0.2); //blends both textures together 80% first input color 20% second input color
}