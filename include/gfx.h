#ifndef GFX_H
#define GFX_H

#include <cstdlib>
#include <glad/glad.h>
// -> this comment is here to prevent autoformat from changing the order of glad and glfw
#include <GLFW/glfw3.h>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace gfx {

namespace color {
static const glm::vec3 BLACK(0.0f, 0.0f, 0.0f);
static const glm::vec3 ORANGERED(1.f, 25 / 255.f, 0.0f);
static const glm::vec3 WHITE(1.0f, 1.0f, 1.0f);
static const glm::vec3 RED(1.0f, 0.0f, 0.0f);
static const glm::vec3 GREEN(0.0f, 1.0f, 0.0f);
static const glm::vec3 BLUE(0.0f, 0.0f, 1.0f);
static const glm::vec3 CYAN(0.0f, 1.0f, 1.0f);
static const glm::vec3 MAGENTA(1.0f, 0.0f, 1.0f);
static const glm::vec3 YELLOW(1.0f, 1.0f, 0.0f);
static const glm::vec3 GRAY(0.5f, 0.5f, 0.5f);
static const glm::vec3 LIGHT_GRAY(0.75f, 0.75f, 0.75f);
static const glm::vec3 DARK_GRAY(0.25f, 0.25f, 0.25f);
static const glm::vec3 ORANGE(1.0f, 0.5f, 0.0f);
static const glm::vec3 PURPLE(0.5f, 0.0f, 0.5f);
static const glm::vec3 BROWN(0.6f, 0.3f, 0.1f);
} // namespace color

static const GLint DEFAULT_WIDTH = 1980, DEFAULT_HEIGHT = 1080;

/**
 * Render Window
 */
class Window {
    float lastFrameTime;

  public:
    float ASPECT;

  protected:
    GLFWwindow *window;

    bool screenRefresh = false;

    GLuint WIDTH;
    GLuint HEIGHT;

    void create(const char *caption, const GLuint width, const GLuint height);

    virtual void configureListeners(GLFWwindow *window);

    virtual void onStart() {}

    virtual void onClose() {}

    virtual void onFrame() {}

    float deltaTime;

  public:
    virtual void onKey(int key, int scancode, int action, int mode) {}

    virtual void onClick(int button, int action, int mods) {}

    virtual void onScroll(double xoffset, double yoffset) {}

    virtual void onPointer(double xoffset, double yoffset) {}

    Window(const char *caption = "NHF", const GLint width = DEFAULT_WIDTH, const GLint height = DEFAULT_HEIGHT);

    int loop();

    virtual ~Window() {}
};

/**
 * @brief Helper class for storing a vertex and fragment shader pair.
 */
class Shader {
  public:
    /**
     * Shader program ID, returned by glCreateProgram()
     */
    GLuint shaderProgram = 0;

  private:
    /**
     * @brief check if shader source code is valid
     * @param shader the shader ID
     * @return true if shader is valid
     */
    bool checkShader(unsigned int shader, const char *TAG = "TAG") {
        GLint length = 0, result = 0;

        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

        if (!result) {
            std::string errlog(length, '\0');
            glGetShaderInfoLog(shader, length, 0, (GLchar *)errlog.data());
            std::cout << "failed to compile shader " << TAG << ":\n" << errlog << "\n";

            return false;
        }

        return true;
    }

    /**
     * @brief check if shaders are linked correctly
     * @param program the shader program ID
     * @return true if the linking is successful
     */
    bool checkLinking(unsigned int program) {
        GLint length = 0, result = 0;

        glGetProgramiv(program, GL_LINK_STATUS, &result);
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

        if (!result) {
            std::string errlog(length, '\0');
            glGetProgramInfoLog(program, length, 0, (GLchar *)errlog.data());
            std::cout << errlog << "\n";

            return false;
        }

        return true;
    }

    /**
     * @brief shader compile helper function
     * @param type the shader type (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER)
     * @param src shader source code
     * @throws const char* exception if compilation fails
     * @returns shader ID
     */
    GLuint compileShader(GLenum type, const char *src, const char *TAG = "TAG") {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        if (!checkShader(shader, TAG)) {
            throw "Shader compilation failed";
        }

        return shader;
    }

