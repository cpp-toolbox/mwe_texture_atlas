#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "window/window.hpp"
#include "shader_cache/shader_cache.hpp"

#include <cstdio>
#include <cstdlib>

unsigned int SCREEN_WIDTH = 640;
unsigned int SCREEN_HEIGHT = 480;

static void error_callback(int error, const char *description) { fprintf(stderr, "Error: %s\n", description); }

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

struct OpenGLDrawingData {
    GLuint vbo_name;
    GLuint ibo_name;
    GLuint vao_name;
};

OpenGLDrawingData prepare_drawing_data_and_opengl_drawing_data(ShaderCache &shader_cache) {
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        0.5f,  0.5f,  0.0f, // top right
        0.5f,  -0.5f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, // bottom left
        -0.5f, 0.5f,  0.0f  // top left
    };
    GLuint indices[] = {
        // note that we start from 0!
        0, 1, 3, // first Triangle
        1, 2, 3  // second Triangle
    };

    // vbo: vertex buffer object
    // vao: vertex array object
    // ibo: index buffer object

    GLuint vbo_name, vao_name, ibo_name;

    glGenVertexArrays(1, &vao_name);
    glGenBuffers(1, &vbo_name);
    glGenBuffers(1, &ibo_name);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and
    // then configure vertex attributes(s).
    glBindVertexArray(vao_name);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_name);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_name);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    shader_cache.configure_vertex_attributes_for_drawables_vao(
        vao_name, vbo_name, ShaderType::ABSOLUTE_POSITION_WITH_SOLID_COLOR, ShaderVertexAttributeVariable::POSITION);

    // note that this is allowed, the call to glVertexAttribPointer registered
    // vbo_name as the vertex attribute's bound vertex buffer object so afterwards
    // we can safely unbind
    /*glBindBuffer(GL_ARRAY_BUFFER, 0);*/

    return {vbo_name, ibo_name, vao_name};
}

int main() {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("mwe_shader_cache_logs.txt", true);
    file_sink->set_level(spdlog::level::info);

    std::vector<spdlog::sink_ptr> sinks = {console_sink, file_sink};

    LiveInputState live_input_state;

    GLFWwindow *window = initialize_glfw_glad_and_return_window(&SCREEN_WIDTH, &SCREEN_HEIGHT, "glfw window", false,
                                                                false, false, &live_input_state);

    std::vector<ShaderType> requested_shaders = {ShaderType::ABSOLUTE_POSITION_WITH_SOLID_COLOR};
    ShaderCache shader_cache(requested_shaders, sinks);

    auto [vbo_name, ibo_name, vao_name] = prepare_drawing_data_and_opengl_drawing_data(shader_cache);

    int width, height;

    glm::vec4 color = glm::vec4(.5, .5, .5, 1);
    shader_cache.set_uniform(ShaderType::ABSOLUTE_POSITION_WITH_SOLID_COLOR, ShaderUniformVariable::RGBA_COLOR, color);

    while (!glfwWindowShouldClose(window)) {

        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_cache.use_shader_program(ShaderType::ABSOLUTE_POSITION_WITH_SOLID_COLOR);
        glBindVertexArray(vao_name); // seeing as we only have a single VAO there's
                                     // no need to bind it every time, but we'll do
                                     // so to keep things a bit more organized
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &vao_name);
    glDeleteBuffers(1, &vbo_name);
    glDeleteBuffers(1, &ibo_name);
    /*glDeleteProgram(shader_program);*/

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
