#include <iostream>
#include <ostream>
#include <glad/glad.h> //glad should always be put first to prevent redefinition use of OpenGL
#include <GLFW/glfw3.h>

const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}";

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

    unsigned int vertexShader; // ID for Vertexxshader
    //creates empty shader object and stores it in variable(like an ID)
    vertexShader = glCreateShader(GL_VERTEX_SHADER); // define that you want a use a vertex shader
    //takes shader object you created(vertex shader) and copies the data in pointer (&vertexShaderSource)
    //Basically how to read shader code and put it in ID
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    //takes the object(ID) and compiles it
    glCompileShader(vertexShader);

    //Check if Shader Compilation was false with glGetShaderiv(object to check, what to check in the object, where to store the returned data)
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        //retrueve error
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


    //same as vertex shader but now for fragment shader
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    //Check if Shader Compilation was false with glGetShaderiv(object to check, what to check in the object, where to store the returned data)
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        //retrueve error
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    //After creating shaders you have to compile them into a shader program
    unsigned int shaderProgram;
    //creates shader program and returns ID to shaderprogram
    shaderProgram = glCreateProgram();

    //attach shaders and link to program
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        //retrueve error
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    //you can now delete the shaders so it wont be sitting in memory and get best performance
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = {
        0.5f, 0.5f, 0.0f, // top right
        0.5f, -0.5f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f // top left
    };

    unsigned int indices[] = {
        // note that we start from 0!
        0, 1, 3, // first Triangle
        1, 2, 3 // second Triangle
    };

    //VBO stores vertex data (data)
    //VAO tells how to use the data (config)
    //stores the buffer(s)
    // VBO or VAO is the "ID" for the buffer because it holds the location of the buffer
    unsigned int VBO, VAO, EBO;
    //makes 1 buffer and stores it in VBO or VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    //bind the vertex array object so opengl knows where to store config
    glBindVertexArray(VAO);
    //Bind the VBO to GL_ARRAY_BUFFER which is the buffer where vertex data is transfered
    //this also makes the following GL_ARRAY_BUFFER calls to configure the data from VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //allocates and stores data of buffer also tells it where to copy from and how to read it
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
    //GL_STATIC_DRAW: the data is set only once and used many times.
    //GL_DYNAMIC_DRAW: the data is changed a lot and used many times.

    //bind EBO and copy indicies into buffer like VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //more info look at VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    //tell openGL how to read and interpret the Vertex attributes from our vertex data
    //where the location of the position vertex attribute in vertex shader is( (location = 0) )
    //how many attributes per vertex
    //the type of data in each component
    //if data should be normalized
    //how big each vertex is (3 floats/3 coordinates)
    //add an offset of where data should start being read
    //glVertexAttribPointer uses the VBO that is bound with GL_ARRAY_BUFFER and it reads from that
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    //since our vertex attribute is in location 0 we give it location 0
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.

    //OpenGL is a state machine this has been set in memory and will stay until something else changes it
    // we unbind it so we dont accidentally mess with it if were trying to change another buffer or VBO/VAO
    glBindVertexArray(0);

    //render loop
    while (!glfwWindowShouldClose(window)) {
        //input function called each frame
        processInput(window);

        //glClear uses this color to clear the screen (sets it doesnt have to be called always)
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //clear the buffer so you wont see previous frame (you have to specify which one)
        glClear(GL_COLOR_BUFFER_BIT);

        //call the program to start using it
        //every shader and rendering call will use this program object and shaders
        glUseProgram(shaderProgram);
        //call the configuration
        glBindVertexArray(VAO);
        // Draw triangles using the indices stored in the currently bound EBO (GL_ELEMENT_ARRAY_BUFFER)
        // Normally, you'd have to bind the correct EBO for each object before calling glDrawElements.
        // However, VAOs remember which EBO was bound when the VAO was created.
        // So simply binding the VAO automatically binds the right EBO, making rendering easier.
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
