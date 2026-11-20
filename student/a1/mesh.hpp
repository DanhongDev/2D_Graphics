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
    verts.push_back({ 0.5f, 0.25f, 1.f, 0.f, 0.f }); // 우상단
    verts.push_back({ -0.5f, 0.25f, 0.f, 1.f, 0.f }); // 좌상단
    verts.push_back({ -0.5f, -0.75f, 0.f, 0.f, 1.f }); // 좌하단
    verts.push_back({ 0.5f, -0.75f, 1.f, 1.f, 0.f }); // 우하단
    verts.push_back({ 0.f, 0.75f, 1.f, 0.f, 1.f }); // 지붕

    verts.push_back({ -0.2f, -0.25f, 1.f, 1.f, 1.f }); // 문 우상단
    verts.push_back({ -0.4f, -0.25f, 1.f, 1.f, 1.f }); // 문 좌상단
    verts.push_back({ -0.4f, -0.75f, 1.f, 1.f, 1.f }); // 문 좌하단
    verts.push_back({ -0.2f, -0.75f, 1.f, 1.f, 1.f }); // 문 우하단

    verts.push_back({ 0.3f, 0.1f, 0.f, 0.f, 0.f }); // 창문 우상단
    verts.push_back({ 0.f, 0.1f, 0.f, 0.f, 0.f }); // 창문 좌상단
    verts.push_back({ 0.f, -0.1f, 0.f, 0.f, 0.f }); // 창문 좌하단
    verts.push_back({ 0.3f, -0.1f, 0.f, 0.f, 0.f }); // 창문 우하단
    return verts;
}
inline std::vector<unsigned> houseIndices()
{
    // TODO A1.2: add triangle indices for all four house parts.
    // Hint: Add vertex indices to std::vector<unsigned>, with three indices per triangle.
    std::vector<unsigned> idxs = { 0,1,2, 0,2,3, 0,1,4, 5,6,7, 5,7,8, 9,10,11, 9,11,12 };
    return idxs;
}
inline void createMesh(Mesh& mesh,const std::vector<Vertex>& vertices,const std::vector<unsigned>& indices)
{
    require(mesh.vao==0,"Create each immutable mesh only once");
    if(vertices.empty()||indices.empty())
        return;
    // TODO A1.3: glCreateBuffers + glNamedBufferStorage for VBO/EBO; byte sizes, flags=0.
    // Hint: Use glCreateBuffers(count, names) and glNamedBufferStorage(buffer, byteSize, data, flags). Use size() * sizeof(element type) and data().
    // Create names in mesh.vbo / mesh.ebo and upload the supplied arrays.
    // TODO A1.4: create VAO, attach VBO to binding 0, attach EBO.
    // Hint: Use glCreateVertexArrays, glVertexArrayVertexBuffer(vao, binding, buffer, offset, stride), and glVertexArrayElementBuffer.
    // Binding index 0; offset 0; stride sizeof(Vertex).
    
    // VAO: 정점 버퍼 연결과 정점 속성 해석 방법등을 관리하는 객체
        glGenVertexArrays(1, &mesh.vao);
        // glGenVertexArrays(n, &arrays): VAO를 식별할 ID를 n개 생성하고, 그 값을 arrays에 저장하는 함수
            // n = 생성할 VAO 이름(ID)의 개수
            // arrays = 생성된 ID를 저장할 변수의 메모리 주소
            // arrays는 VAO 객체 자체가 아니라, OpenGL에서 VAO를 식별하는 정수형 ID를 저장하는 변수이다.
            // ID는 OpenGL이 결정하므로 항상 k가 나오는 것은 아니다.

        glBindVertexArray(mesh.vao);
        // glBindVertexArray(array): arrays에 저장된 ID를 이용해 VAO를 현재 OpenGL Context에 바인딩하는 함수
            // glGenVertexArrays()로 발급받은 ID를 처음 바인딩하면 실제 VAO 객체와 그 상태가 생성된다.

    // VBO: 실제 정점 데이터가 저장되는 GPU 버퍼
        glGenBuffers(1, &mesh.vbo);
        // glGenBuffers(n, &buffers): 버퍼 ID를 n개 발급받아 buffers에 저장하는 함수
            // 버퍼 객체 자체를 즉시 생성하는 것이 아니라, 사용할 ID를 예약한다.

        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        // glBindBuffer(target, buffer): buffer가 가리키는 버퍼 객체를 현재 OpenGL Context의 target 바인딩 지점에 연결
            // target = 버퍼를 바인딩 할 대상
                // 이 버퍼를 어느 바인딩 지점에 연결할 것인지 지정하는 역할
                // GL_ARRAY_BUFFER = 정점의 위치, 색상, 텍스처 좌표등의 데이터
                    // 현재 OpenGL Context의 일반적인 버퍼 바인딩 상태
            // buffer = 바인딩할 버퍼 객체의 ID
            // target 지점에 바인딩했다고 해서 버퍼 객체가 영구적으로 VBO or EBO가 되는것이 아니다.
            // 버퍼의 타입을 영구적으로 결정하는 함수라기보다는 "특정 작업에 사용할 버퍼를 선택하는 함수"이다.

        glBufferData(
            GL_ARRAY_BUFFER,
            vertices.size() * sizeof(Vertex),
            vertices.data(),
            GL_STATIC_DRAW
        );
        // glBufferData(target, size, *data, usage): 현재 target에 바인딩된 버퍼에 size만큼의 바이트 크기 저장 공간을 할당하고 *data가 가리키는 CPU 메모리의 데이터를 복사한다. 그리고 usage를 통해 사용 패턴 힌트를 전달한다
            // target = 데이터를 저장할 대상 버퍼의 바인딩 지점을 지정
            // size = size만큼의 크기를 할당
                // vertices.size() = 정점 개수
                // sizeof(Vertex) = 정점 하나의 크기(Byte)
            // *data = CPU 메모리에 저장된 정점 배열의 시작 주소를 반환
                // CPU메모리 vertices.data() = {v0, v1, v2, ...}를 OpenGL 버퍼 mesh.vbo에 복사한다.
            // usage = 버퍼의 데이터가 어떤 방식으로 사용될 것으로 예상되는지 OpenGL 드라이버에 알려주는 사용 패턴 힌트
                // GL_STATIC_DRAW = 데이터를 거의 수정하지 않고 여러 번 사용 (ex. 고정된 집의 정점)
                // GL_DYNAMIC_DRAW = 데이터를 반복적으로 수정하며 사용 (ex. 자주 바뀌는 정점 데이터)
                // GL_STREAM_DRAW = 데이터를 매우 자주 갱신하거나 새 데이터로 교체 (ex. 매 프레임 새로 생성하는 정점 데이터)

    // EBO: 정점들을 어떤 순서로 조합해서 삼각형을 만들지 기록하는 버퍼
        glGenBuffers(1, &mesh.ebo);
        // 위와 동일
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        // GL_ELEMENT_ARRAY_BUFFER = 정점을 조합하는 인덱스
            // 현재 바인딩된 "VAO"에 저장되는 상태
            // VAO를 먼저 바인딩한 상태에서 EBO를 연결하면 나중에 해당 VAO를 다시 바인딩할 때 EBO 연결도 함께 복원된다.
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned),
            indices.data(),
            GL_STATIC_DRAW
        );
        // sizeof(unsinged) = EBO에는 정점의 인덱스들이 저장되고, 현재 indices 벡터의 원소 자료형이 unsinged이기 때문이다.
        
    
    // TODO A1.5: configure position (location 0, 2 floats) and color (location 1, 3 floats).
    // Hint: Use glVertexArrayAttribFormat, glVertexArrayAttribBinding, and glEnableVertexArrayAttrib. Use offsetof(Vertex, x) / offsetof(Vertex, r) for byte offsets.
    // Both attributes use VBO binding index 0. u/v are not enabled yet.

    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, x))
    );
    // glVertexAttribPointer(index, size, type, normalized, stride, *pointer): 현재 VAO의 index번 Attribute는 현재 바인딩된 VBO에서의 정보를 읽는 함수.
    // 각 정점의 위치는 type형 size개로 구성되고 다음 정점의 위치까지는 stride 바이트 만큼 이동하며 첫번째 위치 데이터는 *pointer지점에서 시작한다.
        // index = VAO의 정점 속성 인덱스
            // glVertexAttribPointer(0, 2, ...): Attribute 0 -> 정점의 위치 (x,y)
            // glVertexAttribPointer(1, 3, ...): Attribute 1 -> 정점의 색상 (r,g,b)
            // Vertex Shader에 다음과 같이 연결 가능
                // layout(location = 0) in vec2 aPos, layout(location = 1) in vec3 aColor
            // 즉, "VAO의 Attribute Index와 Shader의 Attribute Location이 대응하는 구조"이다.
        // size = 속성 하나를 구성하는 성분의 개수
        // type = 각 성분의 자료형
            // VBO 전체의 크기가 아닌 "위치 속성 하나의 데이터 크기"이다.
        // normalized = 정규화
            // 정수형 정점 데이터를 부동소수점 속성으로 읽을 때, 값을 정규화된 범위로 변환할지 결정한다.
                // ex. unsinged char r = 255; -> GL_FALSE = 255.0, GL_TRUE = 1.0
        // stride = 연속된 두 정점의 "동일한 Attribute 데이터 시작 위치 사이의 바이트 간격"
        // *pointer = 정점 구조체 내부에서 해당 속성이 시작하는 위치
            // offsetof(Vertex, x): Vertex 구조체의 시작 지점으로부터 x 멤버까지의 바이트 오프셋을 계산 -> 0Byte
        
    glEnableVertexAttribArray(0);
    // glEnableVertexAttribArray(index): VAO의 index번 정점 속성을 활성화하는 함수

    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, r))
    );
    glEnableVertexAttribArray(1);
    // 위와 동일

    // ==========================
    // DSA 방식 (OpenGL 4.5 이상)
    // ==========================
    glCreateVertexArrays(1, &mesh.vao);
    // glCreateVertexArrays(n, *arrays): ID를 발급하면서 실제 VAO 객체까지 생성하고 초기화하는 함수
        // glGenVertexArrays() + glBindVertexArray() 이다. 다만, 객체를 설정할 때 바인딩 의존성을 줄여주는 것일뿐, 렌더링 시 필요한 바인딩까지 전부 없애는 함수는 아니다.

    glCreateBuffers(1, &mesh.vbo);
    // glCreateBuffers(n, *buffers): 버퍼 ID를 발급하면서 실제 버퍼 객체까지 생성하고 초기화하는 함수
        // 버퍼 객체를 생성한 것과 버퍼에 실제 정점 데이터를 저장하는 것은 별개이다.

    glNamedBufferStorage(
        mesh.vbo,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        0
    );
    // glNamedBufferStorage(buffer, size, *data, flags): buffer를 직접 지정해서 데이터를 저장하는 함수
        // DSA에서는 버퍼 ID를 직접 지정해서 데이터를 저장할 수 있다.
        // buffer = 데이터를 저장할 버퍼 객체 ID
        // size = 할당할 저장 공간의 크기
        // *data = 복사할 CPU 데이터의 시작주소
        // flags = 저장소 생성 옵션 (필요한 플래그는 비트 OR 연산자 '|'로 결합 가능)
            // 0 = 추가 옵션 없음
            // GL_DYNAMIC_STORAGE_BIT = glNamedBufferSubData() 등으로 데이터 갱신 허용
            // GL_MAP_READ_BIT = 버퍼 저장소를 읽기 매핑할 수 있도록 허용
            // GL_MAP_WRITE_BIT = 버퍼 저장소를 쓰기 매핑할 수 있도록 허용
        // glNamedBufferStorage()는 불변 저장소이다. glBufferData()는 같은 버퍼 객체의 "저장 공간"을 재할당 할 수 있지만 glNamedBufferStorage()는 재할당할 수 없다. 단 데이터는 변경가능하다.

    glCreateBuffers(1, &mesh.ebo);
    glNamedBufferStorage(
        mesh.ebo,
        indices.size() * sizeof(unsigned),
        indices.data(),
        0
    );

    glVertexArrayVertexBuffer(mesh.vao, 0, mesh.vbo, 0, sizeof(Vertex));
    // glVertexArrayVertexBuffer(vaobj, bindingindex, buffer, offset, stride): VAO의 "Binding Index"에 VBO를 연결하는 함수
        // vaobj = 연결할 VAO ID
        // bindingindex = VAO 내부의 버퍼 바인딩 슬롯의 번호
        // buffer = VAO에 연결할 VBO ID
        // offset = VBO 시작 지점에서 바인딩이 시작하는 바이트 위치
        // stride = 같은 VBO 내부에서 다음 데이터로 이동하기 위한 바이트 간격

    glVertexArrayAttribBinding(mesh.vao, 0, 0);
    glVertexArrayAttribBinding(mesh.vao, 1, 0);
    // glVertexArrayAttribBinding(vaobj, attribindex, bindingindex): vaobj의 attribindex번 속성을 bindingindex번에 연결하는 함수

    glVertexArrayAttribFormat(mesh.vao, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, x));
    // glVertexArrayAttribFormat(vaobj, attribindex, size, type, normalized, relativeoffset): vaobj의 attribindex번의 속성 형식을 결정하는 함수
        // relativeoffset = BindingOffset을 기준으로 하는 상대적인 위치
            // 데이터 위치 = BindingOffset + VertexIndex * stride + RelativeOffset

    glEnableVertexArrayAttrib(mesh.vao, 0);
    // glEnableVertexArrayAttrib(vaobj, index): 연결할 vaobj를 지정하고 해당 VAO의 index번 attribute를 활성화하는 함수
    glVertexArrayElementBuffer(mesh.vao, mesh.ebo);
    // glVertexArrayElementBuffer(vaobj, buffer): vaobj에 buffer(ebo)를 연결

    
    
    mesh.indexCount=GLsizei(indices.size());
    // draw할 인덱스 개수를 기억하기 위해 indexCount에 저장 (drawMesh()함수 에는 indices를 매개변수로 안 받기 때문.)
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
    // glDrawElements(mode, count, type, *indices)
        // mode = 어떤 도형(Primitive)으로 그릴지 선택
        // count = 참조할 인덱스 개수
        // type = 인덱스 요소의 자료형
        // *indices = EBO 내부에서 인덱스 데이터를 읽기 시작할 바이트 오프셋
            // nullptr은 오프셋 0을 의미 즉, EBO의 맨 처음부터 인덱스를 읽는다.
            // EBO를 참조하는 것은 nullptr 자체가 아니라 "이전에 VAO에 저장해 둔 EBO 바인딩"이다.
}
