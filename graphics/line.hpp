#pragma once

#include <GL/glew.h>

#include "graphics/glm.hpp"

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
    Line(glm::vec3 start, glm::vec3 end);

    void setMVP(glm::mat4 mvp);
    void setColor(glm::vec3 color);
	void setLine(glm::vec3 start, glm::vec3 end);
	void updateLine(glm::vec3 start, glm::vec3 end);
    void draw();

    ~Line();
};

}
