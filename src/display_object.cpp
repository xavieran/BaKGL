#include "logger.hpp"

#include "loadShaders.hpp"

#include "worldFactory.hpp"
#include "meshObject.hpp"

#include "FileManager.h"
#include "FileBuffer.h"
#include "PaletteResource.h"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/range/adaptor/indexed.hpp>

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("display_object");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);

    std::string zone = "Z01";

    auto palz = std::make_unique<PaletteResource>();
    std::stringstream palStr{""};
    palStr << zone << ".PAL";

    FileManager::GetInstance()->Load(palz.get(), palStr.str());
    auto& pal = *palz->GetPalette();

    auto zoneItems = BAK::ZoneItemStore{zone, pal};

    auto objStore = BAK::MeshObjectStorage{};

    for (const auto& item : zoneItems.GetItems())
    {
        auto obj = BAK::MeshObject();
        if (item.GetDatItem().mVertices.size() <= 1) continue;
        obj.LoadFromBaKItem(item, pal);
        objStore.AddObject(item.GetName(), obj);
    }

    float worldScale = 100.;

    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    GLFWwindow* window;

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    unsigned height = 768;
    unsigned width  = 960;

    window = glfwCreateWindow( width, height, "BaK", NULL, NULL);
    if( window == NULL )
    {
        logger.Log(Logging::LogLevel::Error) << "Failed to open GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        logger.Log(Logging::LogLevel::Error) << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders(vertexShader, fragmentShader);

    const auto& vertices = objStore.mVertices;
    const auto& normals = objStore.mNormals;
    const auto& colors = objStore.mColors;
    const auto& indices = objStore.mIndices;

    GLuint mvpMatrixID = glGetUniformLocation(programID, "MVP");
    GLuint modelMatrixID = glGetUniformLocation(programID, "M");
    GLuint viewMatrixID = glGetUniformLocation(programID, "V");

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(std::decay_t<decltype(vertices)>::value_type),
        &vertices.front(),
        GL_STATIC_DRAW);
    
    GLuint normalBuffer;
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        normals.size() * sizeof(std::decay_t<decltype(normals)>::value_type),
        &normals.front(),
        GL_STATIC_DRAW);

    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        colors.size() * sizeof(std::decay_t<decltype(colors)>::value_type),
        &colors.front(),
        GL_STATIC_DRAW);

    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(std::decay_t<decltype(indices)>::value_type),
        &indices.front(),
        GL_STATIC_DRAW);

    glm::mat4 projectionMatrix = glm::perspective(
        glm::radians(45.0f),
        (float) width / (float)height,
        0.1f,
        4000.0f);
      
    // Camera matrix
    glm::mat4 viewMatrix = glm::lookAt(
        glm::vec3(0,0,0), // eye pos
        glm::vec3(0,0,0), // look pos
        glm::vec3(0,1,0)  // orientation
        );
      
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    // Our ModelViewprojectionMatrix : multiplication of our 3 matrices
    glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix; 

    glm::vec3 position = glm::vec3( 0, 1.8, 0 );
    // horizontal angle : toward -Z
    float horizontalAngle = 3.14f;
    // vertical angle : 0, look at the horizon
    float verticalAngle = 0.0f;
    // Initial Field of View
    float initialFoV = 45.0f;
    
    float speed = 6.0f; 
    float mouseSpeed = 0.009f;
    glm::vec3 direction;
    glm::vec3 right;
    glm::vec3 up;

    double currentTime;
    double lastTime = 0;
    float deltaTime;

    glfwSetCursorPos(window, width/2, height/2);

    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    glm::vec3 lightPos = glm::vec3(0,220,0);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);  
    const auto& tex = zoneItems.GetTexture(0);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        tex.mWidth,
        tex.mHeight,
        0,
        GL_RGB,
        GL_FLOAT,
        tex.mTexture.data());
    glGenerateMipmap(GL_TEXTURE_2D);


    do
    {
        currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;
        
        direction = {
            cos(verticalAngle) * sin(horizontalAngle),
            sin(verticalAngle),
            cos(verticalAngle) * cos(horizontalAngle)
        };

        right = {
            sin(horizontalAngle - 3.14f/2.0f),
            0,
            cos(horizontalAngle - 3.14f/2.0f)
        };

        up = glm::cross(right, direction);

        if (glfwGetKey( window, GLFW_KEY_W) == GLFW_PRESS){
            position += direction * deltaTime * speed;
            logger.Info() << "Pos: " << glm::to_string(position) << std::endl;
        }
        // Move backward
        if (glfwGetKey( window, GLFW_KEY_S) == GLFW_PRESS){
        position -= direction * deltaTime * speed;
        }
        // Strafe right
        if (glfwGetKey( window, GLFW_KEY_D) == GLFW_PRESS){
            position += right * deltaTime * speed;
        }
        // Strafe left
        if (glfwGetKey( window, GLFW_KEY_A) == GLFW_PRESS){
            position -= right * deltaTime * speed;
        }

        // Rotate left
        if (glfwGetKey( window, GLFW_KEY_Q) == GLFW_PRESS){
            horizontalAngle += deltaTime * (speed);
        }
        // Rotate left
        if (glfwGetKey( window, GLFW_KEY_E) == GLFW_PRESS){
            horizontalAngle -= deltaTime * (speed);
        }

        if (glfwGetKey( window, GLFW_KEY_X) == GLFW_PRESS){
            verticalAngle += deltaTime * (speed);
        }

        if (glfwGetKey( window, GLFW_KEY_Y) == GLFW_PRESS){
            verticalAngle -= deltaTime * (speed);
        }

        if (glfwGetKey( window, GLFW_KEY_P) == GLFW_PRESS){
            lightPos.y += .5;
        }
        if (glfwGetKey( window, GLFW_KEY_L) == GLFW_PRESS){
            lightPos.y -= .5;
        }

        lightPos.x = position.x;
        lightPos.z = position.z;

        viewMatrix = glm::lookAt(
            position,
            position + direction,
            up);

        //glfwSetCursorPos(window, width/2, height/2);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(
            0,
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glVertexAttribPointer(
            1,
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        glVertexAttribPointer(
            2,
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );
        
        const auto [offset, length] = objStore.GetObject(argv[1]);

        modelMatrix = glm::mat4(1.0f);
        
        //modelMatrix = glm::rotate(modelMatrix, , glm::vec3(0,-1,0));

        MVP = projectionMatrix * viewMatrix * modelMatrix;

        glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, glm::value_ptr(MVP));
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, glm::value_ptr(viewMatrix));

        glDrawElementsBaseVertex(
            GL_TRIANGLES,
            length,
            GL_UNSIGNED_INT,
            (void*) (offset * sizeof(GLuint)),
            offset
        );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &normalBuffer);
    glDeleteBuffers(1, &colorBuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
