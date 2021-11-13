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
    {
        const auto [o, l] = ss.mObjects.GetObject(0);
        for (unsigned i = 0; i < l; i++)
            vertices.emplace_back(ss.mObjects.mVertices[i]);
    }
    logger.Debug() << "Vertices: " << vertices << "\n";

    // indices
    std::vector<unsigned> indices{};
    {
        const auto [o, l] = ss.mObjects.GetObject(0);
        for (unsigned i = 0; i < l; i++)
            indices.emplace_back(ss.mObjects.mIndices[i]);
    }

    logger.Debug() << "Indices: " << indices << "\n";

    // view matrix
    const auto viewMatrix = guiRenderer.mCamera.mViewMatrix;

    unsigned objs = 20;
    // Things that are differing
    // model matrices
    auto transform = std::vector<glm::vec4>{};
    {
        auto pos = glm::vec2{0, 60};
        for (unsigned i = 0; i < ss.mSpriteDimensions.size(); i++)
        {
            const auto dim1 = ss.GetDimensions(i % objs);
            transform.emplace_back(guiScalar * glm::vec4{pos.x, pos.y, dim1.x, dim1.y});
            pos += glm::vec2{dim1.x / 2, 0};
        }
    }

    // texture coords
    auto textureCoords = std::vector<glm::vec4>{};
    for (auto i = ss.mObjects.mTextureCoords.begin() + 6; i != ss.mObjects.mTextureCoords.end(); i++)
        textureCoords.emplace_back(glm::vec4{i->x, i->y, i->z, 0});

    // color modes
    auto colorModes = std::vector<unsigned>{};
    for (unsigned i = 0; i < ss.mSpriteDimensions.size(); i++)
        colorModes.emplace_back(0);

    // colors
    auto colors = std::vector<glm::vec4>{};
    for (unsigned i = 0; i < ss.mSpriteDimensions.size(); i++)
        colors.emplace_back(glm::vec4{1});

    auto params = std::vector<glm::ivec4>{};
    for (unsigned i = 0; i < ss.mSpriteDimensions.size(); i++)
        params.emplace_back(glm::ivec4{0, i % objs, i, 0});

    logger.Debug() << " " << vertices.size() << " " << textureCoords.size() << " " 
        << " " << colorModes.size() <<  " "<< colors.size() << " " << transform.size() << " " << ss.mSpriteDimensions.size() << "\n";

    using namespace Graphics;
    buffers.AddStaticArrayBuffer<glm::vec3>("vertex", GLLocation{0});
    buffers.AddStaticArrayBuffer<glm::vec4>("color", GLLocation{1});
    buffers.AddStaticArrayBuffer<glm::vec4>("transform", GLLocation{2});
    buffers.AddStaticArrayBuffer<glm::vec4>("params", GLLocation{3});

    buffers.AddElementBuffer("elements");
    buffers.AddTextureBuffer("textureCoords");

    buffers.LoadBufferDataGL("vertex", vertices);
    buffers.LoadBufferDataGL("color", colors);
    buffers.LoadBufferDataGL("transform", transform);
    // pack depth with color mode and texcoord
    buffers.LoadBufferDataGL("params", params);
    GLuint tbo;
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo);
    glBufferData(
        GL_TEXTURE_BUFFER,
        textureCoords.size() * sizeof(glm::vec4),
        &textureCoords.front(),
        GL_STATIC_DRAW);

    buffers.LoadBufferDataGL("elements", indices);
    buffers.LoadBufferDataGL("textureCoords", textureCoords);
    GLuint tbo_tex;
    glGenTextures(1, &tbo_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, tbo_tex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffers.GetGLBuffer("textureCoords").mBuffer.mValue);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    buffers.SetAttribDivisor("vertex", 0);
    buffers.SetAttribDivisor("color", 1);
    buffers.SetAttribDivisor("transform", 1);
    buffers.SetAttribDivisor("params", 1);

    buffers.BindArraysGL();


    auto shaderProgram = ShaderProgram{
        "instance.vert.glsl",
        //"geometry.glsl",
        "instance.frag.glsl"}.Compile();
    
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
    inputHandler.BindMouse(GLFW_MOUSE_BUTTON_LEFT, [&](auto p)
    {
        logger.Debug() << p << "\n";
    },
    [](auto){});

    double currentTime = 0;
    double lastTime = 0;
    float deltaTime = 0;

    glfwSetCursorPos(window.get(), width / 2, height / 2);

    glEnable(GL_MULTISAMPLE);  

    glEnable(GL_DEPTH_TEST);
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
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_BUFFER, tbo_tex);

            shaderProgram.UseProgramGL();
            glUniform1i(shaderProgram.GetUniformLocation("texCoords"), 1);

            glActiveTexture(GL_TEXTURE0);
            ss.mTextureBuffer.BindGL();
            // bind tex...
            shaderProgram.SetUniform(
                shaderProgram.GetUniformLocation("V"),
                viewMatrix);
            shaderProgram.SetUniform(
                shaderProgram.GetUniformLocation("texture0"),
                0);
            glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*) 0, objs);
        }

        glfwSwapBuffers(window.get());
    }
    while (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window.get()) == 0);

    ImguiWrapper::Shutdown();

    glfwTerminate();

    return 0;
}


