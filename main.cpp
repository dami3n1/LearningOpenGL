#include <cmath>
#include <iostream>
#include <ostream>
#include <glad/glad.h> //glad should always be put first to prevent redefinition use of OpenGL
#include <GLFW/glfw3.h>
#include "shader_reader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb/stb_image.h"

const unsigned int SCREEN_HEIGHT = 600;
const unsigned int SCREEN_WIDTH = 800;

// glfw will autoatically fill in data
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // set viewport sie for opengl you can make this smaller than the window size and render stuff behind it
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // if it's not pressed, glfwGetKey returns GLFW_RELEASE
        glfwSetWindowShouldClose(window, true);
}

int main()
{
    glfwInit(); // Initiates glfw (returns GL_TRUE if successfull)

    // configure the next glfwCreateWindow() with glfwWindowHint();
    // tells glfw what OpenGL Version will be used Program will crash if client does not have proper version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // create window object
    // define how window should be set up and creates it
    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // makes a context for the window and assigns to it
    glfwMakeContextCurrent(window);
    // tell glfw to call function when window resize; glfw will autoatically fill in data for framebuffer_size_callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // initialize GLAD before we can use any opengl functions
    // cast's the glfw function which gives the OS specific function for GLAD to find
    // the OpenGL function pointer (memory address of opengl executable command)(hardware specific)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // set glViewport so first frame renders correctly before hiting our resize function
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    Shader ourShader("../shaders/vertex.vs", "../shaders/fragment.fs");

    float vertices[] = {
        // positions          // colors           // texture coords
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
    };

    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    // VBO = stores the actual vertex data (like positions, colors, etc.)
    // VAO = remembers how to use that data (how OpenGL should read it)
    // EBO = stores index data (which vertices to draw and in what order)

    // These variables will hold IDs (like handles) for the buffers
    unsigned int VBO, VAO, EBO;

    // Create 1 VAO and store its ID in VAO
    glGenVertexArrays(1, &VAO);

    // Create 1 VBO and store its ID in VBO
    glGenBuffers(1, &VBO);

    // Create 1 EBO and store its ID in EBO
    glGenBuffers(1, &EBO);

    // Bind the VAO so OpenGL knows we are working with it now
    // Any setup we do will be saved inside this VAO
    glBindVertexArray(VAO);

    // Bind the VBO to GL_ARRAY_BUFFER
    // This means we are now working with this VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Copy vertex data into the VBO
    // sizeof(vertices) = size of the data
    // vertices = the actual data
    // GL_STATIC_DRAW = data will not change much
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
    // GL_STATIC_DRAW: the data is set only once and used many times.
    // GL_DYNAMIC_DRAW: the data is changed a lot and used many times.

    // bind EBO and copy indicies into buffer like VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // more info look at VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Tell OpenGL how to read the vertex data from the VBO

    // 0 = location in the vertex shader (layout(location = 0))
    // 3 = number of values per vertex (x, y, z)
    // GL_FLOAT = type of each value
    // GL_FALSE = do NOT normalize the data
    // 3 * sizeof(float) = size of one vertex (3 floats total)
    // (void*)0 = start reading at the beginning of the data
    // IMPORTANT: this uses the VBO currently bound to GL_ARRAY_BUFFER
    // read points from vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);

    // Enable the vertex attribute at location 0 so OpenGL can use it
    glEnableVertexAttribArray(0);

    // read color from vertex data
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // data for where to place texture
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // IMPORTANT:
    // Do NOT unbind the EBO while the VAO is still bound
    // The EBO is stored inside the VAO, so it needs to stay bound

    // Unbind the VAO so we don’t accidentally modify it later
    glBindVertexArray(0);

    // like objects textures are also refrenced by an ID
    unsigned int texture1, texture2;
    glGenTextures(1, &texture1);
    // bind texture so any texture commands willl go to this texture
    glBindTexture(GL_TEXTURE_2D, texture1);

    // s,t,r = x,y,z
    // texture wrapping/filtering/mipmap options
    // wrapping repeats the image horizontally and vertically
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // linearly interpolates between the two mipmaps that most closely match the size of a pixel and samples the interpolated level via nearest neighbor interpolation.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // blends colors but no mipmap option (only works when downscaling)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // use stbimage to read image data
    int width, height, nrChannels;          // stb fills this with data
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = stbi_load("../assets/container.jpg", &width, &height, &nrChannels, 0);

    if (data)
    {
        // target: generate texture on currently bound texture (GL_TEXTURE_2D)
        // level( of detail(mipmap)): 0 = regular image
        // format of color
        // image size width and height
        // border must be 0
        // format of pixel data
        // data type of pixel data
        // pointer to the actual image data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        // generate mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    // free the image from memory
    stbi_image_free(data);

    // texture 2
    // ---------
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    data = stbi_load("../assets/awesomeface.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    ourShader.use(); // don't forget to activate/use the shader before setting uniforms!
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);

    // Unbind the VBO (optional, just to avoid accidental changes later)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // input function called each frame
        processInput(window);

        // glClear uses this color to clear the screen (sets it doesnt have to be called always)
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // clear the buffer so you wont see previous frame (you have to specify which one)
        glClear(GL_COLOR_BUFFER_BIT);

        // allows to use more than one texture assigning it to the texture unit(GL_TEXTURE0)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // call shader program
        ourShader.use();

        // call the configuration
        glBindVertexArray(VAO);
        // Draw triangles using the indices stored in the currently bound EBO (GL_ELEMENT_ARRAY_BUFFER)
        // Normally, you'd have to bind the correct EBO for each object before calling glDrawElements.
        // However, VAOs remember which EBO was bound when the VAO was created.
        // So simply binding the VAO automatically binds the right EBO, making rendering easier.
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window); // swaps color buffer in window
        glfwPollEvents();        // processes events received in window and returns a response(if requested)
    }

    // de allocated resources after usage
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // clean glfw resources;
    glfwTerminate();
    return 0;
}
