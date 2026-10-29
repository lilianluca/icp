// window_effect.cpp
// Prototype: "window into 3D world" (head-coupled perspective) on a 3D Tetris well.
// Head position is simulated by mouse for now.
//
// Controls:
//   mouse move   - move "head" left/right/up/down in front of the monitor
//   mouse wheel  - head distance from the monitor
//   SPACE        - toggle effect ON/OFF (OFF = ordinary fixed camera)
//   ESC          - quit

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// --- physical setup (centimeters) ---------------------------------------------
// The window on the monitor is the "glass" of the window, at z = 0.
// x right, y up, z towards the viewer. The well goes into the monitor (z < 0).
constexpr float SCREEN_W = 40.0f;   // assumed physical width of the window
constexpr int   GRID_X = 8;         // well size in cells
constexpr int   GRID_Y = 5;
constexpr int   GRID_Z = 10;        // number of layers (depth)

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
};

struct Scene {
    float w, h;          // physical size of the window (= well cross-section)
    float cw, ch, cd;    // cell size in x, y, z
    std::vector<Vertex> lines;
    std::vector<Vertex> tris;

    // darker with depth - helps to read the depth a little
    static float depth_shade(float z, float depth) { return 1.0f - 0.55f * (-z / depth); }

    void add_line(glm::vec3 a, glm::vec3 b, glm::vec3 color, float depth) {
        lines.push_back({ a, color * depth_shade(a.z, depth) });
        lines.push_back({ b, color * depth_shade(b.z, depth) });
    }

    void add_quad(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 color) {
        tris.insert(tris.end(), { {a, color}, {b, color}, {c, color}, {a, color}, {c, color}, {d, color} });
    }

    // grid on the four walls and the bottom of the well
    void build_well() {
        const float depth = GRID_Z * cd;
        const glm::vec3 col(0.25f, 0.75f, 0.85f);
        const float x0 = -w / 2, x1 = w / 2, y0 = -h / 2, y1 = h / 2;

        // rings around the walls, one per layer
        for (int k = 0; k <= GRID_Z; k++) {
            float z = -k * cd;
            add_line({ x0, y0, z }, { x1, y0, z }, col, depth);
            add_line({ x1, y0, z }, { x1, y1, z }, col, depth);
            add_line({ x1, y1, z }, { x0, y1, z }, col, depth);
            add_line({ x0, y1, z }, { x0, y0, z }, col, depth);
        }
        // lines going into depth (walls) + grid on the bottom
        for (int i = 0; i <= GRID_X; i++) {
            float x = x0 + i * cw;
            add_line({ x, y0, 0 }, { x, y0, -depth }, col, depth);
            add_line({ x, y1, 0 }, { x, y1, -depth }, col, depth);
            add_line({ x, y0, -depth }, { x, y1, -depth }, col, depth);
        }
        for (int j = 0; j <= GRID_Y; j++) {
            float y = y0 + j * ch;
            add_line({ x0, y, 0 }, { x0, y, -depth }, col, depth);
            add_line({ x1, y, 0 }, { x1, y, -depth }, col, depth);
            add_line({ x0, y, -depth }, { x1, y, -depth }, col, depth);
        }
    }

