#include "bak/camera.hpp"
#include "bak/constants.hpp"
#include "bak/container.hpp"
#include "bak/coordinates.hpp"
#include "bak/fixedObject.hpp"
#include "bak/gameData.hpp"
#include "graphics/inputHandler.hpp"
#include "com/logger.hpp"
#include "bak/screens.hpp"
#include "bak/systems.hpp"
#include "bak/encounter/town.hpp"
#include "bak/encounter/block.hpp"
#include "bak/encounter/zone.hpp"
#include "bak/worldFactory.hpp"
#include "bak/zone.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/cube.hpp"
#include "graphics/guiRenderer.hpp"
#include "graphics/glfw.hpp"
#include "graphics/line.hpp"
#include "graphics/meshObject.hpp"
#include "graphics/renderer.hpp"
#include "graphics/shaderProgram.hpp"
#include "graphics/sprites.hpp"
#include "graphics/texture.hpp"

#include "gui/guiManager.hpp"
#include "gui/window.hpp"

#include "imgui/imguiWrapper.hpp"

#include "xbak/FileManager.h"
#include "xbak/FileBuffer.h"
#include "xbak/PaletteResource.h"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <functional>
#include <memory>
#include <sstream>

#include <getopt.h>

int main(int argc, char** argv)
{
    struct option options[] = {
        {"save",required_argument,0,'s'},
        {"zone",required_argument,0,'z'}
    };
    int optionIndex = 0;

    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);

    BAK::ZoneLabel zoneLabel{};
    glm::vec<3, float> startPosition{0.0f, 0.0f, 0.0f};
    auto containers = std::vector<BAK::Container>{};
    
    BAK::GameData* gameData{nullptr};

    int opt;
    while ((opt = getopt_long (argc, argv, "s:z:", options, &optionIndex)) != -1)
    {   
        if (opt == 's')
        {
            logger.Info() << "Loading save file: " << optarg << std::endl;

            std::string saveFile = optarg;
            gameData = new BAK::GameData(saveFile);
            containers = gameData->LoadContainer();

            std::stringstream ss{};
            ss << "Z" << std::setw(2) << std::setfill('0') << gameData->mLocation.mZone;
            zoneLabel = BAK::ZoneLabel{ss.str()};
            startPosition = BAK::ToGlCoord<float>(gameData->mLocation.mLocation.mPosition);
        }
        else if (opt == 'z')
        {
            logger.Info() << "Loading zone: " << optarg << std::endl;
            auto zone = std::string{optarg};
            zoneLabel = BAK::ZoneLabel{zone};
        }
    }

    auto fixedObjects = BAK::LoadFixedObjects(zoneLabel.GetZoneNumber());

    BAK::DialogStore dialogStore{};
    BAK::DialogIndex dialogIndex{};

    auto palz = std::make_unique<PaletteResource>();
    FileManager::GetInstance()->Load(palz.get(), zoneLabel.GetPalette());
    auto& pal = *palz->GetPalette();

    auto textureStore = BAK::ZoneTextureStore{zoneLabel, pal};
    auto zoneItems   = BAK::ZoneItemStore{zoneLabel, textureStore};
    auto worlds      = BAK::WorldTileStore{zoneItems};

    if (startPosition == glm::vec<3, float>{0,0,0})
        startPosition = worlds.GetTiles().front().GetCenter();

    startPosition.y = 100;

    auto objStore = Graphics::MeshObjectStorage{};
    for (auto& item : zoneItems.GetItems())
        objStore.AddObject(
            item.GetName(),
            BAK::ZoneItemToMeshObject(item, textureStore, pal));

    const auto cube = Graphics::Cuboid{1, 1, 50};
    objStore.AddObject("combat", cube.ToMeshObject(glm::vec4{1.0, 0, 0, .7}));
    objStore.AddObject("trap", cube.ToMeshObject(glm::vec4{.8, 0, 0, .7}));
    objStore.AddObject("dialog", cube.ToMeshObject(glm::vec4{0.0, 1, 0, .7}));
    //objStore.AddObject("sound", cube.ToMeshObject(glm::vec4{1.0, .5, .5, .7}));
    objStore.AddObject("zone", cube.ToMeshObject(glm::vec4{1.0, 1, 0, .7}));
    objStore.AddObject("town", cube.ToMeshObject(glm::vec4{1.0, 0, 1, .7}));
    objStore.AddObject("background", cube.ToMeshObject(glm::vec4{.7, .7, .7, .7}));
    //objStore.AddObject("comment", cube.ToMeshObject(glm::vec4{.7, .7, .7, .7}));
    //objStore.AddObject("health", cube.ToMeshObject(glm::vec4{.7, .7, .7, .7}));
    objStore.AddObject("enable", cube.ToMeshObject(glm::vec4{.7, .7, .7, .7}));
    objStore.AddObject("disable", cube.ToMeshObject(glm::vec4{.7, .7, .7, .7}));
    objStore.AddObject("block", cube.ToMeshObject(glm::vec4{0,0,0, .7}));

    auto clickable = Sphere{1.0, 12, 6, true};
    objStore.AddObject(
        "clickable",
        Graphics::SphereToMeshObject(clickable, glm::vec4{.0, .0, 1.0, .7}));

    auto systems = Systems{};
    std::unordered_map<unsigned, const BAK::Encounter::Encounter*> encounters{};
    std::unordered_map<unsigned, const BAK::WorldItemInstance*> clickables{};

    for (const auto& world : worlds.GetTiles())
    {
        for (const auto& item : world.GetItems())
        {
            if (item.GetZoneItem().GetVertices().size() > 1)
            {
                auto id = systems.GetNextItemId();
                auto renderable = Renderable{
                    id,
                    objStore.GetObject(item.GetZoneItem().GetName()),
                    item.GetLocation(),
                    item.GetRotation(),
                    glm::vec3{item.GetZoneItem().GetScale()}};

                if (item.GetZoneItem().IsSprite())
                    systems.AddSprite(renderable);
                else
                    systems.AddRenderable(renderable);

                if (item.GetZoneItem().GetClickable())
                {
                    systems.AddClickable(
                        Clickable{
                            id,
                            500,
                            item.GetLocation()});
                    clickables.emplace(id, &item);
                    /*systems.AddRenderable(
                        Renderable{
                            id,
                            objStore.GetObject("clickable"),
                            item.GetLocation(),
                            item.GetRotation(),
                            glm::vec3{item.GetZoneItem().GetScale()}});*/
                }
            }
        }
    }

    for (const auto& world : worlds.GetTiles())
    {
        for (const auto& enc : world.GetEncounters())
        {
            auto id = systems.GetNextItemId();
            const auto dims = enc.GetDims();
            systems.AddRenderable(
                Renderable{
                    id,
                    objStore.GetObject(BAK::Encounter::EncounterTypeToString(enc.GetType())),
                    enc.GetLocation(),
                    glm::vec3{0.0},
                    glm::vec3{dims.x, 50.0, dims.y} / BAK::gWorldScale});

            systems.AddIntersectable(
                Intersectable{
                    id,
                    Intersectable::Rect{
                        static_cast<double>(dims.x),
                        static_cast<double>(dims.y)},
                    enc.GetLocation()});

            encounters.emplace(id, &enc);
        }
    }

    const auto towns = BAK::Encounter::TownFactory{};
    const auto zones = BAK::LoadZones();
    const auto zones2 = BAK::Encounter::ZoneFactory{};

    auto guiScalar = 3.0f;

    auto nativeWidth = 320.0f;
    auto nativeHeight = 200.0f;

    auto width = nativeWidth * guiScalar;
    auto height = nativeHeight * guiScalar;
    auto guiScaleInv = glm::vec3{1 / guiScalar, 1 / guiScalar, 0};

    /* OPEN GL / GLFW SETUP  */

    auto window = Graphics::MakeGlfwWindow(
        height,
        width,
        "BaK");

    ImguiWrapper::Initialise(window.get());
    
    
    // Dark blue background
    glClearColor(0.15f, 0.31f, 0.36f, 0.0f);

    auto spriteShader = ShaderProgram{
        "sprite.vert.glsl",
        "sprite.frag.glsl"};
    auto spriteShaderId = spriteShader.Compile();

    auto modelShader = ShaderProgram{
        "normal.vert.glsl",
        "normal.frag.glsl"};
    auto modelShaderId = modelShader.Compile();

    auto spriteManager = Graphics::SpriteManager{};
    auto guiRenderer = Graphics::GuiRenderer{
        width,
        height,
        guiScalar,
        spriteManager};

    auto root = Gui::Window{
        spriteManager,
        width / guiScalar,
        height / guiScalar};
        
    auto gameState = BAK::GameState{gameData};
    auto guiManager = Gui::GuiManager{
        root.GetCursor(),
        spriteManager,
        gameState
    };
    guiManager.EnterMainView();

    root.AddChildFront(&guiManager);

    auto guiShaderProgram = ShaderProgram{
        "gui.vert.glsl",
        "gui.frag.glsl"};
    auto guiShader = guiShaderProgram.Compile();

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    Graphics::GLBuffers buffers{};
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

    Graphics::TextureBuffer textureBuffer{};
    textureBuffer.LoadTexturesGL(
        textureStore.GetTextures(),
        textureStore.GetMaxDim());

    glm::mat4 viewMatrix{1};
    glm::mat4 modelMatrix{1.0f};
    glm::mat4 MVP{0};

    glm::vec3 lightPos = glm::vec3(0,220,0);
    
    Camera camera{static_cast<unsigned int>(width), static_cast<unsigned int>(height), 400 * 30.0f, 2.0f};
    camera.SetPosition(startPosition);

    Graphics::InputHandler inputHandler{};
    inputHandler.Bind(GLFW_KEY_W, [&]{ camera.MoveForward(); });
    inputHandler.Bind(GLFW_KEY_A, [&]{ camera.StrafeLeft(); });
    inputHandler.Bind(GLFW_KEY_D, [&]{ camera.StrafeRight(); });
    inputHandler.Bind(GLFW_KEY_S, [&]{ camera.MoveBackward(); });
    inputHandler.Bind(GLFW_KEY_Q, [&]{ camera.RotateLeft(); });
    inputHandler.Bind(GLFW_KEY_E, [&]{ camera.RotateRight(); });
    inputHandler.Bind(GLFW_KEY_X, [&]{ camera.RotateVerticalUp(); });
    inputHandler.Bind(GLFW_KEY_Y, [&]{ camera.RotateVerticalDown(); });
    inputHandler.Bind(GLFW_KEY_P, [&]{ lightPos.y += .5; });
    inputHandler.Bind(GLFW_KEY_L, [&]{ lightPos.y -= .5; });

    Graphics::InputHandler::BindMouseToWindow(window.get(), inputHandler);
    inputHandler.BindMouse(
        GLFW_MOUSE_BUTTON_LEFT,
        [&](auto click)
        {
            root.LeftMousePress(guiScaleInv * click);
        },
        [&](auto click)
        {
            root.LeftMouseRelease(guiScaleInv * click);
        }
    );

    inputHandler.BindMouse(
        GLFW_MOUSE_BUTTON_RIGHT,
        [&](auto click)
        {
            root.RightMousePress(guiScaleInv * click);
        },
        [&](auto click)
        {
            root.RightMouseRelease(guiScaleInv * click);
        }
    );

    inputHandler.BindMouseMotion(
        [&](auto pos)
        {
            root.MouseMoved(guiScaleInv * pos);
        }
    );

    double currentTime = 0;
    double lastTime = 0;
    float deltaTime = 0;

    glfwSetCursorPos(window.get(), width/2, height/2);
    glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    glEnable(GL_MULTISAMPLE);  

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glActiveTexture(GL_TEXTURE0);
    textureBuffer.BindGL();

    const BAK::Encounter::Encounter* activeEncounter{nullptr};
    const BAK::WorldItemInstance* activeClickable{nullptr};

    double pointerPosX, pointerPosY;

    do
    {
        activeEncounter = nullptr;

        currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        camera.SetDeltaTime(deltaTime);
        
        glfwPollEvents();
        glfwGetCursorPos(window.get(), &pointerPosX, &pointerPosY);
        inputHandler.HandleInput(window.get());

        lightPos.x = camera.GetNormalisedPosition().x;
        lightPos.z = camera.GetNormalisedPosition().z;

        auto RenderItems = [&](auto programId, const auto& renderables)
        {
            GLuint textureID = glGetUniformLocation(programId, "texture0");
            glUniform1i(textureID, 0);

            GLuint lightId = glGetUniformLocation(programId, "lightPosition_worldspace");
            glUniform3f(lightId, lightPos.x, lightPos.y, lightPos.z);
            GLuint cameraPositionId = glGetUniformLocation(programId, "cameraPosition_worldspace");
            glUniform3f(
                cameraPositionId, 
                camera.GetNormalisedPosition().x,
                camera.GetNormalisedPosition().y,
                camera.GetNormalisedPosition().z);

            viewMatrix = camera.GetViewMatrix();

            GLuint mvpMatrixID   = glGetUniformLocation(programId, "MVP");
            GLuint modelMatrixID = glGetUniformLocation(programId, "M");
            GLuint viewMatrixID  = glGetUniformLocation(programId, "V");

            for (const auto& item : renderables)
            {
                if (glm::distance(camera.GetPosition(), item.GetLocation()) > 128000.0) continue;
                const auto [offset, length] = item.GetObject();
                auto modelMatrix = item.GetModelMatrix();

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
        };

        glBindVertexArray(VertexArrayID);
        textureBuffer.BindGL();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        modelShaderId.UseProgramGL();
        RenderItems(modelShaderId.GetHandle(), systems.GetRenderables());

        spriteShaderId.UseProgramGL();
        RenderItems(spriteShaderId.GetHandle(), systems.GetSprites());
        
        auto intersectable = systems.RunIntersection(camera.GetPosition());
        if (intersectable)
            activeEncounter = encounters[*intersectable];

        if (glfwGetMouseButton(window.get(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
            activeClickable = nullptr;

        if (glfwGetMouseButton(window.get(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            auto bestId = systems.RunClickable(
                std::make_pair(
                    camera.GetPosition(), 
                    camera.GetPosition() + (camera.GetDirection() * 3000.0f)));

            if (bestId)
                activeClickable = clickables[*bestId];
        }

        // { *** GUI START ***
        glDisable(GL_DEPTH_TEST);
        guiShader.UseProgramGL();
        guiRenderer.RenderGui(&root);
        spriteManager.DeactivateSpriteSheet();
        glEnable(GL_DEPTH_TEST);


        // { *** IMGUI START ***
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ShowCameraGui(camera);

        if (activeEncounter != nullptr)
        {
            activeClickable = nullptr;
            ImGui::Begin("Encounter");
            std::stringstream ss{};
            ss << "Encounter: " << activeEncounter->GetType() << " Index: "
                << activeEncounter->GetIndex() << std::endl
                << "tile: " << activeEncounter->GetTile() << std::endl
                << " Loc: " << activeEncounter->GetLocation() << std::endl
                << "SA: " << std::hex << activeEncounter->GetSaveAddress() << std::endl;
            ImGui::Text(ss.str().c_str());
            ImGui::End();
            
            const auto encounterType = activeEncounter->GetType();
            switch (encounterType)
            {
                case BAK::Encounter::EncounterType::Dialog:
                {
                    ShowDialogGuiIndex(
                        activeEncounter->GetIndex(),
                        dialogStore,
                        dialogIndex,
                        gameData);
                } break;
                case BAK::Encounter::EncounterType::Town:
                {
                    const auto& town = towns.Get(
                        activeEncounter->GetIndex(),
                        glm::vec<2, unsigned>{0});
                    ShowDialogGui(
                        town.mEntryDialog,
                        dialogStore,
                        dialogIndex,
                        gameData);

                    if (guiManager.mScreens.size() == 1)
                        guiManager.EnterGDSScene(town.mHotspot);
                } break;
                case BAK::Encounter::EncounterType::Zone:
                {
                    ShowDialogGui(
                        zones[activeEncounter->GetIndex()].mDialog,
                        dialogStore,
                        dialogIndex,
                        gameData);
                } break;
                default:
                {
                } break;
            }
        }

        if (activeClickable != nullptr)
        {
            auto bakLocation = activeClickable->GetBakLocation();

            ImGui::Begin("Clickable");
            std::stringstream ss{};
            ss << "Clickable: " << activeClickable->GetZoneItem().GetName() 
                << " Location: " << bakLocation;
            ImGui::Text(ss.str().c_str());

            ImGui::Separator();

            auto cit = std::find_if(containers.begin(), containers.end(),
                [&bakLocation](const auto& x){ return x.mLocation == bakLocation; });
            if (cit != containers.end())
                ShowContainerGui(*cit);

            ImGui::End();

            auto fit = std::find_if(fixedObjects.begin(), fixedObjects.end(),
                [&bakLocation](const auto& x){ return x.mLocation == bakLocation; });
            if (fit != fixedObjects.end())
                ShowDialogGui(fit->mDialogKey, dialogStore, dialogIndex, gameData);
        }

        ImguiWrapper::Draw(window.get());

        // *** IMGUI END *** }
        
        glfwSwapBuffers(window.get());
    }
    while (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window.get()) == 0);

    glDeleteVertexArrays(1, &VertexArrayID);

    ImguiWrapper::Shutdown();

    glfwTerminate();

    delete gameData;

    return 0;
}


