#include <cmath>
#include <iostream>
#include <ostream>
#include <glad/glad.h> //glad should always be put first to prevent redefinition use of OpenGL
#include <GLFW/glfw3.h>

const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "out vec3 ourColor;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "   ourColor = aColor;\n" // set ourColor to the input color we got from vertex data
        "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(ourColor, 1.0);\n"
        "}\0";

const unsigned int SCREEN_HEIGHT = 600;
const unsigned int SCREEN_WIDTH = 800;

//glfw will autoatically fill in data
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    //set viewport sie for opengl you can make this smaller than the window size and render stuff behind it
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // if it's not pressed, glfwGetKey returns GLFW_RELEASE
        glfwSetWindowShouldClose(window, true);
}

int main() {
    glfwInit(); // Initiates glfw (returns GL_TRUE if successfull)

    //configure the next glfwCreateWindow() with glfwWindowHint();
    //tells glfw what OpenGL Version will be used Program will crash if client does not have proper version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    //create window object
    //define how window should be set up and creates it
    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    //makes a context for the window and assigns to it
    glfwMakeContextCurrent(window);
    //tell glfw to call function when window resize; glfw will autoatically fill in data for framebuffer_size_callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //initialize GLAD before we can use any opengl functions
    //cast's the glfw function which gives the OS specific function for GLAD to find
    //the OpenGL function pointer (memory address of opengl executable command)(hardware specific)
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //set glViewport so first frame renders correctly before hiting our resize function
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // This will store the ID (handle) for the vertex shader
    unsigned int vertexShader;

    // Create an empty vertex shader object and return its ID
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // Give the shader its source code
    // vertexShader = the shader object
    // 1 = number of strings
    // &vertexShaderSource = pointer to the shader code
    // NULL = let OpenGL figure out string length
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);

    // Compile the shader code
    glCompileShader(vertexShader);

    // Check if compilation was successful
    int success;
    char infoLog[512];

    // Ask OpenGL for the compile status (success or fail)
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        // If it failed, get the error message
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::VERTEX SHADER COMPILATION FAILED\n" << infoLog << std::endl;
    }

    // Fragment Shader (same process)

    // Store ID for fragment shader
    unsigned int fragmentShader;

    // Create fragment shader object
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Give it source code
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    // Compile it
    glCompileShader(fragmentShader);

    // Check if compilation worked
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        // Get error message if it failed
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::FRAGMENT SHADER COMPILATION FAILED\n" << infoLog << std::endl;
    }

    // Shader Program

    // Create a shader program (this links shaders together)
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    // Attach both shaders to the program
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    // Link the shaders into one complete program
    glLinkProgram(shaderProgram);

    // Check if linking was successful
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success) {
        // Get error message if linking failed
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER PROGRAM LINKING FAILED\n" << infoLog << std::endl;
    }


    // After linking, the individual shaders are no longer needed
    // The program already has everything it needs
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = {
        // positions         // colors
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
       -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
        0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top
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
    //GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
    //GL_STATIC_DRAW: the data is set only once and used many times.
    //GL_DYNAMIC_DRAW: the data is changed a lot and used many times.

    //bind EBO and copy indicies into buffer like VBO
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //more info look at VBO
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    // Tell OpenGL how to read the vertex data from the VBO

    // 0 = location in the vertex shader (layout(location = 0))
    // 3 = number of values per vertex (x, y, z)
    // GL_FLOAT = type of each value
    // GL_FALSE = do NOT normalize the data
    // 3 * sizeof(float) = size of one vertex (3 floats total)
    // (void*)0 = start reading at the beginning of the data
    // IMPORTANT: this uses the VBO currently bound to GL_ARRAY_BUFFER
    //read points from vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);

    // Enable the vertex attribute at location 0 so OpenGL can use it
    glEnableVertexAttribArray(0);

    //read color from vertex data
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind the VBO (optional, just to avoid accidental changes later)
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // IMPORTANT:
    // Do NOT unbind the EBO while the VAO is still bound
    // The EBO is stored inside the VAO, so it needs to stay bound

    // Unbind the VAO so we don’t accidentally modify it later
    glBindVertexArray(0);

    //render loop
    while (!glfwWindowShouldClose(window)) {
        //input function called each frame
        processInput(window);

        //glClear uses this color to clear the screen (sets it doesnt have to be called always)
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //clear the buffer so you wont see previous frame (you have to specify which one)
        glClear(GL_COLOR_BUFFER_BIT);

        float timeValue = glfwGetTime();
        float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
        //you dont need the program to be called to find unifor but you do
        //need it to be called before updating it
        //int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
        //call the program to start using it
        //every shader and rendering call will use this program object and shaders
        glUseProgram(shaderProgram);
        //change color of vertex color through the unifor we made need to call progrm before changing it
        //glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

        //call the configuration
        glBindVertexArray(VAO);
        // Draw triangles using the indices stored in the currently bound EBO (GL_ELEMENT_ARRAY_BUFFER)
        // Normally, you'd have to bind the correct EBO for each object before calling glDrawElements.
        // However, VAOs remember which EBO was bound when the VAO was created.
        // So simply binding the VAO automatically binds the right EBO, making rendering easier.
        glDrawArrays(GL_TRIANGLES, 0, 3);


        glfwSwapBuffers(window); //swaps color buffer in window
        glfwPollEvents(); //processes events received in window and returns a response(if requested)
    }

    //de allocated resources after usage
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    //clean glfw resources;
    glfwTerminate();
    return 0;
}