    // cube in cell (i, j) with its front face at depth z_front (z_front <= 0)
    void add_cube(int i, int j, float z_front, glm::vec3 color) {
        const float depth = GRID_Z * cd;
        float x0 = -w / 2 + i * cw, x1 = x0 + cw;
        float y0 = -h / 2 + j * ch, y1 = y0 + ch;
        float z1 = z_front, z0 = z_front - cd;
        glm::vec3 c = color * depth_shade(z1, depth);

        // front face (towards viewer) brightest, side faces darker
        add_quad({ x0, y0, z1 }, { x1, y0, z1 }, { x1, y1, z1 }, { x0, y1, z1 }, c);
        add_quad({ x0, y0, z0 }, { x0, y0, z1 }, { x0, y1, z1 }, { x0, y1, z0 }, c * 0.6f); // left
        add_quad({ x1, y0, z1 }, { x1, y0, z0 }, { x1, y1, z0 }, { x1, y1, z1 }, c * 0.6f); // right
        add_quad({ x0, y0, z0 }, { x1, y0, z0 }, { x1, y0, z1 }, { x0, y0, z1 }, c * 0.45f); // bottom
        add_quad({ x0, y1, z1 }, { x1, y1, z1 }, { x1, y1, z0 }, { x0, y1, z0 }, c * 0.75f); // top

        // dark edges, so that neighbouring cubes are distinguishable
        const glm::vec3 e(0.05f);
        glm::vec3 p[8] = { {x0,y0,z0}, {x1,y0,z0}, {x1,y1,z0}, {x0,y1,z0},
                           {x0,y0,z1}, {x1,y0,z1}, {x1,y1,z1}, {x0,y1,z1} };
        int edges[12][2] = { {0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7} };
        for (auto& ed : edges) {
            lines.push_back({ p[ed[0]], e });
            lines.push_back({ p[ed[1]], e });
        }
    }

    // cube lying in layer k (0 = top layer, GRID_Z-1 = bottom)
    void add_cube_layer(int i, int j, int k, glm::vec3 color) { add_cube(i, j, -k * cd, color); }

    void build(float aspect, double time) {
        w = SCREEN_W;
        h = SCREEN_W / aspect;
        cw = w / GRID_X;
        ch = h / GRID_Y;
        cd = ch;
        lines.clear();
        tris.clear();

        build_well();

        // already fallen blocks: bottom layer almost full, a few in the layer above
        const int bottom = GRID_Z - 1;
        for (int i = 0; i < GRID_X; i++)
            for (int j = 0; j < GRID_Y; j++)
                if (!((i == 5 && j == 2) || (i == 6 && j == 2) || (i == 2 && j == 3)))  // holes
                    add_cube_layer(i, j, bottom, { 0.9f, 0.45f, 0.2f });
        for (auto [i, j] : std::initializer_list<std::pair<int, int>>{ {0, 0}, {1, 0}, {0, 1}, {7, 4}, {7, 3}, {3, 1} })
            add_cube_layer(i, j, bottom - 1, { 0.85f, 0.75f, 0.2f });

        // falling L piece above the hole at (5,2),(6,2) - slowly falls and repeats
        const float fall_depth = (GRID_Z - 1) * cd;           // from top to the bottom layer
        float z = -std::fmod(static_cast<float>(time) * 2.0f, fall_depth);
        for (auto [i, j] : std::initializer_list<std::pair<int, int>>{ {5, 2}, {6, 2}, {6, 3} })
            add_cube(i, j, z, { 0.45f, 0.85f, 0.35f });
    }
};

// --- shaders --------------------------------------------------------------------
static const char* VS = R"(#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
uniform mat4 mvp;
out vec3 v_color;
void main() { gl_Position = mvp * vec4(pos, 1.0); v_color = color; }
)";

static const char* FS = R"(#version 330 core
in vec3 v_color;
out vec4 frag;
void main() { frag = vec4(v_color, 1.0); }
)";

static GLuint compile(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof log, nullptr, log);
        throw std::runtime_error(std::string("shader: ") + log);
    }
    return s;
}

// --- head (eye) state -----------------------------------------------------------
static float g_eye_dist = 60.0f;    // cm in front of the monitor
static bool g_effect_on = true;

static void scroll_cb(GLFWwindow*, double, double dy) {
    g_eye_dist = std::clamp(g_eye_dist - static_cast<float>(dy) * 5.0f, 20.0f, 150.0f);
}

static void key_cb(GLFWwindow* w, int key, int, int action, int) {
    if (action != GLFW_PRESS)
        return;
    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(w, GLFW_TRUE);
    if (key == GLFW_KEY_SPACE)
        g_effect_on = !g_effect_on;
}

