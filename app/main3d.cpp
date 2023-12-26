#include "bak/camera.hpp"
#include "bak/constants.hpp"
#include "bak/container.hpp"
#include "bak/coordinates.hpp"
#include "bak/gameData.hpp"
#include "bak/screens.hpp"

extern "C" {
#include "com/getopt.h"
}

#include "com/logger.hpp"
#include "com/path.hpp"
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

#include <filesystem>
#include <functional>
#include <memory>
#include <sstream>

#undef main

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");

    const bool showImgui = true;

    auto log = std::ofstream{ std::filesystem::path{GetBakDirectory()} / "main3d.log" };
    Logging::LogState::AddStream(&log);
    Logging::LogState::SetLevel(Logging::LogLevel::Info);

    Logging::LogState::Disable("Compass");
    Logging::LogState::Disable("DialogStore");
    Logging::LogState::Disable("LoadEncounter");
    Logging::LogState::Disable("LoadFixedObjects");
    Logging::LogState::Disable("LoadSceneIndices");
    Logging::LogState::Disable("LoadScenes");
    Logging::LogState::Disable("MeshObjectStore");
    Logging::LogState::Disable("GameData");
    Logging::LogState::Disable("Gui::StaticTTM");
    Logging::LogState::Disable("GuiRenderer");
    //Logging::LogState::Disable("Gui::DialogRunner");
    Logging::LogState::Disable("Gui::DialogDisplay");
    Logging::LogState::Disable("Gui::AnimatorStore");

    struct option options[] = {
        {"help", no_argument,       0, 'h'},
        {"save", required_argument, 0, 's'},
        {"zone", required_argument, 0, 'z'}
    };
    int optionIndex = 0;
    int opt;

    BAK::ZoneLabel zoneLabel{1};
    std::optional<std::string> saveName{};
    
	bool noOptions = true;
    while ((opt = getopt_long(argc, argv, "hs:z:", options, &optionIndex)) != -1)
    {   
        if (opt == 'h')
        {
            std::cout << "Usage: " << argv[0] << " --save SAVE_FILE | --zone ZXX\n";
            exit(0);
        }
        else if (opt == 's')
        {
			noOptions = false;
            logger.Info() << "Loading save file: " << optarg << std::endl;
            saveName = optarg;
        }
        else if (opt == 'z')
        {
			noOptions = false;
            logger.Info() << "Loading zone: " << optarg << std::endl;
            auto zone = std::string{optarg};
            zoneLabel = BAK::ZoneLabel{zone};
        }
    }

	if (noOptions)
	{
		logger.Info() << "Attempting to load default save 'NEW_GAME.GAM'\n";
		saveName = "NEW_GAME.GAM";
	}

    auto guiScalar = 2.0f;

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

    if (showImgui)
    {
        ImguiWrapper::Initialise(window.get());
    }
    

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
        
    auto gameState = BAK::GameState{nullptr};

    auto guiManager = Gui::GuiManager{
        root.GetCursor(),
        spriteManager,
        gameState
    };

    root.AddChildFront(&guiManager);
    guiManager.EnterMainMenu(false);

    Camera lightCamera{
        static_cast<unsigned>(width),
        static_cast<unsigned>(height),
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

    // OpenGL 3D Renderer
    constexpr auto sShadowDim = 4096;
    auto renderer = Graphics::Renderer{
        width,
        height,
        sShadowDim,
        sShadowDim};

    Game::GameRunner gameRunner{
        camera,
        gameState,
        guiManager,
        [&](auto& zoneData){
            renderer.LoadData(zoneData.mObjects, zoneData.mZoneTextures);
        }};

    // Wire up the zone loader to the GUI manager
    guiManager.SetZoneLoader(&gameRunner);
    if (saveName)
    {
        gameRunner.LoadGame(*saveName);
    }
    else
    {
        gameRunner.LoadZoneData(zoneLabel.GetZoneNumber());
        auto position = gameRunner.mZoneData->mWorldTiles
                .GetTiles().front().GetCenter();
        position.y = 100;
        camera.SetPosition(position);
    }

    auto currentTile = camera.GetGameTile();
    logger.Info() << " Starting on tile: " << currentTile << "\n";

    Graphics::Light light{
        glm::vec3{.2, -.1, .05},
        glm::vec3{.5, .5, .5},
        glm::vec3{1,.85,.87},
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

    auto UpdateGameTile = [&]()
    {
        if (camera.GetGameTile() != currentTile && gameRunner.mGameState.mGameData)
        {
            currentTile = camera.GetGameTile();
            logger.Debug() << "New tile: " << currentTile << "\n";
            gameRunner.mGameState.mGameData->ClearTileRecentEncounters();
        }
    };

    Graphics::InputHandler inputHandler{};
    inputHandler.Bind(GLFW_KEY_G,     [&]{ if (guiManager.InMainView()) cameraPtr = &camera; });
    inputHandler.Bind(GLFW_KEY_H,     [&]{ if (guiManager.InMainView()) cameraPtr = &lightCamera; });
    inputHandler.Bind(GLFW_KEY_R,     [&]{
        if (guiManager.InMainView())
            UpdateLightCamera();
    });
    inputHandler.Bind(GLFW_KEY_UP,   [&]{ if (guiManager.InMainView()){cameraPtr->StrafeForward(); UpdateGameTile();}});
    inputHandler.Bind(GLFW_KEY_DOWN, [&]{ if (guiManager.InMainView()){cameraPtr->StrafeBackward(); UpdateGameTile();}});
    inputHandler.Bind(GLFW_KEY_LEFT, [&]{ if (guiManager.InMainView()){cameraPtr->StrafeLeft(); UpdateGameTile();}});
    inputHandler.Bind(GLFW_KEY_RIGHT,[&]{ if (guiManager.InMainView()){cameraPtr->StrafeRight(); UpdateGameTile();}});

    inputHandler.Bind(GLFW_KEY_W, [&]{ if (guiManager.InMainView()){cameraPtr->MoveForward(); UpdateGameTile();}});
    inputHandler.Bind(GLFW_KEY_A, [&]{ if (guiManager.InMainView()){cameraPtr->StrafeLeft(); UpdateGameTile();}});
    inputHandler.Bind(GLFW_KEY_D, [&]{ if (guiManager.InMainView()){cameraPtr->StrafeRight(); UpdateGameTile();}});
    inputHandler.Bind(GLFW_KEY_S, [&]{ if (guiManager.InMainView()){cameraPtr->MoveBackward(); UpdateGameTile();}});
    inputHandler.Bind(GLFW_KEY_Q, [&]{
        if (guiManager.InMainView())
        {
            cameraPtr->RotateLeft();
            guiManager.mMainView.SetHeading(cameraPtr->GetHeading());
        }});
    inputHandler.Bind(GLFW_KEY_E, [&]{ 
        if (guiManager.InMainView())
        {
            cameraPtr->RotateRight();
            guiManager.mMainView.SetHeading(cameraPtr->GetHeading());
        }});
    inputHandler.Bind(GLFW_KEY_X, [&]{ if (guiManager.InMainView()) cameraPtr->RotateVerticalUp(); });
    inputHandler.Bind(GLFW_KEY_Y, [&]{ if (guiManager.InMainView()) cameraPtr->RotateVerticalDown(); });
    inputHandler.Bind(GLFW_KEY_C, [&]{ if (guiManager.InMainView()) gameRunner.mGameState.mGameData->ClearTileRecentEncounters(); });

    inputHandler.Bind(GLFW_KEY_BACKSPACE,   [&]{ if (root.OnKeyEvent(Gui::KeyPress{GLFW_KEY_BACKSPACE})){ ;} });
    inputHandler.BindCharacter([&](char character){ if(root.OnKeyEvent(Gui::Character{character})){ ;} });

    Graphics::InputHandler::BindKeyboardToWindow(window.get(), inputHandler);
    Graphics::InputHandler::BindMouseToWindow(window.get(), inputHandler);

    inputHandler.BindMouse(
        GLFW_MOUSE_BUTTON_LEFT,
        [&](auto clickPos)
        {
            bool guiHandled = root.OnMouseEvent(
                Gui::LeftMousePress{guiScaleInv * clickPos});
            // i.e. only MainView is present
            // should really formalise this with some sort of
            // GuiManager state, interacting with 2d or 3d world..?
            if (!guiHandled && guiManager.mScreenStack.size() == 1)
            {
                gameRunner.CheckClickable(renderer.GetClickedEntity(clickPos));
            }
        },
        [&](auto clickPos)
        {
            root.OnMouseEvent(
                Gui::LeftMouseRelease{guiScaleInv * clickPos});
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
        }
    );

    inputHandler.BindMouseMotion(
        [&](auto pos)
        {
            root.OnMouseEvent(
                Gui::MouseMove{guiScaleInv * pos});
        }
    );

    inputHandler.BindMouseScroll(
        [&](auto pos)
        {
            root.OnMouseEvent(
                Gui::MouseScroll{guiScaleInv * pos});
        }
    );

    double currentTime = 0;
    double lastTime = 0;
    float deltaTime = 0;

    glfwSetCursorPos(window.get(), width/2, height/2);
    //glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    //glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
    console.ToggleLog();

    do
    {
        currentTime = glfwGetTime();

        deltaTime = float(currentTime - lastTime);
        guiManager.OnTimeDelta(currentTime - lastTime);
        lastTime = currentTime;

        cameraPtr->SetDeltaTime(deltaTime);
        gameState.SetLocation(cameraPtr->GetGameLocation());
        guiManager.mFullMap.UpdateLocation();

        glfwPollEvents();
        glfwGetCursorPos(window.get(), &pointerPosX, &pointerPosY);
        inputHandler.HandleInput(window.get());

        // { *** Draw 3D World ***
        UpdateLightCamera();

        glDisable(GL_BLEND);
        glDisable(GL_MULTISAMPLE);  

        renderer.DrawForPicking(
            gameRunner.mSystems->GetRenderables(),
            gameRunner.mSystems->GetSprites(),
            *cameraPtr);

        glEnable(GL_BLEND);
        glEnable(GL_MULTISAMPLE);  

        renderer.BeginDepthMapDraw();
        renderer.DrawDepthMap(
            gameRunner.mSystems->GetRenderables(),
            lightCamera);
        renderer.DrawDepthMap(
            gameRunner.mSystems->GetSprites(),
            lightCamera);
        renderer.EndDepthMapDraw();

        glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
        // Dark blue background
        glClearColor(0.15f, 0.31f, 0.36f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.DrawWithShadow(
            gameRunner.mSystems->GetRenderables(),
            light,
            lightCamera,
            *cameraPtr);

        renderer.DrawWithShadow(
            gameRunner.mSystems->GetSprites(),
            light,
            lightCamera,
            *cameraPtr);

        //// { *** Draw 2D GUI ***
        guiRenderer.RenderGui(&root);

        // { *** IMGUI START ***
        if (showImgui)
        {
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ShowLightGui(light);

			ShowCameraGui(camera);
			console.Draw("Console", &consoleOpen);
		}

        if (gameRunner.mGameState.mGameData)
        {
            gameRunner.RunGameUpdate();
        }

        if (showImgui && gameRunner.mActiveEncounter)
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
                            BAK::DialogStore::Get(),
                            gameRunner.mGameState.mGameData);
                    },
                    [&](const BAK::Encounter::Block& e){
                        ShowDialogGui(
                            e.mDialog,
                            BAK::DialogStore::Get(),
                            gameRunner.mGameState.mGameData);
                    },
                    [&](const BAK::Encounter::Combat& e){
                        ShowDialogGui(
                            e.mEntryDialog,
                            BAK::DialogStore::Get(),
                            gameRunner.mGameState.mGameData);
                    },
                    [&](const BAK::Encounter::Dialog& e){
                        ShowDialogGui(
                            e.mDialog,
                            BAK::DialogStore::Get(),
                            gameRunner.mGameState.mGameData);
                    },
                    [](const BAK::Encounter::EventFlag&){
                    },
                    [&](const BAK::Encounter::Zone& e){
                        ShowDialogGui(
                            e.mDialog,
                            BAK::DialogStore::Get(),
                            gameRunner.mGameState.mGameData);
                    },
                },
                encounter);
        }

        if (showImgui)
        {
            ImguiWrapper::Draw(window.get());
        }

        if (showImgui)
        {
			auto& io = ImGui::GetIO();
            if (io.WantCaptureKeyboard || io.WantCaptureMouse)
            {
                inputHandler.SetHandleInput(false);
            }
            else
            {
				inputHandler.SetHandleInput(true);
            }
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

    if (showImgui)
    {
        ImguiWrapper::Shutdown();
    }

    return 0;
}
