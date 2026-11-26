#pragma once
#include "course.hpp"

using namespace course;

struct Mesh 
{
    GLuint vao=0,vbo=0,ebo=0;
    GLsizei indexCount=0;
    Mesh()=default;
    Mesh(const Mesh&)=delete; 
    Mesh& operator=(const Mesh&)=delete;
    ~Mesh()
    {
        glDeleteVertexArrays(1,&vao);
        glDeleteBuffers(1,&vbo);
        glDeleteBuffers(1,&ebo);
    }
};

inline std::vector<Vertex> houseVertices()
{
    // TODO A1.1: build wall, roof, door, and window vertices (x,y,r,g,b).
    // Hint: Build CPU positions and colors in std::vector<Vertex>; no OpenGL calls are needed here.
    // In A1, (0, 0) maps to the screen center; x/y are normalized coordinates, not pixels.
    // Build the whole house around local (0, 0), using the center of its outer bounds
    // as the origin: (minX + maxX) / 2 = (minY + maxY) / 2 = 0.
    // A2 rotates/scales about this local origin, then places it at Object::position in TRS mode.
    // Center the whole house, not eac5h roof/wall/door/window separately.
    // The starter roof below is only a drawing check; it is not a centered complete house.
        std::vector<Vertex> verts;
        verts.push_back({ 0.5f, 0.25f, 0.984f, 0.808f, 0.694f }); // 우상단
        verts.push_back({ -0.5f, 0.25f, 0.984f, 0.808f, 0.694f }); // 좌상단
        verts.push_back({ -0.5f, -0.75f, 0.984f, 0.808f, 0.694f }); // 좌하단
        verts.push_back({ 0.5f, -0.75f, 0.984f, 0.808f, 0.694f }); // 우하단

        verts.push_back({ 0.f, 0.75f, 0.f, 1.f, 0.f }); // 지붕 윗쪽
        verts.push_back({ -0.6f, 0.25f, 0.f, 1.f, 0.f }); // 지붕 왼쪽
        verts.push_back({ 0.6f, 0.25f, 0.f, 1.f, 0.f }); // 지붕 오른쪽

        verts.push_back({ -0.2f, -0.25f, 0.588f, 0.294f, 0.f }); // 문 우상단
        verts.push_back({ -0.4f, -0.25f, 0.588f, 0.294f, 0.f }); // 문 좌상단
        verts.push_back({ -0.4f, -0.75f, 0.588f, 0.294f, 0.f }); // 문 좌하단
        verts.push_back({ -0.2f, -0.75f, 0.588f, 0.294f, 0.f }); // 문 우하단

        verts.push_back({ 0.3f, 0.1f, 0.529f, 0.808f, 0.922f }); // 창문 우상단
        verts.push_back({ 0.f, 0.1f, 0.529f, 0.808f, 0.922f }); // 창문 좌상단
        verts.push_back({ 0.f, -0.1f, 0.529f, 0.808f, 0.922f }); // 창문 좌하단
        verts.push_back({ 0.3f, -0.1f, 0.529f, 0.808f, 0.922f }); // 창문 우하단
        return verts;
}
inline std::vector<unsigned> houseIndices()
{
    // TODO A1.2: add triangle indices for all four house parts.
    // Hint: Add vertex indices to std::vector<unsigned>, with three indices per triangle.
        std::vector<unsigned> idxs = { 0,1,2, 0,2,3, 4,5,6, 7,8,9, 7,9,10, 11,12,13, 11,13,14 };
        return idxs;
}
inline void createMesh(Mesh& mesh,const std::vector<Vertex>& vertices,const std::vector<unsigned>& indices)
{
    require(mesh.vao==0,"Create each immutable mesh only once");
    if(vertices.empty()||indices.empty())
        return;

    //==========
    // DSA ver.
    //==========
    
    // TODO A1.3: glCreateBuffers + glNamedBufferStorage for VBO/EBO; byte sizes, flags=0.
    // Hint: Use glCreateBuffers(count, names) and glNamedBufferStorage(buffer, byteSize, data, flags). Use size() * sizeof(element type) and data().
    // Create names in mesh.vbo / mesh.ebo and upload the supplied arrays.
        // VBO 생성 + 데이터 저장
        glCreateBuffers(1, &mesh.vbo);
        glNamedBufferStorage(mesh.vbo, vertices.size() * sizeof(Vertex), vertices.data(), 0);
        // EBO 생성 + 데이터 저장
        glCreateBuffers(1, &mesh.ebo);
        glNamedBufferStorage(mesh.ebo, indices.size() * sizeof(unsigned), indices.data(), 0);

    // TODO A1.4: create VAO, attach VBO to binding 0, attach EBO.
    // Hint: Use glCreateVertexArrays, glVertexArrayVertexBuffer(vao, binding, buffer, offset, stride), and glVertexArrayElementBuffer.
    // Binding index 0; offset 0; stride sizeof(Vertex).
        // VAO 생성
        glCreateVertexArrays(1, &mesh.vao);
        // VAO에 VBO, EBO 연결
        glVertexArrayVertexBuffer(mesh.vao, 0, mesh.vbo, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(mesh.vao, mesh.ebo);

    // TODO A1.5: configure position (location 0, 2 floats) and color (location 1, 3 floats).
    // Hint: Use glVertexArrayAttribFormat, glVertexArrayAttribBinding, and glEnableVertexArrayAttrib. Use offsetof(Vertex, x) / offsetof(Vertex, r) for byte offsets.
    // Both attributes use VBO binding index 0. u/v are not enabled yet.
        // VAO 속성 설정 (위치)
        glVertexArrayAttribFormat(mesh.vao, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, x));
        // AttributeIndex와 BindingIndex를 연결
        glVertexArrayAttribBinding(mesh.vao, 0, 0);
        // 속성 활성화
        glEnableVertexArrayAttrib(mesh.vao, 0);
        // VAO 속성 설정 (색상)
        glVertexArrayAttribFormat(mesh.vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, r));
        // AttributeIndex와 BindingIndex를 연결
        glVertexArrayAttribBinding(mesh.vao, 1, 0);
        // 속성 활성화
        glEnableVertexArrayAttrib(mesh.vao, 1);

    mesh.indexCount=GLsizei(indices.size());
}
inline void drawMesh(const Mesh& mesh)
{
    if(!mesh.vao)
        return; // Keeps an unfinished starter runnable.

    // TODO A1.6: bind VAO and issue the indexed draw call.
    // Hint: Use glBindVertexArray followed by glDrawElements(mode, count, type, indices), with GL_TRIANGLES, GL_UNSIGNED_INT, and nullptr for the EBO start offset.
    // Count means number of indices, not number of triangles.
    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
}