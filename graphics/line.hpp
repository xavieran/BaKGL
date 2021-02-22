#pragma once

#include <GL/glew.h>

#include "glm.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

namespace Graphics {

class Line {
    int shaderProgram;
    unsigned int VBO, VAO;
    std::vector<float> vertices;
    glm::vec3 startPoint;
    glm::vec3 endPoint;
    glm::mat4 MVP;
    glm::vec3 lineColor;
public:
    Line(glm::vec3 start, glm::vec3 end) {

        startPoint = start;
        endPoint = end;
        lineColor = glm::vec3(1,0,0);

        const char *vertexShaderSource = "#version 330 core\n"
            "layout (location = 0) in vec3 aPos;\n"
            "uniform mat4 MVP;\n"
            "void main()\n"
            "{\n"
            "   gl_Position = MVP * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
            "}\0";
        const char *fragmentShaderSource = "#version 330 core\n"
            "out vec4 FragColor;\n"
            "uniform vec3 color;\n"
            "void main()\n"
            "{\n"
            "   FragColor = vec4(color, 1.0f);\n"
            "}\n\0";

        // vertex shader
        int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors

        // fragment shader
        int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors

        // link shaders
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        // check for linking errors

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

		glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
		
		setLine(start, end);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glBindVertexArray(0); 

    }

    void setMVP(glm::mat4 mvp) {
        MVP = mvp;
    }

    void setColor(glm::vec3 color) {
        lineColor = color;
    }

	void setLine(glm::vec3 start, glm::vec3 end)
	{
		vertices = {
             startPoint.x, startPoint.y, startPoint.z,
             end.x, end.y, end.z,
        };
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_DYNAMIC_DRAW);
	}


	void updateLine(glm::vec3 start, glm::vec3 end)
	{
		vertices = {
             start.x, start.y, start.z,
             end.x, end.y, end.z,
        };
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices.front());
	}

    void draw() {
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
        glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, &lineColor[0]);

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, 2);
    }

    ~Line() {

        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteProgram(shaderProgram);
    }
};

}
