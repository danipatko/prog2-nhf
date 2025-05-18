// Include standard headers
#include "gfx.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <glad/glad.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>

#include <iostream>
#include <vector>

namespace test {

// const unsigned int SCR_WIDTH = 800;
// const unsigned int SCR_HEIGHT = 600;
// const float PI = 3.14159f;

// // Vertex Shader Source
// const char *vertexShaderSource = R"(
// #version 330 core
// layout(location = 0) in vec2 aPos;
// layout(location = 1) in vec3 aColor;
// out vec3 ourColor;
// void main()
// {
//     gl_Position = vec4(aPos, 0.0, 1.0);
//     ourColor = aColor;
// }
// )";

// // Fragment Shader Source
// const char *fragmentShaderSource = R"(
// #version 330 core
// in vec3 ourColor;
// out vec4 FragColor;
// void main()
// {
//     FragColor = vec4(ourColor, 1.0);
// }
// )";

// // Function to compile shader
// GLuint compileShader(GLenum type, const char *source) {
//     GLuint shader = glCreateShader(type);
//     glShaderSource(shader, 1, &source, NULL);
//     glCompileShader(shader);
//     int success;
//     glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
//     if (!success) {
//         char infoLog[512];
//         glGetShaderInfoLog(shader, 512, NULL, infoLog);
//         std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
//     }
//     return shader;
// }

// int loop() {
//     glfwInit();
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

//     GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "C5 Graph", NULL, NULL);
//     if (!window) {
//         std::cerr << "Failed to create GLFW window" << std::endl;
//         glfwTerminate();
//         return -1;
//     }
//     glfwMakeContextCurrent(window);
//     gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

//     GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
//     GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
//     GLuint shaderProgram = glCreateProgram();
//     glAttachShader(shaderProgram, vertexShader);
//     glAttachShader(shaderProgram, fragmentShader);
//     glLinkProgram(shaderProgram);
//     glDeleteShader(vertexShader);
//     glDeleteShader(fragmentShader);

//     std::vector<float> vertexData;
//     std::vector<float> edgeData;

//     int n = 5;
//     float radius = 0.5f;
//     // std::vector<std::vector<float>> nodeColors = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}};

//     std::vector<std::vector<float>> edgeColors = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}};

//     std::vector<std::pair<float, float>> nodePositions;
//     for (int i = 0; i < n; ++i) {
//         float angle = 2.0f * PI * i / n;
//         float x = radius * cos(angle);
//         float y = radius * sin(angle);
//         nodePositions.push_back({x, y});
//         vertexData.push_back(x);
//         vertexData.push_back(y);
//         // vertexData.insert(vertexData.end(), nodeColors[i].begin(), nodeColors[i].end());
//     }

//     for (int i = 0; i < n; ++i) {
//         auto [x1, y1] = nodePositions[i];
//         auto [x2, y2] = nodePositions[(i + 1) % n];
//         auto color = edgeColors[i];

//         edgeData.push_back(x1);
//         edgeData.push_back(y1);
//         edgeData.insert(edgeData.end(), color.begin(), color.end());

//         edgeData.push_back(x2);
//         edgeData.push_back(y2);
//         edgeData.insert(edgeData.end(), color.begin(), color.end());
//     }

//     GLuint VBOs[2], VAOs[2];
//     glGenVertexArrays(2, VAOs);
//     glGenBuffers(2, VBOs);

//     // VAO/VBO for nodes
//     // glBindVertexArray(VAOs[0]);
//     // glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
//     // glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
//     // glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
//     // glEnableVertexAttribArray(0);
//     // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float)));
//     // glEnableVertexAttribArray(1);

//     // VAO/VBO for edges
//     glBindVertexArray(VAOs[1]);
//     glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
//     glBufferData(GL_ARRAY_BUFFER, edgeData.size() * sizeof(float), edgeData.data(), GL_STATIC_DRAW);
//     glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float)));
//     glEnableVertexAttribArray(1);

//     glBindVertexArray(0);

//     while (!glfwWindowShouldClose(window)) {
//         glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//         glClear(GL_COLOR_BUFFER_BIT);

//         glUseProgram(shaderProgram);

//         // Draw Edges
//         glBindVertexArray(VAOs[1]);
//         glDrawArrays(GL_LINES, 0, 2 * n);

//         // Draw Nodes
//         // glBindVertexArray(VAOs[0]);
//         // glDrawArrays(GL_POINTS, 0, n);

//         glfwSwapBuffers(window);
//         glfwPollEvents();
//     }

//     glDeleteVertexArrays(2, VAOs);
//     glDeleteBuffers(2, VBOs);
//     glDeleteProgram(shaderProgram);

//     glfwDestroyWindow(window);
//     glfwTerminate();
//     return 0;
// }

// main.cpp
// #include <GLFW/glfw3.h>
// #include <glad/glad.h>
// #include <glm/glm.hpp>

// #include <iostream>
// #include <vector>

// === Shader helper ===
GLuint compileShader(GLenum type, const char *source);
GLuint createShaderProgram(const char *vertexSrc, const char *geometrySrc, const char *fragmentSrc);

// === Drawable base class ===
template <typename T> class Drawable {
  protected:
    GLuint VAO, VBO;
    std::vector<T> vertices;

    void __draw__(GLenum type) {
        glBindVertexArray(VAO);
        glDrawArrays(type, 0, static_cast<GLsizei>(vertices.size()));
    }

    void __update__() {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(), GL_DYNAMIC_DRAW);
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

    virtual void draw() {
        __draw__(GL_LINES);
    }

    virtual ~Drawable() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }
};

