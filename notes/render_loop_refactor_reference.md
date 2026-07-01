# Render-loop refactor reference

This file is intentionally not part of the build. It records the alternative
shared-render-function layout:

```cpp
auto renderScene = [&](const glm::mat4 &view,
                       const glm::mat4 &projection,
                       const std::map<float, glm::vec3> &sortedWindows)
{
    // Draw cubes, models, floor, skybox, and transparent objects.
};

// Draw into the off-screen color texture.
glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
renderScene(framebufferView, framebufferProjection, sorted);

// Draw directly into the regular window.
glBindFramebuffer(GL_FRAMEBUFFER, 0);
renderScene(screenView, screenProjection, sorted);
```

The active `main.cpp` keeps both passes expanded so shader activation, uniforms,
texture bindings, and draw calls remain visible while learning OpenGL.
