#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
    // z component of the position is set to the w component to ensure that the depth value is always 1.0, which places the skybox at the farthest depth in the scene, preventing it from being clipped by other geometry.
    // set incoming local position vector as the outcoming texture coordinate for interpolated use in the fragment shader
    // fragment shader will sample from the cubemap texture using the interpolated texture coordinates
}