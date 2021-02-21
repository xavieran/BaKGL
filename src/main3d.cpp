
#include "camera.hpp"
#include "constants.hpp"
#include "coordinates.hpp"
#include "gameData.hpp"
#include "imguiWrapper.hpp"

#include "logger.hpp"

#include "meshObject.hpp"
#include "renderer.hpp"
#include "shaderProgram.hpp"
#include "worldFactory.hpp"

#include "FileManager.h"
#include "FileBuffer.h"
#include "PaletteResource.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
        obj.LoadFromBaKItem(item, textures, pal);
        objStore.AddObject(item.GetName(), obj);
    }

    if( !glfwInit() )
    {
        logger.Error() << "Failed to initialize GLFW" << std::endl;
        std::exit(1);
    }

    GLFWwindow* window;

    const unsigned antiAliasingSamples = 4;
    glfwWindowHint(GLFW_SAMPLES, antiAliasingSamples);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    unsigned height = 800;
    unsigned width  = 1400;
    /*unsigned height = 1600;
    unsigned width  = 2400;*/

    window = glfwCreateWindow(width, height, "BaK", NULL, NULL);
    if( window == NULL )
    {
        logger.Log(Logging::LogLevel::Error) << "Failed to open GLFW window" << std::endl;
        glfwTerminate();
        std::exit(1);
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
    
    ImguiWrapper::Initialise(window);
    
    // Dark blue background
    glClearColor(0.15f, 0.31f, 0.36f, 0.0f);

    auto shaderProgram = ShaderProgram{
        "vertex.glsl",
        //"geometry.glsl",
        "fragment.glsl"};

    auto programId = shaderProgram.Compile();

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    BAK::GLBuffers buffers{};
    buffers.AddBuffer("vertex", 0, 3);
    buffers.AddBuffer("normal", 1, 3);
    buffers.AddBuffer("color", 2, 4);
    buffers.AddBuffer("textureCoord", 3, 3);
    buffers.AddBuffer("textureBlend", 4, 1);

    buffers.LoadBufferDataGL("vertex", GL_ARRAY_BUFFER, objStore.mVertices);
    buffers.LoadBufferDataGL("normal", GL_ARRAY_BUFFER, objStore.mNormals);
    buffers.LoadBufferDataGL("color", GL_ARRAY_BUFFER, objStore.mColors);
    buffers.LoadBufferDataGL("textureCoord", GL_ARRAY_BUFFER, objStore.mTextureCoords);
    buffers.LoadBufferDataGL("textureBlend", GL_ARRAY_BUFFER, objStore.mTextureBlends);

    buffers.LoadBufferDataGL(buffers.mElementBuffer, GL_ELEMENT_ARRAY_BUFFER, objStore.mIndices);
    buffers.BindArraysGL();
    glBindVertexArray(0);

    BAK::TextureBuffer textureBuffer{};
    textureBuffer.LoadTexturesGL(textures);

    GLuint textureID     = glGetUniformLocation(programId, "texture0");
    GLuint mvpMatrixID   = glGetUniformLocation(programId, "MVP");
    GLuint modelMatrixID = glGetUniformLocation(programId, "M");
    GLuint viewMatrixID  = glGetUniformLocation(programId, "V");

    glm::mat4 viewMatrix{1};
    glm::mat4 modelMatrix{1.0f};
    glm::mat4 MVP{0};

    glm::vec3 lightPos = glm::vec3(0,220,0);
    
    float speed = 6 * 30.0f; // 3 units / second
    float turnSpeed = 30.0f; // 3 units / second

    Camera camera{width, height};

    double currentTime;
    double lastTime = 0;
    float deltaTime;

    GLuint lightId = glGetUniformLocation(programId, "lightPosition_worldspace");
    GLuint cameraPositionId = glGetUniformLocation(programId, "cameraPosition_worldspace");

    glfwSetCursorPos(window, width/2, height/2);

    glUseProgram(programId);

    glEnable(GL_MULTISAMPLE);  

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureBuffer.mTextureBuffer);
    glUniform1i(textureID, 0);

    do
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("Info");
            std::stringstream ss{};
            ss << "Pos: " << camera.GetPosition();
            ImGui::Text(ss.str().c_str());
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f 
                / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        
        glBindVertexArray(VertexArrayID);

        currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;
        
        if (glfwGetKey( window, GLFW_KEY_W) == GLFW_PRESS){
            camera.MoveForward(deltaTime * speed);
            logger.Info() << "Pos: " << camera.GetPosition() << std::endl;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.MoveForward(-deltaTime * speed);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.StrafeRight(deltaTime * speed);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.StrafeRight(-deltaTime * speed);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera.RotateLeft(deltaTime * (turnSpeed / 12));
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera.RotateLeft(-deltaTime * (turnSpeed / 12));
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            camera.RotateVertical(deltaTime * (turnSpeed / 12));
        if (glfwGetKey( window, GLFW_KEY_Y) == GLFW_PRESS)
            camera.RotateVertical(-deltaTime * (turnSpeed / 12));

        if (glfwGetKey( window, GLFW_KEY_P) == GLFW_PRESS){
            lightPos.y += .5;
        }
        if (glfwGetKey( window, GLFW_KEY_L) == GLFW_PRESS){
            lightPos.y -= .5;
        }

        lightPos.x = camera.GetPosition().x;
        lightPos.z = camera.GetPosition().z;
        
        // Update the camera position and light position
        glUniform3f(lightId, lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(
            cameraPositionId,
            camera.GetPosition().x,
            camera.GetPosition().y,
            camera.GetPosition().z);

        viewMatrix = camera.GetViewMatrix();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const auto& world : worlds.GetTiles())
        {
            for (const auto& inst : world.GetItems())
            {
                const auto [offset, length] = objStore.GetObject(inst.GetZoneItem().GetName());
                modelMatrix = glm::mat4(1.0f);
                
                auto itemLoc = inst.GetLocation();
                auto relLoc = (itemLoc - worldCenter) / BAK::gWorldScale;
                auto scaleFactor = static_cast<float>(inst.GetZoneItem().GetScale());

                // Lower the ground a little - need to fix issues with objects
                // being rendered on top of each other...
                if (inst.GetZoneItem().GetName() == "ground")
                    modelMatrix = glm::translate(modelMatrix, glm::vec3{0,-.5,0});
                
                modelMatrix = glm::translate(modelMatrix, relLoc);
                modelMatrix = glm::scale(modelMatrix, glm::vec3{scaleFactor});
                modelMatrix = glm::rotate(modelMatrix, inst.GetRotation().y, glm::vec3(0,1,0));

                MVP = camera.GetProjectionMatrix() * viewMatrix * modelMatrix;

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
        
        ImguiWrapper::Draw(window);
        
        // Swap buffers
        glfwSwapBuffers(window);
    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window) == 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);

    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programId);

    ImguiWrapper::Shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

