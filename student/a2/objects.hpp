#pragma once

#include "mesh.hpp"
#include <algorithm>
#include <cmath>

// CPU scene data. Mesh is shared; each Object owns only its transform state.
struct Object
{
    const Mesh* mesh = nullptr;
    Vec2 position{};
    float angle = 0; // radians
    Vec2 size{1, 1};
};

inline Mat3 translation(float x, float y)
{
    // TODO A2.1a: homogeneous 2D translation matrix.
    // Hint: Initialize Mat3 with nine entries in braces, written row by row as on paper.
    return Mat3
    {
        1, 0, x,
        0, 1, y,
        0, 0, 1
    };
}

inline Mat3 rotation(float radians)
{
    // TODO A2.1b: counterclockwise 2D rotation matrix.
    // Hint: Use std::cos(radians), std::sin(radians), and a Mat3 initializer. Write entries row by row.
    return Mat3
    {
        std::cos(radians), -(std::sin(radians)), 0,
        std::sin(radians), std::cos(radians), 0,
        0, 0, 1
    };
}

inline Mat3 scaling(float x, float y)
{
    // TODO A2.1c: independent x/y scaling matrix.
    // Hint: Initialize Mat3 with x/y scale factors and the homogeneous coordinate entry. No OpenGL calls are needed here.
    return Mat3
    {
        x, 0, 0,
        0, y, 0,
        0, 0, 1
    };
}

inline Mat3 modelMatrix(const Object& o, bool useSRT = false)
{
    // TODO A2.2: TRS normally, SRT when useSRT is true.
    // Hint: Reuse translation, rotation, scaling, and Mat3::operator*. A * B applies B first.
    const Mat3 T = translation(o.position.x, o.position.y);
    const Mat3 R = rotation(o.angle);
    const Mat3 S = scaling(o.size.x, o.size.y);

    return !useSRT ? T * R * S : S * R * T;
}

inline constexpr float PI = 3.14159265358979323846f;
inline constexpr float toRadians(float degree)
{
    return degree * (PI / 180.f);
}
inline std::vector<Object> makeObjects(const Mesh& mesh)
{
    // TODO A2.3: create three objects sharing mesh, with distinct position/angle/size.
    // Hint: Add Object{mesh pointer, position, angle, size} entries to std::vector<Object>. A2 positions are not measured in pixels.
    std::vector<Object> objs;
    objs.push_back({ &mesh, {0.f, 0.f}, toRadians(0.f), {0.5f, 0.5f}});
    objs.push_back({ &mesh, {-0.5f, 0.f}, toRadians(45.f), {0.4f, 0.6f}}); // 45도 회전
    objs.push_back({ &mesh, {0.5f, 0.f}, toRadians(90.f), {0.3f, 0.4f}}); // 90도 회전
    return objs; // Prior assignment's single house.
}

inline void updateObject(Object& o, const App& app)
{
    // TODO A2.4: WASD translation, Q/E rotation, Z/X scale, R resets position/angle/size to (0,0)/0/(1,1); use app.dt.
    // Hint: Use app.down(GLFW_KEY_...) for held keys, app.pressed(...) for a new key press, and app.dt for frame time in seconds.
    // Only modify o; do not recreate the VBO or modify shared mesh vertices.
    constexpr float move_speed = 0.5f;
    constexpr float rotate_speed = 1.f;
    constexpr float scale_speed = 0.5f;

    // Translation
    if (app.down(GLFW_KEY_W)) o.position.y += move_speed * app.dt;
    if (app.down(GLFW_KEY_S)) o.position.y -= move_speed * app.dt;
    if (app.down(GLFW_KEY_A)) o.position.x -= move_speed * app.dt;
    if (app.down(GLFW_KEY_D)) o.position.x += move_speed * app.dt;

    // Rotation
    if (app.down(GLFW_KEY_Q)) o.angle -= rotate_speed * app.dt;
    if (app.down(GLFW_KEY_E)) o.angle += rotate_speed * app.dt;

    // Scale
    o.size.x = std::max(o.size.x, 0.1f);
    o.size.y = std::max(o.size.y, 0.1f);
    if (app.down(GLFW_KEY_Z))
    { 
        o.size.x -= scale_speed * app.dt; 
        o.size.y -= scale_speed * app.dt; 
    }
    if (app.down(GLFW_KEY_X)) 
    { 
        o.size.x += scale_speed * app.dt; 
        o.size.y += scale_speed * app.dt; 
    }

    // Reset
    if (app.pressed(GLFW_KEY_R))
    {
        o.position = { 0.f, 0.f };
        o.angle = 0.f;
        o.size = { 1.f, 1.f };
    }
}

inline void drawObject(const Object& o, GLuint program, bool useSRT = false)
{
    if (!o.mesh) return;

    // TODO A2.5: compute model matrix and upload uModel
    // Hint: Use the GLint returned by glGetUniformLocation(program, "uModel") with glProgramUniformMatrix3fv(program, location, count, transpose, data). Access matrix data with m.data().
    // (column-major, transpose GL_FALSE).
    // glProgramUniformMatrix3fv uploads to the named program.

    // Compute ModelMatrix
    Mat3 model = modelMatrix(o, useSRT);

    // Get Uniform Location
    GLint location = glGetUniformLocation(program, "uModel");

    // Set Uniform
    glProgramUniformMatrix3fv(program, location, 1, GL_FALSE, model.m.data());

    drawMesh(*o.mesh);
}
