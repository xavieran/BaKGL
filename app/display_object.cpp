#include "com/logger.hpp"

#include "bak/palette.hpp"
#include "bak/camera.hpp"
#include "bak/worldFactory.hpp"
#include "bak/zone.hpp"

#include "game/systems.hpp"

#include "graphics/glfw.hpp"
#include "graphics/inputHandler.hpp"
#include "graphics/renderer.hpp"
#include "graphics/meshObject.hpp"
#include "graphics/opengl.hpp"
#include "graphics/shaderProgram.hpp"

#include "imgui/imguiWrapper.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_set>
#include <vector>

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("display_object");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    Logging::LogState::SetLogTime(false);
    Logging::LogState::Disable("Combat");
    Logging::LogState::Disable("CreateFileBuffer");
    Logging::LogState::Disable("LoadFixedObjects");
    Logging::LogState::Disable("MeshObjectStore");
    //Logging::LogState::Disable("ZoneItemToMeshObject");
    Logging::LogState::Disable("World");
    Logging::LogState::Disable("LoadEncounter");
    Logging::LogState::Disable("ZoneFactory");
    Logging::LogState::Disable("ShaderProgram");
    Logging::LogState::Disable("GLBuffers");

    if (argc != 2)
    {
        logger.Error() << "Call with <ZONE>" << std::endl;
        std::exit(1);
    }

    auto zoneData = std::make_unique<BAK::Zone>(std::atoi(argv[1]));

    auto guiScalar = 6.0f;

    auto nativeWidth = 320.0f;
    auto nativeHeight = 200.0f;

    auto width = nativeWidth * guiScalar;
    auto height = nativeHeight * guiScalar;

    auto window = Graphics::MakeGlfwWindow(
        height,
        width,
        "BaK");

    ImguiWrapper::Initialise(window.get());

    auto renderer = Graphics::Renderer{
        width,
        height,
        1024,
        1024};
    auto renderData = Graphics::RenderData{};
    renderData.LoadData(
        zoneData->mObjects,
        zoneData->mZoneTextures.GetTextures(),
        zoneData->mZoneTextures.GetMaxDim());

    Camera lightCamera{
        static_cast<unsigned>(width),
        static_cast<unsigned>(height),
        400 * 30.0f,
        2.0f};
    lightCamera.UseOrthoMatrix(400, 400);

    Camera camera{
        static_cast<unsigned>(width),
        static_cast<unsigned>(height),
        50*30.0f,
        2.0f};
    Camera* cameraPtr = &camera;
    camera.SetPosition({-10, 0, -10});
    Graphics::Light light{
        glm::vec3{.2, -.1, .05},
        glm::vec3{.5, .5, .5},
        glm::vec3{1,.85,.87},
        glm::vec3{.2,.2,.2},
        .0005f,
        glm::vec3{.15, .31, .36}
    };

    auto systems = Systems{};

    std::vector<std::string> objectNames;
    objectNames.reserve(zoneData->mObjects.mObjects.size());
    for (const auto& [name, offsetAndLength] : zoneData->mObjects.mObjects)
        objectNames.emplace_back(name);
    std::sort(objectNames.begin(), objectNames.end());

    std::unordered_set<std::string> zoneItemNames;
    for (const auto& item : zoneData->mZoneItems.GetItems())
        zoneItemNames.insert(item.GetName());

    int currentIndex = -1;
    std::optional<BAK::EntityIndex> currentRenderableId;

    auto selectObject = [&](int idx)
    {
        if (currentRenderableId)
        {
            systems.RemoveRenderable(*currentRenderableId);
            currentRenderableId.reset();
        }

        if (idx < 0 || idx >= static_cast<int>(objectNames.size()))
            return;

        auto renderable = Renderable{
            systems.GetNextItemId(),
            zoneData->mObjects.GetObject(objectNames[idx]),
            {0,0,0},
            {0,0,0},
            glm::vec3{1.0f}};
        systems.AddRenderable(renderable);
        currentRenderableId = renderable.GetId();
    };

    if (!objectNames.empty())
    {
        currentIndex = 0;
        selectObject(currentIndex);
    }

    glfwSetCursorPos(window.get(), width/2, height/2);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);  

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    Graphics::InputHandler inputHandler{};
    inputHandler.Bind(GLFW_KEY_W, [&]{ cameraPtr->MoveForward(); });
    inputHandler.Bind(GLFW_KEY_S, [&]{ cameraPtr->MoveBackward(); });
    inputHandler.Bind(GLFW_KEY_A, [&]{ cameraPtr->StrafeLeft(); });
    inputHandler.Bind(GLFW_KEY_D, [&]{ cameraPtr->StrafeRight(); });
    inputHandler.Bind(GLFW_KEY_X, [&]{ cameraPtr->RotateVerticalUp(); });
    inputHandler.Bind(GLFW_KEY_Y, [&]{ cameraPtr->RotateVerticalDown(); });
    inputHandler.Bind(GLFW_KEY_Q, [&]{ cameraPtr->RotateLeft(); });
    inputHandler.Bind(GLFW_KEY_E, [&]{ cameraPtr->RotateRight(); });
    Graphics::InputHandler::BindKeyboardToWindow(window.get(), inputHandler);
    Graphics::InputHandler::BindMouseToWindow(window.get(), inputHandler);

    char filterBuf[128] = "";

    double currentTime;
    double lastTime = 0;
    float deltaTime;

    int prevUp = GLFW_RELEASE;
    int prevDown = GLFW_RELEASE;

    do
    {
        currentTime = glfwGetTime();

        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        cameraPtr->SetDeltaTime(deltaTime);

        inputHandler.HandleInput(window.get());

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(300, height));
            ImGui::Begin("Objects", nullptr,
                ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoCollapse);

            ImGui::Text("Zone %d - %zu objects",
                std::atoi(argv[1]),
                objectNames.size());
            ImGui::InputText("Filter", filterBuf, sizeof(filterBuf));
            const bool filterActive = ImGui::IsItemActive();

            ImGui::BeginChild("ObjectList", ImVec2(0, -60), true);

            auto matchesFilter = [](const std::string& name, const std::string& filter)
            {
                if (filter.empty())
                    return true;

                auto lower = [](std::string s)
                {
                    std::transform(s.begin(), s.end(), s.begin(),
                        [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
                    return s;
                };

                auto n = lower(name);
                auto f = lower(filter);
                return n.find(f) != std::string::npos;
            };

            std::string filterStr = filterBuf;
            std::vector<int> filteredIndices;
            for (int i = 0; i < static_cast<int>(objectNames.size()); ++i)
                if (matchesFilter(objectNames[i], filterStr))
                    filteredIndices.emplace_back(i);

            {
                const int curUp   = glfwGetKey(window.get(), GLFW_KEY_UP);
                const int curDown = glfwGetKey(window.get(), GLFW_KEY_DOWN);

                const bool upPressed   = curUp   == GLFW_PRESS && prevUp   == GLFW_RELEASE;
                const bool downPressed = curDown == GLFW_PRESS && prevDown == GLFW_RELEASE;

                if (!filterActive && !filteredIndices.empty())
                {
                    auto itPos = std::find(
                        filteredIndices.begin(),
                        filteredIndices.end(),
                        currentIndex);
                    int pos = itPos == filteredIndices.end()
                        ? -1
                        : static_cast<int>(itPos - filteredIndices.begin());

                    int newPos = pos;
                    if (upPressed)
                        newPos = pos < 0
                            ? static_cast<int>(filteredIndices.size()) - 1
                            : (pos - 1 + static_cast<int>(filteredIndices.size()))
                                % static_cast<int>(filteredIndices.size());
                    else if (downPressed)
                        newPos = pos < 0
                            ? 0
                            : (pos + 1) % static_cast<int>(filteredIndices.size());

                    if (newPos != pos)
                    {
                        currentIndex = filteredIndices[newPos];
                        selectObject(currentIndex);
                    }
                }

                prevUp = curUp;
                prevDown = curDown;
            }

            for (int i : filteredIndices)
            {
                bool selected = (i == currentIndex);
                if (ImGui::Selectable(objectNames[i].c_str(), selected))
                {
                    currentIndex = i;
                    selectObject(i);
                }

                if (selected)
                {
                    const float rowHeight = ImGui::GetTextLineHeightWithSpacing();
                    const float scrollY = ImGui::GetScrollY();
                    const float visibleHeight = ImGui::GetWindowHeight();
                    const float cursorY = ImGui::GetCursorPosY() - rowHeight;
                    if (cursorY < scrollY)
                        ImGui::SetScrollHereY(0.0f);
                    else if (cursorY > scrollY + visibleHeight - rowHeight * 3.0f)
                        ImGui::SetScrollHereY(1.0f);
                }
            }

            ImGui::EndChild();

            if (currentIndex >= 0
                && currentIndex < static_cast<int>(objectNames.size()))
            {
                const auto& name = objectNames[currentIndex];
                if (zoneItemNames.contains(name))
                {
                    const auto& item = zoneData->mZoneItems.GetZoneItem(name);
                    ImGui::Text("Entity Type: %s",
                        std::string{BAK::ToString(item.GetEntityType())}.c_str());
                    ImGui::Text("Terrain Type: %s",
                        std::string{BAK::ToString(item.GetTerrainType())}.c_str());
                }
                else
                {
                    ImGui::TextDisabled("Entity Type: N/A");
                    ImGui::TextDisabled("Terrain Type: N/A");
                }
            }

            ImGui::End();
        }

        glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
        // Dark blue background
        glClearColor(0.15f, 0.31f, 0.36f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.DrawWithShadow(
            renderData,
            systems.GetRenderables(),
            light,
            lightCamera,
            *cameraPtr,
            false);

        // Render ImGui on top of the 3D scene
        ImguiWrapper::Draw(window.get());

        // Swap buffers
        glfwSwapBuffers(window.get());
        glfwPollEvents();

    } // Check if the window was closed
    while (glfwWindowShouldClose(window.get()) == 0 );
    
    ImguiWrapper::Shutdown();

    return 0;
}