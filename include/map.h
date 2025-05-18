#ifndef H_MAP
#define H_MAP

#include "drawing.h"
#include "gfx.h"

#include <functional>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float4.hpp>

using namespace gfx;

class Map : public Window {
    Shader line_shader;
    Shader point_shader;

    glm::mat4 view_mat;
    glm::mat4 proj_mat;

    bool paused = false;

    void onStart() override;
    void onFrame() override;

    void onClick(int button, int action, int mods) override;
    void onPointer(double xoffset, double yoffset) override;
    void onScroll(double xoffset, double yoffset) override;
    void onKey(int key, int scancode, int action, int mode) override;

  public:
    Panzoom panzoom;
    std::function<void(void)> callback;

    // renderables

    /**
     * @brief underlying map itself
     */
    PolyLine geo_roads;

    /**
     * @brief roads discovered by the algorithm (colored red)
     */
    PolyLine discovered;

    /**
     * @brief the final route (colored cyan)
     */
    PolyLine route;

    /**
     * @brief Drawable point (used to mark source and target)
     */
    DPoint points;

    Map();

    float road_alpha = 1.f;
    float discovered_alpha = 1.f;

    /**
     * @brief Set alpha channel for the base map (useful for emphasizing the route)
     */
    void dim_roads(const float f = 0.5) {
        road_alpha = f;
    }

    /**
     * @brief Set alpha channel for discovered segments (useful for emphasizing the route)
     */
    void dim_discovered(const float f = 0.5) {
        discovered_alpha = f;
    }

    /**
     * @brief Multiplies p vector with the projection matrix
     */
    glm::vec2 project(const glm::vec2 &p) {
        glm::vec4 result = proj_mat * glm::vec4(p, 0, 0);
        return {result.x, result.y};
    }
};

#endif // H_MAP