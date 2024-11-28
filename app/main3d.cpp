#include "app/config.hpp"

#include "bak/backgroundSounds.hpp"
#include "bak/camera.hpp"
#include "bak/constants.hpp"

#include "bak/state/encounter.hpp"
#include "bak/encounter//encounter.hpp"
#include "bak/zone.hpp"

extern "C" {
#include "com/getopt.h"
}

#include "com/logger.hpp"
#include "com/path.hpp"
#include "com/visit.hpp"

#include "game/console.hpp"
#include "game/gameRunner.hpp"
#include "game/screens.hpp"
#include "game/systems.hpp"

#include "graphics/inputHandler.hpp"
#include "graphics/guiRenderer.hpp"
#include "graphics/glfw.hpp"
#include "graphics/renderer.hpp"
#include "graphics/sprites.hpp"

#include "gui/guiManager.hpp"
#include "gui/window.hpp"

#include "imgui/imguiWrapper.hpp"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <cmath>
#include <filesystem>
#include <functional>
#include <memory>
#include <numbers>
#include <sstream>

#undef main
struct Options
{
    bool showImgui{true};
    std::string logLevel{""};
    std::string configFile{""};
};

Options Parse(int argc, char** argv)
{
    Options values{};

    struct option options[] = {
        {"help", no_argument,       0, 'h'},
        {"config", required_argument, 0, 'c'},
        {"log_level", required_argument, 0, 'l'},
        {"imgui", no_argument, 0, 'i'},
    };
    int optionIndex = 0;
    int opt;
    while ((opt = getopt_long(argc, argv, "hil:c:", options, &optionIndex)) != -1)
    {
        if (opt == 'h')
        {
            exit(0);
        }
        else if (opt == 'c')
        {
            if (optarg == nullptr)
            {
                std::cerr << "No argument provide to '-c/--config'" << std::endl;
                exit(1);
            }
            values.configFile = std::string{optarg};
        }
        else if (opt == 'i')
        {
            values.showImgui = false;
        }
        else if (opt == 'l')
        {
            if (optarg == nullptr)
            {
                std::cerr << "No argument provide to '-c/--config'" << std::endl;
                exit(1);
            }
            values.logLevel = std::string{optarg};
        }
    }

    return values;
}

Config::Config LoadConfigFile(std::string configPath)
{
    auto config = Config::Config{};
    auto TryLoad = [&config](std::string path)
    {
        try
        {
            std::cout << "Loading config file: " << path << std::endl;
            config = Config::LoadConfig(path);
            return "";
        }
        catch (const std::exception& error)
        {
            std::cerr << "Failed to load config file due to: " << error.what() << std::endl;
            exit(1);
        }
    };

    const auto defaultConfig = (Paths::Get().GetBakDirectoryPath() / "config.json").string();

    if (!configPath.empty())
    {
        TryLoad(configPath);
    }
    else if (std::filesystem::exists(defaultConfig))
    {
        TryLoad(defaultConfig);
    }
    else
    {
        std::cout << "Not loading a config file.\n";
    }

    return config;
}

