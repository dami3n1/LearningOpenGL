#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D windowTexture;

void main()
{
    vec4 windowColor = texture(windowTexture, TexCoords);

    // The PNG occupies a rectangular quad. Do not draw its transparent border.
    if (windowColor.a < 0.05)
        discard;

    // Keep the source alpha so the scene remains visible through the glass.
    FragColor = windowColor;
}
