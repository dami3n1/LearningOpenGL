#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> //for opengl headers
#include <glm/glm.hpp>
#include "logger.h"

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
            logger(ERROR, "Shader file not successfully read: " + std::string(e.what()));
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
    void use() const
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
    // ------------------------------------------------------------------------
    void setVec2(const std::string &name, const glm::vec2 &value) const
    { 
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec2(const std::string &name, float x, float y) const
    { 
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string &name, const glm::vec3 &value) const
    { 
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec3(const std::string &name, float x, float y, float z) const
    { 
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string &name, const glm::vec4 &value) const
    { 
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec4(const std::string &name, float x, float y, float z, float w) const
    { 
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string &name, const glm::mat2 &mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string &name, const glm::mat3 &mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
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
                logger(ERROR, "Shader compilation error of type: " + type + "\n" + std::string(infoLog));
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                logger(ERROR, "Program linking error of type: " + type + "\n" + std::string(infoLog));
            }
        }
    }
};

#endif
