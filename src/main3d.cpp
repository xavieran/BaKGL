#include "constants.hpp"
#include "coordinates.hpp"
#include "gameData.hpp"
#include "geometry.hpp"

#include "logger.hpp"

#include "meshObject.hpp"
#include "renderer.hpp"
#include "shaderProgram.hpp"
#include "worldFactory.hpp"

#include "FileManager.h"
#include "FileBuffer.h"
#include "PaletteResource.h"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include "glm.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/range/adaptor/indexed.hpp>

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    //Logging::LogState::Disable("MeshObjectStore");
    //Logging::LogState::Disable("WorldTileStore");
    
    std::string saveFile{argv[1]};

    logger.Info() << "Loading save:" << saveFile << std::endl;

    /*auto fb = FileBufferFactory::CreateFileBuffer(saveFile);
    BAK::GameData gameData(fb);

    std::stringstream zoneSS{""};
    zoneSS << "Z" << std::setw(2) << std::setfill('0')<< gameData.mZone;
    std::string zone = zoneSS.str();*/

    BAK::ZoneLabel zoneLabel{argv[1]};

    auto palz = std::make_unique<PaletteResource>();
    FileManager::GetInstance()->Load(palz.get(), zoneLabel.GetPalette());
    auto& pal = *palz->GetPalette();

    auto textures    = BAK::TextureStore{zoneLabel, pal};
    auto zoneItems   = BAK::ZoneItemStore{zoneLabel, textures};
    auto worlds      = BAK::WorldTileStore{zoneItems};
    auto worldCenter = worlds.GetTiles().front().GetCenter();
    //auto worldCenter = glm::vec3{978140, 0, -973103};
    //auto loc = gameData.mLocus.mPosition;
    //auto worldCenter = glm::vec3{loc.x, 1.6, loc.y};

    logger.Info() << "World Center: " << worldCenter << std::endl;

    auto objStore = BAK::MeshObjectStorage{};

    for (auto& item : zoneItems.GetItems())
    {
        auto obj = BAK::MeshObject{};
        if (item.GetVertices().size() <= 1) continue;
        obj.LoadFromBaKItem(item, textures, pal);
        objStore.AddObject(item.GetName(), obj);
    }

    // FIXME: Need to make this consistent, floaing around a few other places

    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    GLFWwindow* window;

    const unsigned antiAliasingSamples = 4;
    glfwWindowHint(GLFW_SAMPLES, antiAliasingSamples);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    unsigned height = 800;
    unsigned width  = 1400;

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
        logger.Log(Logging::LogLevel::Error) 
            << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.15f, 0.31f, 0.36f, 0.0f);

    auto shaderProgram = ShaderProgram{
        "vertex.glsl",
        //"geometry.glsl",
        "fragment.glsl"};

    auto programId = shaderProgram.Compile();

    const auto& vertices      = objStore.mVertices;
    const auto& normals       = objStore.mNormals;
    const auto& colors        = objStore.mColors;
    const auto& textureCoords = objStore.mTextureCoords;
    const auto& textureBlends = objStore.mTextureBlends;
    const auto& indices       = objStore.mIndices;

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    BAK::GLBuffers buffers{};
    buffers.AddBuffer("vertex", 0, 3);
    buffers.AddBuffer("normal", 1, 3);
    buffers.AddBuffer("color", 2, 4);
    buffers.AddBuffer("textureCoord", 3, 3);
    buffers.AddBuffer("textureBlend", 4, 1);

    buffers.LoadBufferDataGL("vertex", GL_ARRAY_BUFFER, vertices);
    buffers.LoadBufferDataGL("normal", GL_ARRAY_BUFFER, normals);
    buffers.LoadBufferDataGL("color", GL_ARRAY_BUFFER, colors);
    buffers.LoadBufferDataGL("textureCoord", GL_ARRAY_BUFFER, textureCoords);
    buffers.LoadBufferDataGL("textureBlend", GL_ARRAY_BUFFER, textureBlends);

    buffers.LoadBufferDataGL(buffers.mElementBuffer, GL_ELEMENT_ARRAY_BUFFER, indices);
    buffers.BindArraysGL();
    glBindVertexArray(0);

    BAK::TextureBuffer textureBuffer{};
    textureBuffer.LoadTexturesGL(textures);

    GLuint textureID     = glGetUniformLocation(programId, "texture0");
    GLuint mvpMatrixID   = glGetUniformLocation(programId, "MVP");
    GLuint modelMatrixID = glGetUniformLocation(programId, "M");
    GLuint viewMatrixID  = glGetUniformLocation(programId, "V");

    glm::mat4 projectionMatrix = glm::perspective(
        glm::radians(50.0f),
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
    float speed = 6 * 30.0f; // 3 units / second
    float turnSpeed = 30.0f; // 3 units / second
    float mouseSpeed = 0.009f;
    //double xpos, ypos;
    glm::vec3 direction;
    glm::vec3 right;
    glm::vec3 up;
    double currentTime;
    double lastTime = 0;
    float deltaTime;

    GLuint LightID = glGetUniformLocation(programId, "lightPosition_worldspace");
    GLuint CameraPositionID = glGetUniformLocation(programId, "cameraPosition_worldspace");

    glfwSetCursorPos(window, width/2, height/2);

    glUseProgram(programId);


    glEnable(GL_MULTISAMPLE);  

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Currently just using discard. Will swap to proper transparency if
    // discard costs too much.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureBuffer.mTextureBuffer);
    glUniform1i(textureID, 0);

    do
    {
        glBindVertexArray(VertexArrayID);

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
            logger.Info() << "Pos: " << position << std::endl;
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
            for (const auto& inst : world.GetItems())
            {
                if (inst.GetZoneItem().GetVertices().size() <= 1)
                {
                    continue;
                }

                const auto [offset, length] = objStore.GetObject(inst.GetZoneItem().GetName());

                modelMatrix = glm::mat4(1.0f);
                
                auto scaleFactor = static_cast<float>(
                    inst.GetZoneItem().GetScale());

                auto itemLoc = inst.GetLocation();
                auto relLoc = (itemLoc - worldCenter) / BAK::gWorldScale;

                if (inst.GetZoneItem().GetName() == "ground")
                {
                    // Lower the ground a little - need to fix issues with objects
                    // being rendered on top of each other...
                    modelMatrix = glm::translate(modelMatrix, glm::vec3{0,-.2,0});
                }
                
                modelMatrix = glm::translate(modelMatrix, relLoc);
                modelMatrix = glm::scale(modelMatrix, glm::vec3{scaleFactor});
                modelMatrix = glm::rotate(modelMatrix, inst.GetRotation().y, glm::vec3(0,1,0));

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
    glDeleteProgram(programId);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

