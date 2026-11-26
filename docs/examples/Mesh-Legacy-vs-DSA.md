# Mesh 생성 — Legacy vs DSA

> **학습 주제:** A1 `Mesh`의 VAO/VBO/EBO 생성 및 Draw Call 비교.  
> **관련 Wiki:** [VAO / VBO / EBO](../wiki/VAO-VBO-EBO.md)

## 1. 전제 및 예제 자료형

- 실제 과제에서는 제공된 `Mesh`, `Vertex`, `require()` 및 OpenGL 로더를 사용한다.
- `Vertex`는 `x`, `y`, `r`, `g`, `b` 필드를 가진다고 가정한다. 실제 과제에 `u`, `v`가 더 있다면 `sizeof(Vertex)`가 그 크기를 반영한다.
- 동일한 `Mesh`에 Legacy/DSA 생성 함수를 **모두 호출하지 않는다**.
- DSA 코드는 OpenGL **4.5+**와 지원되는 함수 로더가 필요하다.

```cpp
// 예시 자료형. 실제 과제의 Mesh/Vertex 선언이 있다면 중복 선언하지 않는다.
struct Vertex {
    float x, y;
    float r, g, b;
};

struct Mesh {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei indexCount = 0;
};
```

## A. 기존 방식 — `glGen*` / `glBind*` / `glVertexAttribPointer`

```cpp
inline void createMeshLegacy(
    Mesh& mesh,
    const std::vector<Vertex>& vertices,
    const std::vector<unsigned>& indices)
{
    require(mesh.vao == 0, "Create each mesh only once");
    if (vertices.empty() || indices.empty()) return;

    // 1. VAO 이름 생성 및 현재 VAO로 바인딩
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // 2. VBO 생성 및 데이터 복사 (가변 저장소)
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_STATIC_DRAW
    );

    // 3. Attribute[0] = 위치 (x, y)
    //    glVertexAttribPointer 호출 당시 바인딩된 VBO가 VAO에 기록됨
    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, x))
    );
    glEnableVertexAttribArray(0);

    // 4. Attribute[1] = 색상 (r, g, b)
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, r))
    );
    glEnableVertexAttribArray(1);

    // 5. EBO 생성 및 데이터 복사
    //    GL_ELEMENT_ARRAY_BUFFER 연결은 현재 VAO 상태에 저장됨
    glGenBuffers(1, &mesh.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(unsigned),
        indices.data(),
        GL_STATIC_DRAW
    );

    mesh.indexCount = static_cast<GLsizei>(indices.size());
}
```

## B. DSA 방식 — OpenGL 4.5+

```cpp
inline void createMeshDSA(
    Mesh& mesh,
    const std::vector<Vertex>& vertices,
    const std::vector<unsigned>& indices)
{
    require(mesh.vao == 0, "Create each mesh only once");
    if (vertices.empty() || indices.empty()) return;

    // 1. VAO 객체 즉시 생성 (현재 VAO 바인딩 불필요)
    glCreateVertexArrays(1, &mesh.vao);

    // 2. VBO / EBO 객체 생성 + 불변 저장소 할당 + 초기 데이터 업로드
    glCreateBuffers(1, &mesh.vbo);
    glNamedBufferStorage(
        mesh.vbo,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        0 // 이후 부분 갱신을 허용하지 않는 정적 저장소
    );

    glCreateBuffers(1, &mesh.ebo);
    glNamedBufferStorage(
        mesh.ebo,
        indices.size() * sizeof(unsigned),
        indices.data(),
        0
    );

    // 3. Binding[0] ← VBO / VAO ← EBO
    glVertexArrayVertexBuffer(
        mesh.vao, 0, mesh.vbo, 0, sizeof(Vertex)
    );
    glVertexArrayElementBuffer(mesh.vao, mesh.ebo);

    // 4. Attribute[0] = 위치, Binding[0] 사용
    glVertexArrayAttribFormat(
        mesh.vao, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, x)
    );
    glVertexArrayAttribBinding(mesh.vao, 0, 0);
    glEnableVertexArrayAttrib(mesh.vao, 0);

    // 5. Attribute[1] = 색상, Binding[0] 사용
    glVertexArrayAttribFormat(
        mesh.vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, r)
    );
    glVertexArrayAttribBinding(mesh.vao, 1, 0);
    glEnableVertexArrayAttrib(mesh.vao, 1);

    mesh.indexCount = static_cast<GLsizei>(indices.size());
}
```

