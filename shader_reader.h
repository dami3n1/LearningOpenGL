#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> //for opengl headers

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    //program ID
    unsigned int ID;

    //constructor to read and build shader
    Shader(const char *vertexPath, const char *fragmentPath) {
        // retrieve the vertex/fragment source code from filepath
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        //ensure ifstream objects can throw exceptions
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            //open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            //read files buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            //close file handlers
            vShaderFile.close();
            fShaderFile.close();
            //convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        } catch (std::ifstream::failure &e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        const char *vShaderCode = vertexCode.c_str();
        const char *fShaderCode = fragmentCode.c_str();

        // This will store the ID (handle) for the vertex shader
        unsigned int vertexShader;
        // Store ID for fragment shader
        unsigned int fragmentShader;

        // Create an empty vertex shader object and return its ID
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        // Create fragment shader object
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        // Give the shader its source code
        // vertexShader = the shader object
        // 1 = number of strings
        // &vShaderCode = pointer to the shader code
        // NULL = let OpenGL figure out string length
        glShaderSource(vertexShader, 1, &vShaderCode, NULL);
        // Give it source code
        glShaderSource(fragmentShader, 1, &fShaderCode, NULL);

        // Compile the shader code
        glCompileShader(vertexShader);
        checkCompileErrors(vertexShader, "VERTEX");
        // Compile it
        glCompileShader(fragmentShader);
        checkCompileErrors(fragmentShader, "FRAGMENT");

        // Shader Program

        // Create a shader program (this links shaders together)
        ID = glCreateProgram();

        // Attach both shaders to the program
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragmentShader);

        // Link the shaders into one complete program
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // After linking, the individual shaders are no longer needed
        // The program already has everything it needs
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    // activate the shader
    // ------------------------------------------------------------------------
    void use()
    {
        glUseProgram(ID);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

#endif
