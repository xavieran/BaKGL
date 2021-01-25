#include "logger.hpp"

#include "loadShaders.hpp"

#include "meshObject.hpp"
#include "renderer.hpp"
#include "worldFactory.hpp"

#include "FileManager.h"
#include "FileBuffer.h"
#include "PaletteResource.h"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/range/adaptor/indexed.hpp>

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

    auto textures  = BAK::TextureStore{zoneLabel, pal};
    auto zoneItems = BAK::ZoneItemStore{zoneLabel, textures};

    auto objStore = BAK::MeshObjectStorage{};

    for (const auto& item : zoneItems.GetItems())
    {
        auto obj = BAK::MeshObject();
        if (item.GetVertices().size() <= 1) continue;

        if (item.GetName() == objectToDisplay)
        {
            std::stringstream ss{""};
            for (unsigned i = 0; i < item.GetColors().size(); i++)
            {
                auto p = item.GetPalettes()[i];
                auto c = item.GetColors()[i];
                ss << i << " p: 0x" << std::hex << +p << " " << std::dec << +p
                    << " c: 0x" << std::hex << +c << " " << std::dec << +c << std::endl;
            }
            logger.Info() << "Colors and Palettes" << std::endl
                << ss.str() << std::endl;
        }

        obj.LoadFromBaKItem(item, textures, pal);
        objStore.AddObject(item.GetName(), obj);
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

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders(
        std::string{"vertex.glsl"},
        std::string{"fragment.glsl"});
    
    const auto& vertices      = objStore.mVertices;
    const auto& normals       = objStore.mNormals;
    const auto& colors        = objStore.mColors;
    const auto& textureCoords = objStore.mTextureCoords;
    const auto& textureBlends = objStore.mTextureBlends;
    const auto& indices       = objStore.mIndices;

    BAK::GLBuffers buffers{};
    buffers.LoadBufferDataGL(buffers.mVertexBuffer, GL_ARRAY_BUFFER, vertices);
    buffers.LoadBufferDataGL(buffers.mNormalBuffer, GL_ARRAY_BUFFER, normals);
    buffers.LoadBufferDataGL(buffers.mColorBuffer, GL_ARRAY_BUFFER, colors);
    buffers.LoadBufferDataGL(buffers.mTextureCoordBuffer, GL_ARRAY_BUFFER, textureCoords);
    buffers.LoadBufferDataGL(buffers.mTextureBlendBuffer, GL_ARRAY_BUFFER, textureBlends);
    buffers.LoadBufferDataGL(buffers.mElementBuffer, GL_ELEMENT_ARRAY_BUFFER, indices);

    BAK::TextureBuffer textureBuffer{};
    textureBuffer.LoadTexturesGL(textures);

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

    GLuint textureID     = glGetUniformLocation(programID, "texture0");
    GLuint mvpMatrixID   = glGetUniformLocation(programID, "MVP");
    GLuint modelMatrixID = glGetUniformLocation(programID, "M");
    GLuint viewMatrixID  = glGetUniformLocation(programID, "V");

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
    glm::vec3 lightPos = glm::vec3(0,320,0);
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

    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // Setup active arrays and textures
    glUseProgram(programID);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    buffers.BindArraysGL();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureBuffer.mTextureBuffer);
    glUniform1i(textureID, 0);

    do
    {
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

        if (glfwGetKey( window, GLFW_KEY_P) == GLFW_PRESS){
            lightPos.y += .5;
        }
        if (glfwGetKey( window, GLFW_KEY_L) == GLFW_PRESS){
            lightPos.y -= .5;
        }

        lightPos.x = position.x;
        lightPos.z = position.z;

        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

        viewMatrix = glm::lookAt(
            position,
            position + direction,
            up);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto [offset, length] = objStore.GetObject(objectToDisplay);
        //auto offset = 0;
        //auto length = 6;

        modelMatrix = glm::mat4(1.0f);
        
        //modelMatrix = glm::rotate(modelMatrix, , glm::vec3(0,-1,0));

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
    glDeleteProgram(programID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
