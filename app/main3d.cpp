#include "bak/camera.hpp"
#include "bak/constants.hpp"
#include "bak/container.hpp"
#include "bak/coordinates.hpp"
#include "bak/fixedObject.hpp"
#include "bak/gameData.hpp"
#include "bak/palette.hpp"
#include "bak/screens.hpp"
#include "bak/systems.hpp"
#include "bak/encounter/encounter.hpp"
#include "bak/encounter/block.hpp"
#include "bak/encounter/zone.hpp"
#include "bak/worldFactory.hpp"

#include "com/logger.hpp"
#include "com/visit.hpp"

#include "graphics/inputHandler.hpp"
#include "graphics/IGuiElement.hpp"
#include "graphics/cube.hpp"
#include "graphics/guiRenderer.hpp"
#include "graphics/glfw.hpp"
#include "graphics/line.hpp"
#include "graphics/meshObject.hpp"
#include "graphics/opengl.hpp"
#include "graphics/renderer.hpp"
#include "graphics/shaderProgram.hpp"
#include "graphics/sprites.hpp"
#include "graphics/texture.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/guiManager.hpp"
#include "gui/window.hpp"

#include "imgui/imguiWrapper.hpp"

#include "xbak/FileManager.h"
#include "xbak/FileBuffer.h"

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

    const auto pal = BAK::Palette{zoneLabel.GetPalette()};
    auto fixedObjects = BAK::LoadFixedObjects(zoneLabel.GetZoneNumber());
    auto textureStore = BAK::ZoneTextureStore{zoneLabel, pal};
    auto zoneItems   = BAK::ZoneItemStore{zoneLabel, textureStore};
    const auto encounterFactory = BAK::Encounter::EncounterFactory{};

    auto worlds = BAK::WorldTileStore{zoneItems, encounterFactory};

    if (startPosition == glm::vec<3, float>{0,0,0})
        startPosition = worlds.GetTiles().front().GetCenter();

    startPosition.y = 100;

    auto objectStore = Graphics::MeshObjectStorage{};
    for (auto& item : zoneItems.GetItems())
        objectStore.AddObject(
            item.GetName(),
            BAK::ZoneItemToMeshObject(item, textureStore, pal));

    const auto cube = Graphics::Cuboid{1, 1, 50};
    objectStore.AddObject("Combat", cube.ToMeshObject(glm::vec4{1.0, 0, 0, .3}));
    objectStore.AddObject("Trap", cube.ToMeshObject(glm::vec4{.8, 0, 0, .3}));
    objectStore.AddObject("Dialog", cube.ToMeshObject(glm::vec4{0.0, 1, 0, .3}));
    objectStore.AddObject("Zone", cube.ToMeshObject(glm::vec4{1.0, 1, 0, .3}));
    objectStore.AddObject("GDSEntry", cube.ToMeshObject(glm::vec4{1.0, 0, 1, .3}));
    objectStore.AddObject("EventFlag", cube.ToMeshObject(glm::vec4{.0, .0, .7, .3}));
    objectStore.AddObject("Block", cube.ToMeshObject(glm::vec4{0,0,0, .3}));

    auto clickable = Sphere{1.0, 12, 6, true};
    objectStore.AddObject(
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
                    objectStore.GetObject(item.GetZoneItem().GetName()),
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
                            objectStore.GetObject("clickable"),
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
                    objectStore.GetObject(std::string{BAK::Encounter::ToString(enc.GetEncounter())}),
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

    auto guiScalar = 5.0f;

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

    // OpenGL 3D Renderer
    auto renderer = Graphics::Renderer{};
    renderer.LoadData(objectStore, textureStore);

    glm::vec3 lightPos = glm::vec3{0, 220, 0};
    
    Camera camera{
        static_cast<unsigned>(width),
        static_cast<unsigned>(height),
        400 * 30.0f,
        2.0f};
    camera.SetPosition(startPosition);
    camera.SetAngle(startHeading);

    auto dialogScene = Gui::DynamicDialogScene{
        [&](){ camera.SetAngle(glm::vec2{3.14, 0}); },
        [&](){ camera.SetAngle(glm::vec2{0}); },
        [&](){ }
    };

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
        [&](const auto click)
        {
            root.OnMouseEvent(
                Gui::LeftMousePress{guiScaleInv * click});
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


        // { *** Draw 3D World ***
        lightPos.x = camera.GetNormalisedPosition().x;
        lightPos.z = camera.GetNormalisedPosition().z;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.Draw<true>(systems.GetRenderables(), lightPos, camera);
        renderer.Draw<false>(systems.GetSprites(), lightPos, camera);

        // { *** Draw 2D GUI ***
        guiRenderer.RenderGui(&root);


        // { *** Check for intersection with encounter
        auto intersectable = systems.RunIntersection(camera.GetPosition());
        if (intersectable)
            activeEncounter = encounters[*intersectable];

        //if (glfwGetMouseButton(window.get(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        //    activeClickable = nullptr;

        //if (glfwGetMouseButton(window.get(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        //{
        //    auto bestId = systems.RunClickable(
        //        std::make_pair(
        //            camera.GetPosition(), 
        //            camera.GetPosition() + (camera.GetDirection() * 3000.0f)));

        //    if (bestId)
        //        activeClickable = clickables[*bestId];
        //}


        // { *** IMGUI START ***
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ShowCameraGui(camera);

        if (activeEncounter != nullptr)
        {
            //activeClickable = nullptr;
            ImGui::Begin("Encounter");
            std::stringstream ss{};
            ss << "Encounter: " << *activeEncounter << std::endl;
            ImGui::TextWrapped(ss.str().c_str());
            ImGui::End();
            
            const auto& encounter = activeEncounter->GetEncounter();
            std::visit(
                overloaded{
                    [&](const BAK::Encounter::GDSEntry& gds){
                        ShowDialogGui(
                            gds.mEntryDialog,
                            dialogStore,
                            gameData);

                        if (guiManager.mScreenStack.size() == 1)
                            guiManager.EnterGDSScene(gds.mHotspot);

                    },
                    [&](const BAK::Encounter::Block& e){
                        ShowDialogGui(
                            e.mDialog,
                            dialogStore,
                            gameData);
                        if (guiManager.mScreenStack.size() == 1)
                            guiManager.StartDialog(
                                e.mDialog,
                                false,
                                &dialogScene);
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
                        if (guiManager.mScreenStack.size() == 1)
                            guiManager.StartDialog(
                                e.mDialog,
                                false,
                                &dialogScene);
                    },
                    [](const BAK::Encounter::EventFlag&){
                    },
                    [&](const BAK::Encounter::Trap& e){
                        ShowDialogGui(
                            e.mEntryDialog,
                            dialogStore,
                            gameData);
                    },
                    [&](const BAK::Encounter::Zone& e){
                        ShowDialogGui(
                            e.mDialog,
                            dialogStore,
                            gameData);
                        if (guiManager.mScreenStack.size() == 1)
                            guiManager.StartDialog(
                                e.mDialog,
                                false,
                                &dialogScene);
                    },
                },
                encounter);
        }

        //if (activeClickable != nullptr)
        //{
        //    auto bakLocation = activeClickable->GetBakLocation();

        //    ImGui::Begin("Clickable");
        //    std::stringstream ss{};
        //    ss << "Clickable: " << activeClickable->GetZoneItem().GetName() 
        //        << " Location: " << bakLocation;
        //    ImGui::Text(ss.str().c_str());

        //    ImGui::Separator();

        //    auto cit = std::find_if(containers.begin(), containers.end(),
        //        [&bakLocation](const auto& x){ return x.mLocation == bakLocation; });
        //    if (cit != containers.end())
        //        ShowContainerGui(*cit);

        //    ImGui::End();

        //    auto fit = std::find_if(fixedObjects.begin(), fixedObjects.end(),
        //        [&bakLocation](const auto& x){ return x.mLocation == bakLocation; });
        //    if (fit != fixedObjects.end())
        //        ShowDialogGui(fit->mDialogKey, dialogStore, gameData);
        //}

        ImguiWrapper::Draw(window.get());

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