// === Polyline class ===
struct PolylineVertex {
    glm::vec2 position;
    glm::vec3 color;
    float width;
};

class Polyline : public Drawable<PolylineVertex> {
  public:
    Polyline() : Drawable() {
        glEnableVertexAttribArray(0); // position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(PolylineVertex), (void *)offsetof(PolylineVertex, position));
        glEnableVertexAttribArray(1); // color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PolylineVertex), (void *)offsetof(PolylineVertex, color));
        glEnableVertexAttribArray(2); // width
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(PolylineVertex), (void *)offsetof(PolylineVertex, width));
    }

    void addPoint(const glm::vec2 &pos, const glm::vec3 &col, float w) {
        vtx().push_back({pos, col, w});
        __update__();
    }

    void clear() {
        vtx().clear();
        __update__();
    }
};

// === Shaders ===

// Vertex Shader
const char *vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in float aWidth;

out vec2 vertexPos;
out vec3 vertexColor;
out float vertexWidth;

void main() {
    vertexPos = aPos;
    vertexColor = aColor;
    vertexWidth = aWidth;
}
)";

// Geometry Shader
const char *geometryShaderSource = R"(
#version 330 core
layout(lines) in;
layout(triangle_strip, max_vertices = 256) out;

in vec2 vertexPos[];
in vec3 vertexColor[];
in float vertexWidth[];

out vec3 fragColor;

uniform mat4 view;
uniform mat4 projection;

const int CIRCLE_SEGMENTS = 36; // Higher = smoother circles

void emit_vertex(vec2 pos, vec3 color) {
    gl_Position = projection * view * vec4(pos, 0.0, 1.0);
    fragColor = color;
    EmitVertex();
}

void emit_circle(vec2 center, vec3 color, float radius) {
    for (int i = 0; i <= CIRCLE_SEGMENTS; ++i) {
        float angle = radians(360.0 * float(i) / CIRCLE_SEGMENTS);
        vec2 offset = vec2(cos(angle), sin(angle)) * radius;
        emit_vertex(center, color);
        emit_vertex(center + offset, color);
    }
    EndPrimitive();
}

void main() {
    vec2 p0 = vertexPos[0];
    vec2 p1 = vertexPos[1];

    vec3 c0 = vertexColor[0];
    vec3 c1 = vertexColor[1];

    float w0 = vertexWidth[0];
    float w1 = vertexWidth[1];

    vec2 dir = normalize(p1 - p0);
    vec2 normal = vec2(-dir.y, dir.x);

    vec2 offset0 = normal * w0 * 0.5;
    vec2 offset1 = normal * w1 * 0.5;

    // --- 1. Emit Main Quad (the line body) ---
    emit_vertex(p0 + offset0, c0);
    emit_vertex(p0 - offset0, c0);
    emit_vertex(p1 + offset1, c1);
    emit_vertex(p1 - offset1, c1);
    EndPrimitive();

    // --- 2. Emit Circle at p0 ---
    emit_circle(p0, c0, w0 * 0.5);

    // --- 3. Emit Circle at p1 ---
    emit_circle(p1, c1, w1 * 0.5);
}

)";

// Fragment Shader
const char *fragmentShaderSource = R"(
#version 330 core
in vec3 fragColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(fragColor, 1.0);
}
)";

// === Main ===
int main() {
    // --- Initialize GLFW ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Polyline Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // --- Load GLAD ---
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    static const GLuint WIDTH = 800, HEIGHT = 600;
    static const float ASPECT = (float)WIDTH / HEIGHT;

    glViewport(0, 0, WIDTH, HEIGHT);

    // --- Create Shader Program ---
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, geometryShaderSource, fragmentShaderSource);
    glUseProgram(shaderProgram);

    // --- Setup Projection ---
    glm::mat4 projection = glm::ortho(-ASPECT, ASPECT, -1.f, 1.f);
    glm::mat4 view = glm::mat4(1.0f);

    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");

    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

    // --- Create a Polyline ---
    Polyline poly;

    auto WHITE = glm::vec3{1.0f, 1.0f, 1.0f};
    auto W = 0.1f;

    poly.addPoint({-0.8f, -0.5f}, WHITE, W);
    poly.addPoint({-0.2f, 0.5f}, WHITE, W);
    poly.addPoint({0.2f, 0.5f}, WHITE, W);
    poly.addPoint({0.8f, -0.5f}, WHITE, W);

    // --- Render Loop ---
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        poly.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// === Shader Helpers ===
GLuint compileShader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader Compilation Failed\n" << infoLog << '\n';
    }
    return shader;
}

GLuint createShaderProgram(const char *vertexSrc, const char *geometrySrc, const char *fragmentSrc) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint geometryShader = compileShader(GL_GEOMETRY_SHADER, geometrySrc);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, geometryShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader Linking Failed\n" << infoLog << '\n';
    }

    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    return program;
}

} // namespace test
