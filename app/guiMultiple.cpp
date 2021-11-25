#include "com/logger.hpp"

#include "com/ostream.hpp"

#include "graphics/inputHandler.hpp"
#include "graphics/glm.hpp"
#include "graphics/glfw.hpp"
#include "graphics/guiRenderer.hpp"

#include "gui/window.hpp"
#include "gui/icons.hpp"

#include "imgui/imguiWrapper.hpp"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <memory>
#include <sstream>

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);

    auto guiScalar = 4.0f;

    auto width = 1600.0f;
    auto height = 1000.0f;
    auto window = Graphics::MakeGlfwWindow(
        height,
        width,
        "BMX Explorer");

    glViewport(0, 0, width, height);

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

    auto icons = Gui::Icons{spriteManager};
    unsigned iconI = 0;
    auto picture = Gui::Widget{
        Gui::ImageTag{},
        Graphics::SpriteSheetIndex{1},
        Graphics::TextureIndex{0},
        glm::vec2{0,0},
        glm::vec2{100,100},
        true};

    root.AddChildBack(&picture);
    root.HideCursor();


    auto vao = Graphics::VertexArrayObject{};
    vao.BindGL();

    const auto& ss = spriteManager.GetSpriteSheet(Graphics::SpriteSheetIndex{1}); auto buffers = Graphics::GLBuffers{};
    logger.Debug() << "Made buffers\n";
    // vertices
    std::vector<glm::vec3> vertices{};
    std::vector<unsigned> indices{};
    auto transform = std::vector<glm::vec4>{};
    auto texCoords = std::vector<glm::vec4>{};
    auto colorModes = std::vector<unsigned>{};
    auto colors = std::vector<glm::vec4>{};
    auto params = std::vector<glm::ivec4>{};

    unsigned objs = 20;
    unsigned index = 0;
    auto pos = glm::vec2{0, 60};
    for (unsigned objectIndex = 1; objectIndex < (ss.mObjects.mTextureCoords.size() / 6 - 1); objectIndex++)
    {
        const auto dim1 = ss.GetDimensions(objectIndex % objs);
        pos += glm::vec2{dim1.x / 2, 0};

        const auto [o, l] = ss.mObjects.GetObject(0);
        for (unsigned i = 0; i < l; i++)
        {
            transform.emplace_back(guiScalar * glm::vec4{pos.x, pos.y, dim1.x, dim1.y});
            colorModes.emplace_back(0);
            colors.emplace_back(glm::vec4{1});
            params.emplace_back(glm::ivec4{0, objectIndex % objs, objectIndex, 0});
            vertices.emplace_back(ss.mObjects.mVertices[i]);
            indices.emplace_back(index++);
            const auto& tex = ss.mObjects.mTextureCoords[objectIndex * 6 + i];
            logger.Debug() << "ob: " << objectIndex << " i: " << i << " " << ss.mObjects.mTextureCoords.size() << std::endl;
            texCoords.emplace_back(glm::vec4{tex.x, tex.y, tex.z, 0});
        }
    }
    logger.Debug() << "Vertices: " << vertices << "\n";
    logger.Debug() << "Indices: " << indices << "\n";

    // view matrix
    const auto viewMatrix = guiRenderer.mCamera.mViewMatrix;

    logger.Debug() << " " << vertices.size() << " " << texCoords.size() << " " 
        << " " << colorModes.size() <<  " "<< colors.size() << " " << transform.size() << " " << ss.mSpriteDimensions.size() << "\n";

    using namespace Graphics;
    buffers.AddStaticArrayBuffer<glm::vec3>("vertex", GLLocation{0});
    buffers.AddStaticArrayBuffer<glm::vec4>("color", GLLocation{1});
    buffers.AddStaticArrayBuffer<glm::vec4>("transform", GLLocation{2});
    buffers.AddStaticArrayBuffer<glm::vec4>("params", GLLocation{3});
    buffers.AddStaticArrayBuffer<glm::vec4>("texCoords", GLLocation{4});

    buffers.AddElementBuffer("elements");

    buffers.LoadBufferDataGL("vertex", vertices);
    buffers.LoadBufferDataGL("color", colors);
    buffers.LoadBufferDataGL("transform", transform);
    buffers.LoadBufferDataGL("params", params);
    buffers.LoadBufferDataGL("texCoords", texCoords);

    buffers.LoadBufferDataGL("elements", indices);

    buffers.SetAttribDivisor("vertex", 0);
    buffers.SetAttribDivisor("texCoords", 0);
    buffers.SetAttribDivisor("color", 0);
    buffers.SetAttribDivisor("transform", 0);
    buffers.SetAttribDivisor("params", 0);

    buffers.BindArraysGL();


    auto shaderProgram = ShaderProgram{
        "guiMultiple.vert.glsl",
        //"geometry.glsl",
        "guiMultiple.frag.glsl"}.Compile();
    
    Graphics::InputHandler inputHandler{};
    Graphics::InputHandler::BindMouseToWindow(window.get(), inputHandler);
    Graphics::InputHandler::BindKeyboardToWindow(window.get(), inputHandler);
    bool useOriginal = true;
    inputHandler.Bind(GLFW_KEY_K, [&]{ useOriginal = !useOriginal; });
    unsigned k = 0;
    inputHandler.Bind(GLFW_KEY_R, [&]{
        auto params = std::vector<glm::ivec4>{};
        for (unsigned i = k++; i < ss.mSpriteDimensions.size(); i++)
            params.emplace_back(glm::ivec4{0, i % objs, i, 0});
        buffers.LoadBufferDataGL("params", params);

    });
    inputHandler.Bind(GLFW_KEY_RIGHT, [&]{
        iconI++;
        picture.SetTexture(Graphics::TextureIndex{iconI});
        logger.Debug() << "Pic: " << picture << "\n";
    });
    inputHandler.Bind(GLFW_KEY_LEFT, [&]{
        iconI--;
        const auto& [ss, ti, dims] = icons.GetInventoryLockIcon(iconI);
        picture.SetSpriteSheet(ss);
        picture.SetTexture(ti);
        picture.SetDimensions(dims);
        logger.Debug() << "Pic: " << picture << "\n";
    });

    double currentTime = 0;
    double lastTime = 0;
    float deltaTime = 0;

    glfwSetCursorPos(window.get(), width / 2, height / 2);

    glEnable(GL_MULTISAMPLE);  

    //glEnable(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    double pointerPosX, pointerPosY;

    double acc = 0;
    unsigned i = 0;
    do
    {
        currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        acc += deltaTime;
        if (acc > .2)
        { 
            i = (i + 1) % 20;
            acc = 0;
        }

        lastTime = currentTime;

        glfwPollEvents();
        glfwGetCursorPos(window.get(), &pointerPosX, &pointerPosY);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // { *** Draw 2D GUI ***
        if (useOriginal)
        {
            guiRenderer.RenderGui(&root);
        }
        else
        {
            vao.BindGL();
            shaderProgram.UseProgramGL();

            glActiveTexture(GL_TEXTURE0);
            ss.mTextureBuffer.BindGL();
            // bind tex...
            shaderProgram.SetUniform(
                shaderProgram.GetUniformLocation("V"),
                viewMatrix);
            shaderProgram.SetUniform(
                shaderProgram.GetUniformLocation("texture0"),
                0);
            glDrawElements(
                GL_TRIANGLES,
                6 * objs,
                GL_UNSIGNED_INT,
                (void*) 0);
        }

        glfwSwapBuffers(window.get());
    }
    while (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window.get()) == 0);

    ImguiWrapper::Shutdown();

    glfwTerminate();

    return 0;
}


