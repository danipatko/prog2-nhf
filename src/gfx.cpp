#include "gfx.h"
#include "GLFW/glfw3.h"
#include <glm/ext/vector_float2.hpp>

using namespace gfx;

static Window *app;
// static bool paused = false;

static void handlePointer(GLFWwindow *window, double xpos, double ypos) {
    app->onPointer(xpos, ypos);
}

static void handleScroll(GLFWwindow *window, double xoffset, double yoffset) {
    app->onScroll(xoffset, yoffset);
}

static void handleClick(GLFWwindow *window, int button, int action, int mods) {
    app->onClick(button, action, mods);
}

static void handleKey(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_UNKNOWN)
        return;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
        return;
    }

    app->onKey(key, scancode, action, mode);
}

Window::Window(const char *caption, const GLint width, const GLint height) {
    if (app != NULL)
        throw "There can only be a single window running at a time!";

    app = this;
    create(caption, width, height);
}

void Window::create(const char *caption, const GLuint width, const GLuint height) {
    std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
    // Init GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to start GLFW\n";
        exit(EXIT_FAILURE);
    }

    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, caption);

    WIDTH = width;
    HEIGHT = height;
    ASPECT = width / (float)height;

    // Create a GLFWwindow object that we can use for GLFW's functions
    window = glfwCreateWindow(WIDTH, HEIGHT, caption, NULL, NULL);
    glfwMakeContextCurrent(window);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
    int version = gladLoadGL();
    if (version == 0) {
        std::cerr << "Failed to initialize OpenGL context\n";
        exit(EXIT_FAILURE);
    }

    // Successfully loaded OpenGL
    std::cout << "Loaded OpenGL " << GLVersion.major << "." << GLVersion.minor << std::endl;

    glViewport(0, 0, WIDTH, HEIGHT);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    // enable multisampling for smoother edges
    glfwWindowHint(GLFW_SAMPLES, 4);
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    configureListeners(window);
}

void Window::configureListeners(GLFWwindow *window) {
    glfwSetScrollCallback(window, handleScroll);
    glfwSetMouseButtonCallback(window, handleClick);
    glfwSetCursorPosCallback(window, handlePointer);
    glfwSetKeyCallback(window, handleKey);
}

int Window::loop() {
    onStart();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        float now = (float)glfwGetTime();
        deltaTime = now - lastFrameTime;
        lastFrameTime = now;

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        onFrame();

        glfwSwapBuffers(window);
    }

    onClose();
    // TOFIX: this segfaults
    // glfwTerminate();
    return 0;
}

//