## C. 공통 그리기 과정

```cpp
inline void drawMeshExample(const Mesh& mesh)
{
    if (mesh.vao == 0) return;

    glBindVertexArray(mesh.vao);
    glDrawElements(
        GL_TRIANGLES,
        mesh.indexCount,
        GL_UNSIGNED_INT,
        nullptr // EBO 바이트 오프셋 0
    );
}
```

> 위 Draw 함수는 **유효한 Shader Program이 선택되어 있다는 전제**다. Shader 설정은 [Shader / Uniform 비교 예제](Shader-Uniform-Legacy-vs-DSA.md) 참고.

## D. 기존 방식 ↔ DSA 매칭 요약

| 단계 | Legacy | DSA |
| --- | --- | --- |
| VAO 생성 | `glGenVertexArrays` + `glBindVertexArray` | `glCreateVertexArrays` |
| VBO/EBO 생성 | `glGenBuffers` + `glBindBuffer` | `glCreateBuffers` |
| 가변 저장소 | `glBufferData` | `glNamedBufferData` |
| 불변 저장소 | `glBufferStorage` | `glNamedBufferStorage` |
| Attribute 연결 | `glBindBuffer(GL_ARRAY_BUFFER)` + `glVertexAttribPointer` | `glVertexArrayVertexBuffer` + `glVertexArrayAttribFormat` + `glVertexArrayAttribBinding` |
| Attribute 활성화 | `glEnableVertexAttribArray` | `glEnableVertexArrayAttrib` |
| EBO 연결 | 현재 VAO 바인딩 + `glBindBuffer(GL_ELEMENT_ARRAY_BUFFER)` | `glVertexArrayElementBuffer` |
| 부분 갱신 | `glBufferSubData` | `glNamedBufferSubData` |
| 그리기 | `glBindVertexArray` + `glDrawElements` | **동일** |
| 자원 해제 | `glDeleteVertexArrays` + `glDeleteBuffers` | **동일** |

## E. 상태 캡처와 Binding Index 이해

```text
Legacy
  현재 GL_ARRAY_BUFFER = vboA
  glVertexAttribPointer(0, ...)
    → VAO의 Attribute 0은 vboA를 읽도록 기록
  glBindBuffer(GL_ARRAY_BUFFER, vboB)
    → Attribute 0의 소스가 자동으로 vboB로 바뀌지는 않음

DSA
  Binding[0] → vboA (offset=0, stride=sizeof(Vertex))
  Attribute[0] → Binding[0] (position)
  Attribute[1] → Binding[0] (color)
    → Binding[0]의 버퍼를 교체하면 두 Attribute 모두 그 버퍼를 사용
```

## F. 메모리/자원 관리 주의

```cpp
// OpenGL Context가 유효할 때 실행
inline void deleteMeshExample(Mesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ebo);
    mesh.vao = mesh.vbo = mesh.ebo = 0;
    mesh.indexCount = 0;
}
```

- **주의:** 실제 과제의 `Mesh` 소멸자가 자동 해제한다면 위 함수를 추가 호출해 이중으로 관리할 필요는 없다.
- `glNamedBufferStorage(..., flags=0)`은 `glNamedBufferSubData()`를 통한 변경이 불가능하다. 수정할 계획이면 생성 시 플래그를 설계해야 한다.
- 예제는 `std::vector`와 `offsetof` 및 OpenGL 헤더가 제공되는 환경에서 사용하는 **학습용 발췌 코드**다.

## G. 자가 점검

- [ ] `GL_ARRAY_BUFFER` 바인딩과 `GL_ELEMENT_ARRAY_BUFFER` 바인딩이 VAO에 미치는 영향 설명하기
- [ ] `AttribIndex`와 `BindingIndex`를 구분하기
- [ ] `glDrawElements`의 count가 인덱스 개수임을 설명하기
- [ ] 불변 저장소를 선택하는 이유와 갱신 제약 설명하기
