
#include "camera.hpp"
#include "constants.hpp"
#include "coordinates.hpp"
#include "gameData.hpp"
#include "logger.hpp"
#include "worldFactory.hpp"

#include "graphics/meshObject.hpp"
#include "graphics/renderer.hpp"
#include "graphics/shaderProgram.hpp"

#include "imgui/imguiWrapper.hpp"

#include "xbak/FileManager.h"
#include "xbak/FileBuffer.h"
#include "xbak/PaletteResource.h"

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
    //auto loc = gameData.mLocus.mPosition;
    //auto worldCenter = glm::vec3{loc.x, 1.6, loc.y};

    logger.Info() << "World Center: " << worldCenter << std::endl;

    /*auto systems = Systems{};
    for (const auto& world : worlds)
    {
        for (const auto& inst : world.GetItems())
        {
            systems.AddIntersectable(
                Intersectable{}
        }
    }*/

    auto objStore = Graphics::MeshObjectStorage{};

    for (auto& item : zoneItems.GetItems())
    {
        objStore.AddObject(
            item.GetName(),
            BAK::ZoneItemToMeshObject(item, textures, pal));
    }

    auto sphere = Sphere{30.0, 36, 18, true};
    objStore.AddObject(
        "combat",
        Graphics::SphereToMeshObject(sphere, glm::vec4{1.0, 0, 0, .7}));

    objStore.AddObject(
        "dialog",
        Graphics::SphereToMeshObject(sphere, glm::vec4{0.0, 1, 0, .7}));

    objStore.AddObject(
        "transition",
        Graphics::SphereToMeshObject(sphere, glm::vec4{1.0, 1, 0, .7}));

    objStore.AddObject(
        "town",
        Graphics::SphereToMeshObject(sphere, glm::vec4{1.0, 0, 1, .7}));

    objStore.AddObject(
        "grey",
        Graphics::SphereToMeshObject(sphere, glm::vec4{.7, .7, .7, .7}));


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
    
    float speed = 400 * 30.0f; // 3 units / second
    float turnSpeed = 30.0f; // 3 units / second

    Camera camera{width, height, worldCenter};
    camera.SetPosition(worldCenter);

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
            ss << "Pos: " << camera.GetPosition() 
                << "\nNPos: " << camera.GetNormalisedPosition()
                << "\nTile: " << glm::floor(camera.GetPosition() / glm::vec3{BAK::gTileSize});
            ImGui::Text(ss.str().c_str());
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f 
                / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        
        glBindVertexArray(VertexArrayID);

        currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;
        
        if (glfwGetKey( window, GLFW_KEY_W) == GLFW_PRESS)
            camera.MoveForward(deltaTime * speed);
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

        if (glfwGetKey( window, GLFW_KEY_P) == GLFW_PRESS)
            lightPos.y += .5;
        if (glfwGetKey( window, GLFW_KEY_L) == GLFW_PRESS)
            lightPos.y -= .5;

        lightPos.x = camera.GetNormalisedPosition().x;
        lightPos.z = camera.GetNormalisedPosition().z;
        
        // Update the camera position and light position
        glUniform3f(lightId, lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(
            cameraPositionId,
            camera.GetNormalisedPosition().x,
            camera.GetNormalisedPosition().y,
            camera.GetNormalisedPosition().z);

        viewMatrix = camera.GetViewMatrix();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const auto& world : worlds.GetTiles())
        {
            for (const auto& inst : world.GetItems())
            {
                const auto [offset, length] = objStore.GetObject(
                    inst.GetZoneItem().GetName());

                modelMatrix = glm::mat4(1.0f);
                
                auto relLoc = inst.GetLocation() / BAK::gWorldScale;
                auto scaleFactor = static_cast<float>(inst.GetZoneItem().GetScale());

                // Lower the ground a little - needed to fix issues with 
                // the road z-fighting
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
        
        for (const auto& world : worlds.GetTiles())
        {
            for (const auto& inst : world.GetEncounters())
            {
                const auto GetZoneItem = [&](auto encounterType){
                    switch (encounterType)
                    {
                    case BAK::EncounterType::Combat: return "combat";
                    case BAK::EncounterType::Dialog: return "dialog";
                    case BAK::EncounterType::Transition: return "transition";
                    case BAK::EncounterType::Town: return "town";
                    default: return "grey";
                    }
                };

                const auto [offset, length] = objStore.GetObject(GetZoneItem(inst.GetType()));

                modelMatrix = glm::mat4(1.0f);
                auto location = glm::vec3{
                    static_cast<float>(world.GetTile()[0]) * 64000 + (static_cast<float>(inst.GetOffset().x << 2)),
                    0.0f,
                    -(static_cast<float>(world.GetTile()[1]) * 64000 + (static_cast<float>(inst.GetOffset().y << 2)))};
                auto relLoc = location / BAK::gWorldScale;
                float scaleFactor = 1;//static_cast<float>(inst.GetZoneItem().GetScale());

                modelMatrix = glm::translate(modelMatrix, relLoc);
                modelMatrix = glm::scale(modelMatrix, glm::vec3{scaleFactor});

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

