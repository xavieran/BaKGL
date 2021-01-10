#include "logger.hpp"

#include "loadShaders.hpp"

#include "meshObject.hpp"
#include "renderer.hpp"
#include "worldFactory.hpp"

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
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);

    std::string zone = argv[1];

    auto palz = std::make_unique<PaletteResource>();
    std::stringstream palStr{""};
    palStr << zone << ".PAL";
    FileManager::GetInstance()->Load(palz.get(), palStr.str());
    auto& pal = *palz->GetPalette();

    auto textures  = BAK::TextureStore{zone, pal};
    auto zoneItems = BAK::ZoneItemStore{zone, textures};
    auto worlds    = BAK::WorldTileStore{zoneItems};
    auto worldCenter = worlds.GetTiles().at(0).mCenter;

    auto objStore = BAK::MeshObjectStorage{};

    for (const auto& item : zoneItems.GetItems())
    {
        auto obj = BAK::MeshObject{};
        if (item.GetDatItem().mVertices.size() <= 1) continue;
        obj.LoadFromBaKItem(item, textures, pal);
        objStore.AddObject(item.GetName(), obj);
    }

    // FIXME: Need to make this consistent, floaing around a few other places
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

    unsigned height = 600;
    unsigned width  = 800;

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

    const auto& vertices      = objStore.mVertices;
    const auto& normals       = objStore.mNormals;
    const auto& colors        = objStore.mColors;
    const auto& textureCoords = objStore.mTextureCoords;
    const auto& textureBlends = objStore.mTextureBlends;
    const auto& indices       = objStore.mIndices;

    BAK::GLBuffers buffers{};
    buffers.LoadBufferDataGL(buffers.mVertexBuffer, GL_ARRAY_BUFFER, vertices);
    buffers.LoadBufferDataGL(buffers.mNormalBuffer, GL_ARRAY_BUFFER, normals);
    buffers.LoadBufferDataGL(buffers.mColorBuffer, GL_ARRAY_BUFFER, colors);
    buffers.LoadBufferDataGL(buffers.mTextureCoordBuffer, GL_ARRAY_BUFFER, textureCoords);
    buffers.LoadBufferDataGL(buffers.mTextureBlendBuffer, GL_ARRAY_BUFFER, textureBlends);
    buffers.LoadBufferDataGL(buffers.mElementBuffer, GL_ELEMENT_ARRAY_BUFFER, indices);

    BAK::TextureBuffer textureBuffer{};
    textureBuffer.LoadTexturesGL(textures);

    GLuint textureID     = glGetUniformLocation(programID, "texture0");
    GLuint mvpMatrixID   = glGetUniformLocation(programID, "MVP");
    GLuint modelMatrixID = glGetUniformLocation(programID, "M");
    GLuint viewMatrixID  = glGetUniformLocation(programID, "V");

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

    glm::vec3 position = glm::vec3( 0, 1.2, 0 );
    glm::vec3 lightPos = glm::vec3(0,220,0);
    // horizontal angle : toward -Z
    float horizontalAngle = 3.14f;
    // vertical angle : 0, look at the horizon
    float verticalAngle = 0.0f;
    // Initial Field of View
    float initialFoV = 45.0f;
    
    //float speed = 120.0f; // 3 units / second
    float speed = 3 * 30.0f; // 3 units / second
    float turnSpeed = 10.0f; // 3 units / second
    float mouseSpeed = 0.009f;
    //double xpos, ypos;
    glm::vec3 direction;
    glm::vec3 right;
    glm::vec3 up;
    double currentTime;
    double lastTime = 0;
    float deltaTime;

    GLuint LightID = glGetUniformLocation(programID, "lightPosition_worldspace");
    GLuint CameraPositionID = glGetUniformLocation(programID, "CameraPosition_worldspace");

    glfwSetCursorPos(window, width/2, height/2);

    glUseProgram(programID);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Currently just using discard. Will swap to proper transparency if
    // discard costs too much.
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    buffers.BindArraysGL();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureBuffer.mTextureBuffer);
    glUniform1i(textureID, 0);

    do
    {
        //glfwGetCursorPos(window, &xpos, &ypos);

        currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;
        
        //horizontalAngle += mouseSpeed * deltaTime * float(width/2 - xpos );
        //verticalAngle   += mouseSpeed * deltaTime * float(height/2 - ypos );
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
            horizontalAngle += deltaTime * (turnSpeed / 12);
        }
        // Rotate left
        if (glfwGetKey( window, GLFW_KEY_E) == GLFW_PRESS){
            horizontalAngle -= deltaTime * (turnSpeed / 12);
        }

        if (glfwGetKey( window, GLFW_KEY_X) == GLFW_PRESS){
            verticalAngle += deltaTime * (turnSpeed / 12);
        }

        if (glfwGetKey( window, GLFW_KEY_Y) == GLFW_PRESS){
            verticalAngle -= deltaTime * (turnSpeed / 12);
        }

        if (glfwGetKey( window, GLFW_KEY_P) == GLFW_PRESS){
            lightPos.y += .5;
        }
        if (glfwGetKey( window, GLFW_KEY_L) == GLFW_PRESS){
            lightPos.y -= .5;
        }

        lightPos.x = position.x;
        lightPos.z = position.z;
        
        // Update the camera position and light position
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(CameraPositionID, position.x, position.y, position.z);

        viewMatrix = glm::lookAt(
            position,
            position + direction,
            up);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const auto& world : worlds.GetTiles())
        {
            for (const auto& inst : world.mItemInsts)
            {
                if (inst.GetZoneItem().GetDatItem().mVertices.size() <= 1) continue;

                const auto [offset, length] = objStore.GetObject(inst.GetZoneItem().GetName());

                modelMatrix = glm::mat4(1.0f);
                
                auto scaleFactor = static_cast<float>(
                    inst.GetZoneItem().GetDatItem().mScale);

                auto instLoc = inst.GetLocation();
                auto itemLoc = glm::vec3{instLoc.x, 0, instLoc.y};
                auto relLoc = (itemLoc - worldCenter) / worldScale;

                if (inst.GetZoneItem().GetName() == "ground")
                {
                    modelMatrix = glm::translate(modelMatrix, glm::vec3{0,-.1,0});
                }
                
                /*
                if (inst.GetZoneItem().GetName().substr(0,4) == "tree")
                {
                    continue;
                }
                */
                modelMatrix = glm::translate(modelMatrix, relLoc);
                modelMatrix = glm::scale(modelMatrix, glm::vec3{scaleFactor});
                modelMatrix = glm::rotate(modelMatrix, inst.GetRotation().z, glm::vec3(0,-1,0));

                MVP = projectionMatrix * viewMatrix * modelMatrix;

                glUniformMatrix4fv(mvpMatrixID,   1, GL_FALSE, glm::value_ptr(MVP));
                glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(modelMatrix));
                glUniformMatrix4fv(viewMatrixID,  1, GL_FALSE, glm::value_ptr(viewMatrix));

                glDrawElementsBaseVertex(
                    GL_TRIANGLES,
                    length,
                    GL_UNSIGNED_INT,
                    (void*) (offset * sizeof(GLuint)),
                    offset
                );
            }
        }

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window) == 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);

    // Cleanup VBO
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