int main(int argc, char** argv)
{
    const auto options = Parse(argc, argv);
    const auto config = LoadConfigFile(options.configFile);
    Logging::LogState::SetLogTime(config.mLogging.mLogTime);
    Logging::LogState::SetLogColor(config.mLogging.mLogColours);
    if (options.logLevel != "")
    {
        Logging::LogState::SetLevel(options.logLevel);
    }
    else
    {
        Logging::LogState::SetLevel(config.mLogging.mLogLevel);
    }

    auto log = std::ofstream{Paths::Get().GetBakDirectoryPath() / "main3d.log"};
    Logging::LogState::AddStream(&log);

    const auto& logger = Logging::LogState::GetLogger("main");
    for (const auto& disabled : config.mLogging.mDisabledLoggers)
    {
        Logging::LogState::Disable(disabled);
    }
    
    if (!config.mPaths.mGameData.empty())
    {
        Paths::Get().SetBakDirectory(config.mPaths.mGameData);
    }

    if (!config.mPaths.mGraphicsOverrides.empty())
    {
        Paths::Get().SetModDirectory(config.mPaths.mGraphicsOverrides);
    }

    if (config.mAudio.mEnableAudio)
    {
        auto& provider = AudioA::AudioManagerProvider::Get();
        auto audioManager = std::make_unique<AudioA::AudioManager>();
        audioManager->Set(audioManager.get());
        audioManager->SwitchMidiPlayer(AudioA::StringToMidiPlayer(config.mAudio.mMidiPlayer));
        provider.SetAudioManager(std::move(audioManager));
    }
    else
    {
        auto& provider = AudioA::AudioManagerProvider::Get();
        auto audioManager = std::make_unique<AudioA::NullAudioManager>();
        provider.SetAudioManager(std::move(audioManager));
    }

    const bool showImgui = config.mGraphics.mEnableImGui;


    auto guiScalar = config.mGraphics.mResolutionScale;

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
    bool runningGame = false;
    auto renderer = Graphics::Renderer{
        width,
        height,
        sShadowDim,
        sShadowDim};

    Game::GameRunner gameRunner{
        camera,
        gameState,
        guiManager};

    // Wire up the zone loader to the GUI manager
    guiManager.SetZoneLoader(&gameRunner);

    auto currentTile = camera.GetGameTile();
    logger.Info() << " Starting on tile: " << currentTile << "\n";

    Graphics::Light light{
        glm::vec3{.0, -.25,  .00},
        glm::vec3{.5,  .5,   .5},
        glm::vec3{ 1,  .85,  .87},
        glm::vec3{.2,  .2,   .2},
        .0005f,
        glm::vec3{.15, .31, .36}
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
        if (camera.GetGameTile() != currentTile && gameRunner.mGameState.GetGameData())
        {
            currentTile = camera.GetGameTile();
            logger.Debug() << "New tile: " << currentTile << "\n";
            gameRunner.mGameState.Apply(BAK::State::ClearTileRecentEncounters);
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
    inputHandler.Bind(GLFW_KEY_C, [&]{ if (guiManager.InMainView()) gameRunner.mGameState.Apply(BAK::State::ClearTileRecentEncounters); });

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
            if (!guiHandled && guiManager.InMainView())
            {
                glDisable(GL_BLEND);
                glDisable(GL_MULTISAMPLE);
                renderer.DrawForPicking(
                    gameRunner.GetZoneRenderData(),
                    gameRunner.mSystems->GetRenderables(),
                    gameRunner.mSystems->GetSprites(),
                    gameRunner.mSystems->GetDynamicRenderables(),
                    *cameraPtr);
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
    console.mGuiManager = &guiManager;
    console.mGameState = &gameState;
    console.ToggleLog();

    // Do this last so we don't blast Imgui's callback hooks
    if (showImgui)
    {
        ImguiWrapper::Initialise(window.get());
    }

    do
    {
        currentTime = glfwGetTime();

        deltaTime = float(currentTime - lastTime);
        guiManager.OnTimeDelta(currentTime - lastTime);
        gameRunner.OnTimeDelta(currentTime - lastTime);
        lastTime = currentTime;

        cameraPtr->SetDeltaTime(deltaTime);
        if (gameRunner.mGameState.GetGameData())
        {
            if (guiManager.InMainView())
            {
                gameState.SetLocation(cameraPtr->GetGameLocation());
            }
        }

        glfwPollEvents();
        glfwGetCursorPos(window.get(), &pointerPosX, &pointerPosY);
        inputHandler.HandleInput(window.get());

        // { *** Draw 3D World ***
        UpdateLightCamera();

        glEnable(GL_BLEND);
        glEnable(GL_MULTISAMPLE);  

        if (gameRunner.mGameState.GetGameData() != nullptr)
        {
            double bakTimeOfDay = (gameState.GetWorldTime().GetTime().mTime % 43200);
            auto twoPi = std::numbers::pi_v<double> * 2.0;
            // light starts at 6 after midnight
            auto sixHours = 7200.0;
            auto beginDay = bakTimeOfDay - sixHours;
            bool isNight = bakTimeOfDay < 7200|| bakTimeOfDay > 36000;
            light.mDirection = glm::vec3{
                std::cos(beginDay * (twoPi / (28800 * 2))),
                isNight ? .1 : -.25,
                0};
            float ambient = isNight
                ? .05
                : std::sin(beginDay * (twoPi / 57600));
            light.mAmbientColor = glm::vec3{ambient};
            light.mDiffuseColor = ambient * glm::vec3{
                1.,
                std::sin(beginDay * (twoPi / (57600 * 2))),
                std::sin(beginDay * (twoPi / (57600 * 2)))
            };

            light.mSpecularColor = isNight ? glm::vec3{0} : ambient * glm::vec3{
                1.,
                std::sin(beginDay * (twoPi / (57600 * 2))),
                std::sin(beginDay * (twoPi / (57600 * 2)))
            };
            light.mFogColor = ambient * glm::vec3{.15, .31, .36};

            renderer.BeginDepthMapDraw();
            renderer.DrawDepthMap(
                gameRunner.GetZoneRenderData(),
                gameRunner.mSystems->GetRenderables(),
                lightCamera);
            renderer.DrawDepthMap(
                gameRunner.GetZoneRenderData(),
                gameRunner.mSystems->GetSprites(),
                lightCamera);
            renderer.EndDepthMapDraw();

            glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
            // Dark blue background
            glClearColor(ambient * 0.15f, ambient * 0.31f, ambient * 0.36f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderer.DrawWithShadow(
                gameRunner.GetZoneRenderData(),
                gameRunner.mSystems->GetRenderables(),
                light,
                lightCamera,
                *cameraPtr);

            renderer.DrawWithShadow(
                gameRunner.GetZoneRenderData(),
                gameRunner.mSystems->GetSprites(),
                light,
                lightCamera,
                *cameraPtr);

            const auto& dynamicRenderables = gameRunner.mSystems->GetDynamicRenderables();
            for (const auto& obj : dynamicRenderables)
            {
                std::vector<DynamicRenderable> data{};
                data.emplace_back(obj);
                renderer.DrawWithShadow(
                    *obj.GetRenderData(),
                    data,
                    light,
                    lightCamera,
                    *cameraPtr);
            }


        }

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

        if (gameRunner.mGameState.GetGameData() && guiManager.InMainView())
        {
            gameRunner.RunGameUpdate(config.mGame.mAdvanceTime);
            if (config.mAudio.mEnableBackgroundSounds)
            {
				BAK::PlayBackgroundSounds(gameRunner.mGameState);
            }
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
                            gameRunner.mGameState.GetGameData());
                    },
                    [&](const BAK::Encounter::Block& e){
                        ShowDialogGui(
                            e.mDialog,
                            BAK::DialogStore::Get(),
                            gameRunner.mGameState.GetGameData());
                    },
                    [&](const BAK::Encounter::Combat& e){
                        ShowDialogGui(
                            e.mEntryDialog,
                            BAK::DialogStore::Get(),
                            gameRunner.mGameState.GetGameData());
                    },
                    [&](const BAK::Encounter::Dialog& e){
                        ShowDialogGui(
                            e.mDialog,
                            BAK::DialogStore::Get(),
                            gameRunner.mGameState.GetGameData());
                    },
                    [](const BAK::Encounter::EventFlag&){
                    },
                    [&](const BAK::Encounter::Zone& e){
                        ShowDialogGui(
                            e.mDialog,
                            BAK::DialogStore::Get(),
                            gameRunner.mGameState.GetGameData());
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