  public:
    /**
     * Constructor
     * @throws const char* exception if shader compilation or linking fails
     */
    Shader(const char *vertexShaderSource, const char *fragmentShaderSource, const char *geometryShaderSource = nullptr) {
        GLuint vert = compileShader(GL_VERTEX_SHADER, vertexShaderSource, "vertexShaderSource");
        GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource, "fragmentShaderSource");

        GLuint geom = 0;

        shaderProgram = glCreateProgram();
        if (!shaderProgram) {
            std::cerr << "ShaderProgram was not created!\n";
            exit(EXIT_FAILURE);
        }

        glAttachShader(shaderProgram, vert);
        glAttachShader(shaderProgram, frag);

        if (geometryShaderSource != nullptr) {
            geom = compileShader(GL_GEOMETRY_SHADER, geometryShaderSource, "geometryShaderSource");
            glAttachShader(shaderProgram, geom);
        }

        glLinkProgram(shaderProgram);

        if (!checkLinking(shaderProgram)) {
            std::cerr << "Failed to link\n";
            exit(EXIT_FAILURE);
        }

        glDeleteShader(vert);
        glDeleteShader(frag);
        if (geom != 0)
            glDeleteShader(geom);
    }

    std::unordered_map<std::string, int> locations;

    /**
     * @brief Locate uniform locations and save/load cache
     */
    int locate(const std::string &name) {
        // load from memory
        if (locations.count(name) > 0)
            return locations[name];

        int location = glGetUniformLocation(shaderProgram, name.c_str());
        if (location < 0)
            throw "uniform name could not be found!";

        locations[name] = location;
        return location;
    }

    void use() {
        glUseProgram(shaderProgram);
    }

    /**
     * @brief set uniform variable in shader program (for vec3 type)
     * @param v the vector to set
     * @param name name of the uniform variable
     */
    void setUniform(const std::string &name, const glm::vec3 &v) {
        glUniform3fv(locate(name.c_str()), 1, &v.x);
    }

    /**
     * @brief set uniform variable in shader program (for vec4 type)
     * @param v the vector to set
     * @param name name of the uniform variable
     */
    void setUniform(const std::string &name, const glm::vec4 &v) {
        glUniform4fv(locate(name.c_str()), 1, &v.x);
    }

    /**
     * @brief set uniform variable in shader program (for mat4 type)
     * @param mat name of the uniform variable (mat4)
     * @param name name of the uniform variable
     */
    void setUniform(const std::string &name, const glm::mat4 &mat) {
        glUniformMatrix4fv(locate(name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }

    /**
     * @brief set uniform variable in shader program (for float type)
     * @param f name of the uniform variable (float)
     * @param name name of the uniform variable
     */
    void setUniform(const std::string &name, const float f) {
        glUniform1f(locate(name.c_str()), f);
    }

    /**
     * @brief Destructor
     * Deletes the shader program
     */
    ~Shader() {
        if (shaderProgram != 0)
            glDeleteProgram(shaderProgram);
    }
};

/**
 * @brief Helper class for reusable drawable components
 */
template <typename T> class Drawable {
    GLuint VAO;
    GLuint VBO;

    /**
     * @brief Vertex data
     */
    std::vector<T> vertices;

  protected:
    void __draw__(Shader *sh, GLenum type) {
        glBindVertexArray(VAO);
        glDrawArrays(type, 0, (int)vertices.size());
    }

    /// CPU -> GPU
    void __update__() {
        if (vertices.size() == 0)
            return;

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), &vertices[0], GL_DYNAMIC_DRAW);
    }

    std::vector<T> &vtx() {
        return vertices;
    }

  public:
    Drawable() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
    }

    virtual void draw(Shader *sh) {
        __draw__(sh, GL_LINES);
    }

    void bind() {
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
    }

    virtual ~Drawable() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }
};

/**
 * @brief Event handler
 */
class EventHandler {
    /**
     * @brief handle pointer move event
     * @param window the window
     * @param xpos x position of the pointer
     * @param ypos y position of the pointer
     */
    virtual void handlePointer(GLFWwindow *window, double xpos, double ypos) {};

