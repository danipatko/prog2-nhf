#include "map.h"
#include "GLFW/glfw3.h"
#include "drawing.h"
#include "gfx.h"

// Vertex Shader
const char *VSS = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in float aRating;

uniform mat4 view;
uniform mat4 projection;

out vec3 vertexColor;
out float vertexRating;

void main() {
    vertexColor = aColor;
    vertexRating = aRating;

    gl_Position = view * projection * vec4(aPos, 0.0, 1.0);
}
)";

// Fragment Shader
const char *FSS = R"(
#version 330 core

uniform mat4 view;
uniform float alpha = 1.0;

in vec3 vertexColor;
in float vertexRating;

out vec4 FragColor;

void main() {
    // drop under 0.1 alpha
    if(vertexRating * view[0][0] < 10)
        discard;

    FragColor = vec4(vertexColor, alpha * smoothstep(0, 100, vertexRating * view[0][0]));
    // FragColor = vec4(vertexColor, 1.0);
}
)";

const char *pVSS = R"(
#version 330 core

layout(location = 0) in vec2 pos;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = view * projection * vec4(pos, 0.0, 1.0);
}
)";

const char *pFSS = R"(
#version 330 core

out vec4 FragColor;

void main() {
    FragColor = vec4(0, 1, 1, 1);
}
)";

using namespace gfx;

Map::Map()
    : Window("NHF"),            //
      line_shader(VSS, FSS),    //
      point_shader(pVSS, pFSS), //
      proj_mat(glm::ortho(-ASPECT, ASPECT, -1.f, 1.f)) {}

void Map::onStart() {
    line_shader.use();

    line_shader.setUniform("projection", proj_mat);
    line_shader.setUniform("view", view_mat);

    route.width(3.f);
}

void Map::onFrame() {
    line_shader.use();
    line_shader.setUniform("projection", proj_mat);
    line_shader.setUniform("view", view_mat);

    line_shader.setUniform("alpha", road_alpha);
    geo_roads.draw(&line_shader);

    line_shader.setUniform("alpha", discovered_alpha);
    discovered.draw(&line_shader);

    line_shader.setUniform("alpha", 1.f);
    route.draw(&line_shader);

    point_shader.use();
    point_shader.setUniform("projection", proj_mat);
    point_shader.setUniform("view", view_mat);
    points.draw(&point_shader);

    if (!paused && callback != nullptr)
        callback();
}

void Map::onKey(int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        paused = !paused;
}

void Map::onClick(int button, int action, int mods) {
    panzoom.handleClick(window, button, action, mods);
}

void Map::onPointer(double xoffset, double yoffset) {
    panzoom.handlePointer(window, xoffset, yoffset);
    panzoom.transform(view_mat, WIDTH, HEIGHT);

    line_shader.setUniform("view", view_mat);
    point_shader.setUniform("view", view_mat);
}

void Map::onScroll(double xoffset, double yoffset) {
    panzoom.handleScroll(window, xoffset, yoffset);
    panzoom.transform(view_mat, WIDTH, HEIGHT);

    line_shader.setUniform("view", view_mat);
    point_shader.setUniform("view", view_mat);
}
