#include "bak/camera.hpp"
#include "bak/constants.hpp"
#include "bak/container.hpp"
#include "bak/coordinates.hpp"
#include "bak/gameData.hpp"
#include "bak/screens.hpp"

#include "com/logger.hpp"
#include "com/visit.hpp"

#include "game/console.hpp"
#include "game/gameRunner.hpp"
#include "game/systems.hpp"

#include "graphics/inputHandler.hpp"
#include "graphics/guiRenderer.hpp"
#include "graphics/glfw.hpp"
#include "graphics/framebuffer.hpp"
#include "graphics/renderer.hpp"
#include "graphics/sprites.hpp"

#include "gui/guiManager.hpp"
#include "gui/window.hpp"

#include "imgui/imguiWrapper.hpp"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

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
    Logging::LogState::Disable("Camera");
    Logging::LogState::Disable("Compass");
    Logging::LogState::Disable("DialogStore");
    Logging::LogState::Disable("LoadEncounter");
    Logging::LogState::Disable("LoadSceneIndices");
    Logging::LogState::Disable("LoadScenes");
    Logging::LogState::Disable("MeshObjectStore");
    Logging::LogState::Disable("Gui::StaticTTM");


    BAK::ZoneLabel zoneLabel{1};
    glm::vec<3, float> startPosition{0.0f, 0.0f, 0.0f};
    glm::vec<2, float> startHeading{3.14f, 0.0f};
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
            const auto zone = gameData->mLocation.mZone;
            containers = gameData->LoadContainers(zone);
            logger.Info() << "Loaded save: " << gameData->mName << "\n";

            zoneLabel = BAK::ZoneLabel{zone};
            startPosition = BAK::ToGlCoord<float>(gameData->mLocation.mLocation.mPosition);
            startHeading = BAK::ToGlAngle(gameData->mLocation.mLocation.mHeading);
            logger.Info() << "StartHeading: " << startHeading << "\n";
        }
        else if (opt == 'z')
        {
            logger.Info() << "Loading zone: " << optarg << std::endl;
            auto zone = std::string{optarg};
            zoneLabel = BAK::ZoneLabel{zone};
        }
    }

    BAK::DialogStore dialogStore{};

    auto guiScalar = 4.0f;

    auto nativeWidth = 320.0f;
    auto nativeHeight = 200.0f;

    auto width = nativeWidth * guiScalar;
    auto height = nativeHeight * guiScalar;
    auto guiScaleInv = glm::vec2{1 / guiScalar, 1 / guiScalar};

    /* OPEN GL / GLFW SETUP  */

    auto window = Graphics::MakeGlfwWindow(
        height,
        width,
        "BaK");

    ImguiWrapper::Initialise(window.get());
    
    
    // Dark blue background
    glClearColor(0.15f, 0.31f, 0.36f, 0.0f);

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

    constexpr auto SHADOW_DIM = 8096;
    Camera lightCamera{
        static_cast<unsigned>(SHADOW_DIM),
        static_cast<unsigned>(SHADOW_DIM),
        400 * 30.0f,
        2.0f};
    lightCamera.UseOrthoMatrix(400, 400);

    Camera camera{
        static_cast<unsigned>(width),
        static_cast<unsigned>(height),
        400 * 30.0f,
        2.0f};
    Camera* cameraPtr = &camera;

    guiManager.mMainView.SetHeading(camera.GetHeading());
    guiManager.mMainView.UpdatePartyMembers(gameState);

    // OpenGL 3D Renderer
    auto renderer = Graphics::Renderer{};

    Game::GameRunner gameRunner{
        camera,
        gameState,
        guiManager,
        [&](auto& zoneData){
            renderer.LoadData(zoneData.mObjects, zoneData.mZoneTextures);
        }};

    // Wire up the zone loader to the GUI manager
    guiManager.SetZoneLoader(&gameRunner);
    gameRunner.LoadZoneData(zoneLabel.GetZoneNumber());

    if (startPosition == glm::vec<3, float>{0,0,0})
        startPosition = gameRunner.mZoneData->mWorldTiles.GetTiles().front().GetCenter();

    startPosition.y = 100;

    camera.SetPosition(startPosition);
    camera.SetAngle(startHeading);

    Graphics::Light light{
        glm::vec3{.2, -1, 0},
        glm::vec3{.5, .5, .5},
        glm::vec3{1,1,1},
        glm::vec3{.2,.2,.2}
    };

    const auto UpdateLightCamera = [&]{
        const auto lightPos = camera.GetNormalisedPosition() - 100.0f * glm::normalize(light.mDirection);
        const auto diff = lightCamera.GetNormalisedPosition() - camera.GetNormalisedPosition();
        const auto horizDistance = glm::sqrt((diff.x * diff.x) + (diff.z * diff.z));
        const auto yAngle = -glm::atan(diff.y / horizDistance);
        const auto xAngle = glm::atan(diff.x, diff.z) - ((180.0f / 360.0f) * (2 * 3.141592)) ;

        lightCamera.SetAngle(glm::vec2{xAngle, yAngle});
        lightCamera.SetPosition(lightPos * BAK::gWorldScale);
    };

    Graphics::InputHandler inputHandler{};
    inputHandler.Bind(GLFW_KEY_G,     [&]{ cameraPtr = &camera; });
    inputHandler.Bind(GLFW_KEY_H,     [&]{ cameraPtr = &lightCamera; });
    inputHandler.Bind(GLFW_KEY_R,     [&]{
        UpdateLightCamera();
    });
    inputHandler.Bind(GLFW_KEY_UP,   [&]{ cameraPtr->StrafeForward(); });
    inputHandler.Bind(GLFW_KEY_DOWN, [&]{ cameraPtr->StrafeBackward(); });
    inputHandler.Bind(GLFW_KEY_LEFT, [&]{ cameraPtr->StrafeLeft(); });
    inputHandler.Bind(GLFW_KEY_RIGHT,[&]{ cameraPtr->StrafeRight(); });

    inputHandler.Bind(GLFW_KEY_W, [&]{ cameraPtr->MoveForward(); });
    inputHandler.Bind(GLFW_KEY_A, [&]{ cameraPtr->StrafeLeft(); });
    inputHandler.Bind(GLFW_KEY_D, [&]{ cameraPtr->StrafeRight(); });
    inputHandler.Bind(GLFW_KEY_S, [&]{ cameraPtr->MoveBackward(); });
    inputHandler.Bind(GLFW_KEY_Q, [&]{
        cameraPtr->RotateLeft();
        guiManager.mMainView.SetHeading(cameraPtr->GetHeading());
        });
    inputHandler.Bind(GLFW_KEY_E, [&]{ 
        cameraPtr->RotateRight();
        guiManager.mMainView.SetHeading(cameraPtr->GetHeading());
        });
    inputHandler.Bind(GLFW_KEY_X, [&]{ cameraPtr->RotateVerticalUp(); });
    inputHandler.Bind(GLFW_KEY_Y, [&]{ cameraPtr->RotateVerticalDown(); });
    inputHandler.Bind(GLFW_KEY_C, [&]{ gameRunner.mGameState.mGameData->ClearTileRecentEncounters(); });

    Graphics::InputHandler::BindMouseToWindow(window.get(), inputHandler);

    inputHandler.BindMouse(
        GLFW_MOUSE_BUTTON_LEFT,
        [&](const auto click)
        {
            bool guiHandled = root.OnMouseEvent(
                Gui::LeftMousePress{guiScaleInv * click});
            if (!guiHandled)
            {
                gameRunner.CheckClickable();
            }
        },
        [&](const auto click)
        {
            root.OnMouseEvent(
                Gui::LeftMouseRelease{guiScaleInv * click});
        }
    );

    inputHandler.BindMouse(
        GLFW_MOUSE_BUTTON_RIGHT,
        [&](auto click)
        {
            root.OnMouseEvent(
                Gui::RightMousePress{guiScaleInv * click});
        },
        [&](auto click)
        {
            root.OnMouseEvent(
                Gui::RightMouseRelease{guiScaleInv * click});
            gameRunner.ResetClickable();
        }
    );

    inputHandler.BindMouseMotion(
        [&](auto pos)
        {
            root.OnMouseEvent(
                Gui::MouseMove{guiScaleInv * pos});
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
    
    double pointerPosX, pointerPosY;

    bool consoleOpen = true;
    auto console = Console{};
    console.mCamera = &camera;
    console.mGameRunner = &gameRunner;
    console.mGameState = &gameState;

    do
    {
        currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        guiManager.OnTimeDelta(currentTime - lastTime);
        lastTime = currentTime;

        cameraPtr->SetDeltaTime(deltaTime);
        
        glfwPollEvents();
        glfwGetCursorPos(window.get(), &pointerPosX, &pointerPosY);
        inputHandler.HandleInput(window.get());

        // { *** Draw 3D World ***
        UpdateLightCamera();

        renderer.mDepthFB.BindGL();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, SHADOW_DIM, SHADOW_DIM);

        renderer.Draw(
            gameRunner.mSystems->GetRenderables(),
            lightCamera);
        renderer.Draw(
            gameRunner.mSystems->GetSprites(),
            lightCamera);

        renderer.mDepthFB.UnbindGL();

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.DrawShadow(
            gameRunner.mSystems->GetRenderables(),
            light,
            lightCamera,
            *cameraPtr);

        renderer.DrawShadow(
            gameRunner.mSystems->GetSprites(),
            light,
            lightCamera,
            *cameraPtr);

        // { *** Draw 2D GUI ***
        guiRenderer.RenderGui(&root);

        // { *** IMGUI START ***
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ShowLightGui(light);

        ShowCameraGui(camera);
        console.Draw("Console", &consoleOpen);

        if (gameRunner.mActiveClickable)
        {
            ImGui::Begin("Clickable");
            std::stringstream ss{};
            ss << "Clickable: " << gameRunner.mActiveClickable->GetZoneItem().GetName()
                << "\nLocation: " << gameRunner.mActiveClickable->GetBakLocation() << "\n";
            ImGui::Text(ss.str().c_str());

            ImGui::Separator();

            ImGui::End();
        }

        gameRunner.RunGameUpdate();

        if (gameRunner.mActiveEncounter)
        {
            ImGui::Begin("Encounter");
            std::stringstream ss{};
            ss << "Encounter: " << *gameRunner.mActiveEncounter << std::endl;
            ImGui::TextWrapped(ss.str().c_str());
            ImGui::End();
            
            const auto& encounter = gameRunner.mActiveEncounter->GetEncounter();
            std::visit(
                overloaded{
                    [&](const BAK::Encounter::GDSEntry& gds){
                        ShowDialogGui(
                            gds.mEntryDialog,
                            dialogStore,
                            gameData);
                    },
                    [&](const BAK::Encounter::Block& e){
                        ShowDialogGui(
                            e.mDialog,
                            dialogStore,
                            gameData);
                    },
                    [&](const BAK::Encounter::Combat& e){
                        ShowDialogGui(
                            e.mEntryDialog,
                            dialogStore,
                            gameData);
                    },
                    [&](const BAK::Encounter::Dialog& e){
                        ShowDialogGui(
                            e.mDialog,
                            dialogStore,
                            gameData);
                    },
                    [](const BAK::Encounter::EventFlag&){
                    },
                    [&](const BAK::Encounter::Zone& e){
                        ShowDialogGui(
                            e.mDialog,
                            dialogStore,
                            gameData);
                    },
                },
                encounter);
        }

        ImguiWrapper::Draw(window.get());
        auto& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard || io.WantCaptureMouse)
        {
            inputHandler.SetHandleInput(false);
        }
        else
        {
            inputHandler.SetHandleInput(true);
        }

        // *** IMGUI END *** }
        
        glfwSwapBuffers(window.get());
    }
    while (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window.get()) == 0);

    ImguiWrapper::Shutdown();

    glfwTerminate();

    delete gameData;

    return 0;
}