    /**
     * @brief handle a keypress
     * @note escape key automatically close window
     * @param window the window
     */
    virtual void handleKey(GLFWwindow *window, int key, int scancode, int action, int mode) {};

    /**
     * @brief handle scroll event
     * @param window the window
     * @param xoffset horizontal difference
     * @param yoffset vertical difference
     */
    virtual void handleScroll(GLFWwindow *window, double xoffset, double yoffset) {};

    /**
     * @brief handle click event
     * @param window the window
     * @param button the button clicked
     * @param action the action (pressed/released)
     * @param mods the modifiers (shift, ctrl, alt)
     */
    virtual void handleClick(GLFWwindow *window, int button, int action, int mods) {};

    // virtual void handleClick(GLFWwindow *window, int button, int action, int mods) {};
};

class Zoom : public EventHandler {
    float _zoom;

  public:
    Zoom() : _zoom(1.f) {};

    float get() {
        return _zoom;
    }

    void set(float zoom) {
        _zoom = zoom;
    }

    void handleScroll(GLFWwindow *window, double xoffset, double yoffset) override {
        const static float zoomFactor = 1.025f;
        _zoom *= (yoffset > 0) ? zoomFactor : 1.0f / zoomFactor;
    }
};

class Drag : public EventHandler {
    bool dragging;
    double _lastX, _lastY, _dx, _dy;

  public:
    Drag() : dragging(false) {};

    double lastX() {
        return _lastX;
    }

    double lastY() {
        return _lastY;
    }

    double dx() {
        return _dx;
    }

    double dy() {
        return _dy;
    }

    void handlePointer(GLFWwindow *window, double xpos, double ypos) override {
        if (!dragging) {
            _dx = _dy = 0;
            return;
        }

        _dx = _lastX - xpos;
        _dy = _lastY - ypos;

        // std::cout << "dx = " << _dx << ", dy = " << _dy << "\n";

        _lastX = xpos;
        _lastY = ypos;
    }

    void handleClick(GLFWwindow *window, int button, int action, int mods) override {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                dragging = true;
                glfwGetCursorPos(window, &_lastX, &_lastY);
            } else if (action == GLFW_RELEASE) {
                dragging = false;
            }
        }
    }
};

struct Gesture {
    /**
     * @brief perform a transformation on the view matrix
     * @param view the view matrix
     */
    virtual void transform(glm::mat4 &view, const GLint WIDTH, const GLint HEIGHT) = 0;
};

class Panzoom : public Gesture, public EventHandler {
    glm::vec2 pan = {0.f, 0.f};

    Zoom zoom;
    Drag drag;

  public:
    /**
     * @brief perform a transformation on the view matrix
     * @param view the view matrix
     */
    void transform(glm::mat4 &view, const GLint WIDTH, const GLint HEIGHT) override {
        // Normalize to [-1, 1] range (in screen space)
        float ndc_dx = 2.0f * drag.dx() / WIDTH;
        float ndc_dy = 2.0f * drag.dy() / HEIGHT;

        // Adjust for zoom and Y-axis flip
        pan -= glm::vec2(ndc_dx, -ndc_dy) / zoom.get();
        // where();

        // calcuate new viewmodel
        view = glm::mat4(1.0f);
        view = glm::scale(view, glm::vec3(zoom.get(), zoom.get(), 1.0f));
        view = glm::translate(view, glm::vec3(pan, 0.f));
    }

    void handleClick(GLFWwindow *window, int button, int action, int mods) override {
        drag.handleClick(window, button, action, mods);
    }

    void handlePointer(GLFWwindow *window, double xpos, double ypos) override {
        drag.handlePointer(window, xpos, ypos);
    }

    void handleScroll(GLFWwindow *window, double xoffset, double yoffset) override {
        zoom.handleScroll(window, xoffset, yoffset);
    }

    // TODO: remove me
    void where() {
        std::cout << "pan x: " << pan.x << " y: " << pan.y << "\n";
    }

    void set(glm::vec2 ndc) {
        pan = -ndc;
    }

    void set(float _zoom) {
        zoom.set(_zoom);
    }

    ~Panzoom() {};
};

} // namespace gfx

#endif // GFX_H
