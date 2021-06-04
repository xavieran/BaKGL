#include "bak/logger.hpp"

#include "bak/worldFactory.hpp"

#include "graphics/meshObject.hpp"
#include "graphics/renderer.hpp"
#include "graphics/shaderProgram.hpp"

#include "xbak/FileManager.h"
#include "xbak/FileBuffer.h"
#include "xbak/PaletteResource.h"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("display_object");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    //Logging::LogState::Disable("MeshObjectStore");

    if (argc != 3)
    {
        logger.Error() << "Call with <ZONE> <OBJECT>" << std::endl;
        std::exit(1);
    }

    BAK::ZoneLabel zoneLabel{argv[1]};
    auto objectToDisplay = argv[2];

    auto palz = std::make_unique<PaletteResource>();
    FileManager::GetInstance()->Load(palz.get(), zoneLabel.GetPalette());
    auto& pal = *palz->GetPalette();

    auto textureStore  = BAK::ZoneTextureStore{zoneLabel, pal};
    auto zoneItems = BAK::ZoneItemStore{zoneLabel, textureStore};

    auto objStore = Graphics::MeshObjectStorage{};
    
    for (auto& item : zoneItems.GetItems())
    {
        if (item.GetVertices().size() <= 1) continue;

        if (item.GetName() == objectToDisplay)
        {
            objStore.AddObject(
                item.GetName(),
                BAK::ZoneItemToMeshObject(item, textureStore, pal));
        }
    }


    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    GLFWwindow* window;

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    unsigned height = 600;
    unsigned width  = 800;

    window = glfwCreateWindow( width, height, "BaK", NULL, NULL);
    if( window == NULL )
    {
        logger.Log(Logging::LogLevel::Error) << "Failed to open GLFW window" << std::endl;
        glfwTerminate();
        return -1;
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    //
    auto shaderProgram = ShaderProgram{
        "vertex.glsl",
        //"geometry.glsl",
        "fragment.glsl"};
    
    auto programId = shaderProgram.Compile();

    const auto& vertices      = objStore.mVertices;
    const auto& normals       = objStore.mNormals;
    const auto& colors        = objStore.mColors;
    const auto& textureCoords = objStore.mTextureCoords;
    const auto& textureBlends = objStore.mTextureBlends;
    const auto& indices       = objStore.mIndices;

    Graphics::GLBuffers buffers{};
    buffers.AddBuffer("vertex", 0, 3);
    buffers.AddBuffer("normal", 1, 3);
    buffers.AddBuffer("color", 2, 4);
    buffers.AddBuffer("textureCoord", 3, 3);
    buffers.AddBuffer("textureBlend", 4, 1);

    buffers.LoadBufferDataGL("vertex", GL_ARRAY_BUFFER, vertices);
    buffers.LoadBufferDataGL("normal", GL_ARRAY_BUFFER, normals);
    buffers.LoadBufferDataGL("color", GL_ARRAY_BUFFER, colors);
    buffers.LoadBufferDataGL("textureCoord", GL_ARRAY_BUFFER, textureCoords);
    buffers.LoadBufferDataGL("textureBlend", GL_ARRAY_BUFFER, textureBlends);

    buffers.LoadBufferDataGL(buffers.mElementBuffer, GL_ELEMENT_ARRAY_BUFFER, indices);
    buffers.BindArraysGL();
    glBindVertexArray(0);

    Graphics::TextureBuffer textureBuffer{};
    textureBuffer.LoadTexturesGL(textureStore.GetTextures(), textureStore.GetMaxDim());

    /*

    auto vertices = std::vector<glm::vec3>{};
    for (auto i : {
        glm::vec3{0.0, 1.0, 0.0},
        glm::vec3{1.0, 1.0, 0.0},
        glm::vec3{1.0, 0.0, 0.0},
        glm::vec3{0.0, 0.0, 0.0}})
        vertices.emplace_back(i);

    auto normals = std::vector<glm::vec3>{};
    for (auto i : {
        glm::vec3{0.0, 0.0, 1.0},
        glm::vec3{0.0, 0.0, 1.0},
        glm::vec3{0.0, 0.0, 1.0},
        glm::vec3{0.0, 0.0, 1.0}})
        normals.emplace_back(i);

    auto colors = std::vector<glm::vec3>{};
    for (auto i : {
        glm::vec3{0,0,0},
        glm::vec3{0,0,0},
        glm::vec3{0,0,0},
        glm::vec3{0,0,0}})
        colors.emplace_back(i);

    auto textureCoords = std::vector<glm::vec3>{};
    for (auto i : {
        glm::vec3{0, 1, 0},
        glm::vec3{1, 1, 0},
        glm::vec3{1, 0, 0},
        glm::vec3{0, 0, 0}})
        textureCoords.emplace_back(i);

    auto indices = std::vector<unsigned>{};
    for (auto i : {
        0,1,2,0,3,2})
        indices.emplace_back(i);
    */

    GLuint textureID     = glGetUniformLocation(programId.GetHandle(), "texture0");
    GLuint mvpMatrixID   = glGetUniformLocation(programId.GetHandle(), "MVP");
    GLuint modelMatrixID = glGetUniformLocation(programId.GetHandle(), "M");
    GLuint viewMatrixID  = glGetUniformLocation(programId.GetHandle(), "V");

    glm::mat4 projectionMatrix = glm::perspective(
        glm::radians(45.0f),
        (float) width / (float)height,
        0.1f,
        4000.0f);
      
    // Camera matrix
    glm::mat4 viewMatrix = glm::lookAt(
        glm::vec3(0,0,0), // eye pos
        glm::vec3(0,0,0), // look pos
        glm::vec3(0,1,0)  // orientation
        );
      
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    // Our ModelViewprojectionMatrix : multiplication of our 3 matrices
    glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix; 

    glm::vec3 position = glm::vec3( 0, 1.8, 0 );
    glm::vec3 lightPos = glm::vec3(0,20,0);
    // horizontal angle : toward -Z
    float horizontalAngle = 3.14f;
    // vertical angle : 0, look at the horizon
    float verticalAngle = 0.0f;
    // Initial Field of View
    float initialFoV = 45.0f;
    
    float speed = 6.0f; 
    glm::vec3 direction;
    glm::vec3 right;
    glm::vec3 up;

    double currentTime;
    double lastTime = 0;
    float deltaTime;

    GLuint LightID = glGetUniformLocation(programId.GetHandle(), "lightPosition_worldspace");
    GLuint CameraPositionID = glGetUniformLocation(programId.GetHandle(), "cameraPosition_worldspace");

    // Setup active arrays and textures
    glUseProgram(programId.GetHandle());

    glEnable(GL_MULTISAMPLE);  

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureBuffer.mTextureBuffer);
    glUniform1i(textureID, 0);

    do
    {
        glBindVertexArray(VertexArrayID);

        currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;
        
        direction = {
            cos(verticalAngle) * sin(horizontalAngle),
            sin(verticalAngle),
            cos(verticalAngle) * cos(horizontalAngle)
        };

        right = {
            sin(horizontalAngle - 3.14f/2.0f),
            0,
            cos(horizontalAngle - 3.14f/2.0f)
        };

        up = glm::cross(right, direction);

        if (glfwGetKey( window, GLFW_KEY_W) == GLFW_PRESS){
            position += direction * deltaTime * speed;
            logger.Info() << "Pos: " << glm::to_string(position) << std::endl;
        }
        // Move backward
        if (glfwGetKey( window, GLFW_KEY_S) == GLFW_PRESS){
        position -= direction * deltaTime * speed;
        }
        // Strafe right
        if (glfwGetKey( window, GLFW_KEY_D) == GLFW_PRESS){
            position += right * deltaTime * speed;
        }
        // Strafe left
        if (glfwGetKey( window, GLFW_KEY_A) == GLFW_PRESS){
            position -= right * deltaTime * speed;
        }

        // Rotate left
        if (glfwGetKey( window, GLFW_KEY_Q) == GLFW_PRESS){
            horizontalAngle += deltaTime * (speed);
        }
        // Rotate left
        if (glfwGetKey( window, GLFW_KEY_E) == GLFW_PRESS){
            horizontalAngle -= deltaTime * (speed);
        }

        if (glfwGetKey( window, GLFW_KEY_X) == GLFW_PRESS){
            verticalAngle += deltaTime * (speed);
        }

        if (glfwGetKey( window, GLFW_KEY_Y) == GLFW_PRESS){
            verticalAngle -= deltaTime * (speed);
        }
        
        auto lightOffset = glm::vec3{
            glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS ? -1.0 
            : glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS ? 1.0 : 0,
            glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS ? -1.0 
            : glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS ? 1.0 : 0,
            glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS ? -1.0 
            : glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS ? 1.0 : 0};
        lightPos += lightOffset;

        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(CameraPositionID, position.x, position.y, position.z);

        viewMatrix = glm::lookAt(
            position,
            position + direction,
            up);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto [offset, length] = objStore.GetObject(objectToDisplay);
        //auto offset = 0;
        //auto length = 6;

        modelMatrix = glm::mat4(1.0f);
        
        MVP = projectionMatrix * viewMatrix * modelMatrix;

        glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, glm::value_ptr(MVP));
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, glm::value_ptr(viewMatrix));

        glDrawElementsBaseVertex(
            GL_TRIANGLES,
            length,
            GL_UNSIGNED_INT,
            (void*) (offset * sizeof(GLuint)),
            offset
        );

        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, lightPos);
        
        MVP = projectionMatrix * viewMatrix * modelMatrix;

        glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, glm::value_ptr(MVP));
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, glm::value_ptr(viewMatrix));

        glDrawElementsBaseVertex(
            GL_TRIANGLES,
            length,
            GL_UNSIGNED_INT,
            (void*) (offset * sizeof(GLuint)),
            offset
        );




        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS 
        && glfwWindowShouldClose(window) == 0 );
    
    // If I need to swap shaders I will need 
    // to move this back into the main loop
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);

    // Cleanup VBO
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
