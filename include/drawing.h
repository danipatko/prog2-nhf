#ifndef DRAWING_H
#define DRAWING_H

#include "gfx.h"
#include <cstddef>

using namespace gfx;

// TODO: more colors
const glm::vec3 WHITE = glm::vec3(1, 1, 1);

struct CVertex {
    glm::vec2 position;
    glm::vec3 color;
};

struct PolyVertex {
    glm::vec2 position;
    glm::vec3 color;
    float rating;
};

/**
 * @brief Variable color and width polyline
 */
class PolyLine : public Drawable<PolyVertex> {
    float _width = 1.f;

  public:
    PolyLine() : Drawable<PolyVertex>() {
        // position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(PolyVertex), (void *)0);
        glEnableVertexAttribArray(0);

        // color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PolyVertex), (void *)offsetof(PolyVertex, color));
        glEnableVertexAttribArray(1);

        // rating
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(PolyVertex), (void *)offsetof(PolyVertex, rating));
        glEnableVertexAttribArray(2);
    };

    void add(const glm::vec2 &from, const glm::vec2 &to, const glm::vec3 &color = WHITE, const float rating = 1.f) {
        vtx().push_back({from, color, rating});
        vtx().push_back({to, color, rating});
    }

    void update() {
        __update__();
    }

    float width() {
        return _width;
    }

    void width(float w) {
        _width = w;
    }

    void draw(Shader *shader) {
        glLineWidth(_width);
        __draw__(shader, GL_LINES);
    }
};

class DPoint : public Drawable<glm::vec2> {
  public:
    DPoint() : Drawable<glm::vec2>() {
        // position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        // // color
        // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CVertex), (void
        // *)offsetof(PolyVertex, color)); glEnableVertexAttribArray(1);

        // __update__();
    };

    void add(glm::vec2 point) {
        vtx().push_back(point);
    }

    void update() {
        __update__();
    }

    void draw(Shader *sh) override {
        glPointSize(12);
        __draw__(sh, GL_POINTS);
    }
};

#endif // DRAWING_H