// Off-axis projection: the window (screen rectangle w x h at z = 0) is fixed,
// only the eye moves. Frustum edges go from the eye through the screen edges.
static glm::mat4 window_projection(glm::vec3 eye, float w, float h, float far_z) {
    const float n = 1.0f;                 // near plane distance
    const float s = n / eye.z;            // scale from screen plane to near plane
    glm::mat4 proj = glm::frustum((-w / 2 - eye.x) * s, (w / 2 - eye.x) * s,
                                  (-h / 2 - eye.y) * s, (h / 2 - eye.y) * s,
                                  n, eye.z + far_z);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), -eye);
    return proj * view;
}

int main() {
    try {
        if (!glfwInit())
            throw std::runtime_error("glfwInit failed");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_SAMPLES, 4);

        GLFWwindow* win = glfwCreateWindow(1280, 800, "Window effect prototype", nullptr, nullptr);
        if (!win)
            throw std::runtime_error("glfwCreateWindow failed");
        glfwMakeContextCurrent(win);
        glfwSwapInterval(1);
        glfwSetScrollCallback(win, scroll_cb);
        glfwSetKeyCallback(win, key_cb);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
            throw std::runtime_error("glewInit failed");

        GLuint prog = glCreateProgram();
        glAttachShader(prog, compile(GL_VERTEX_SHADER, VS));
        glAttachShader(prog, compile(GL_FRAGMENT_SHADER, FS));
        glLinkProgram(prog);
        GLint mvp_loc = glGetUniformLocation(prog, "mvp");

        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);

        std::cout << "mouse = head position, wheel = distance, SPACE = effect on/off, ESC = quit\n";

        Scene scene;
        while (!glfwWindowShouldClose(win)) {
            int fb_w, fb_h, win_w, win_h;
            glfwGetFramebufferSize(win, &fb_w, &fb_h);
            glfwGetWindowSize(win, &win_w, &win_h);
            if (fb_w == 0 || fb_h == 0) {      // minimized
                glfwWaitEvents();
                continue;
            }
            glViewport(0, 0, fb_w, fb_h);
            float aspect = static_cast<float>(fb_w) / fb_h;

            scene.build(aspect, glfwGetTime());

            // mouse -> simulated head position in front of the window
            glm::vec3 eye(0.0f, 0.0f, g_eye_dist);
            if (g_effect_on) {
                double mx, my;
                glfwGetCursorPos(win, &mx, &my);
                float nx = std::clamp(static_cast<float>(mx / win_w) * 2.0f - 1.0f, -1.0f, 1.0f);
                float ny = std::clamp(static_cast<float>(my / win_h) * 2.0f - 1.0f, -1.0f, 1.0f);
                eye.x = nx * scene.w * 0.5f;
                eye.y = -ny * scene.h * 0.5f;
            }
            glm::mat4 mvp = window_projection(eye, scene.w, scene.h, GRID_Z * scene.cd + 10.0f);

            glClearColor(0.02f, 0.02f, 0.04f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(prog);
            glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));

            // cubes first, pushed slightly back so that edge lines stay visible
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(1.0f, 1.0f);
            glBufferData(GL_ARRAY_BUFFER, scene.tris.size() * sizeof(Vertex), scene.tris.data(), GL_STREAM_DRAW);
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(scene.tris.size()));
            glDisable(GL_POLYGON_OFFSET_FILL);

            glBufferData(GL_ARRAY_BUFFER, scene.lines.size() * sizeof(Vertex), scene.lines.data(), GL_STREAM_DRAW);
            glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(scene.lines.size()));

            char title[160];
            std::snprintf(title, sizeof title, "Window effect prototype - effect %s | head [%.0f, %.0f, %.0f] cm",
                          g_effect_on ? "ON" : "OFF (SPACE)", eye.x, eye.y, eye.z);
            glfwSetWindowTitle(win, title);

            glfwSwapBuffers(win);
            glfwPollEvents();
        }

        glfwDestroyWindow(win);
        glfwTerminate();
    }
    catch (std::exception const& e) {
        std::cerr << "App failed : " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
