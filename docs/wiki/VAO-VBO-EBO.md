# VAO / VBO / EBO — OpenGL API Wiki

> **학습 범위:** OpenGL Core Profile의 정점 입력, 인덱스 렌더링, OpenGL 4.5 Direct State Access(DSA).  
> **관련 예제:** [Mesh — Legacy vs DSA](../examples/Mesh-Legacy-vs-DSA.md) · [문제 해결 기록](../troubleshooting/OpenGL-Errors.md)

## 1. 핵심 개념

| 객체/용어 | 역할 |
| --- | --- |
| **VBO** (Vertex Buffer Object) | 위치·색상·텍스처 좌표 등 정점 데이터를 보관하는 **버퍼 객체**의 관용적 이름 |
| **EBO** (Element Buffer Object) | 정점을 참조할 인덱스들을 보관하는 버퍼 객체의 관용적 이름 |
| **VAO** (Vertex Array Object) | 정점 속성 활성화·형식·버퍼 연결·EBO 연결 등 **정점 입력 상태** 보관 |
| **Attribute Index** | Vertex Shader의 `layout(location = N)`에 대응하는 속성 번호 |
| **Binding Index** | VAO의 VBO 연결 슬롯 번호. 각 슬롯이 buffer/offset/stride를 저장 |
| **Stride** | 인접한 두 정점의 같은 속성 시작 위치 사이 간격(바이트) |
| **Relative Offset** | 버퍼 바인딩의 시작 offset으로부터 한 정점 내부에서 속성이 시작하는 바이트 위치 |

한 정점의 속성 읽기 주소(개념식):

```text
bindingOffset + vertexIndex * stride + relativeOffset
```

**핵심:** VAO는 정점 데이터를 직접 복사해서 소유하는 저장소가 아니다. 정점 데이터를 **어느 버퍼에서 어떤 형식으로 읽을지** 관리한다.

## 2. 학습용 Vertex 레이아웃

```cpp
struct Vertex 
{
    float x, y;       // Attribute 0: 2 x float
    float r, g, b;    // Attribute 1: 3 x float
    // 실제 과제 Vertex에는 u, v가 추가되어 있을 수 있다.
};
```

- Position: `layout(location = 0) in vec2 aPos;` — `offsetof(Vertex, x)`
- Color: `layout(location = 1) in vec3 aColor;` — `offsetof(Vertex, r)`
- Stride: `sizeof(Vertex)`. **실제 과제의 `Vertex` 구조체 정의**에 따라 크기가 달라진다.
- 두 Attribute가 동일한 VBO의 **Binding 0**을 공유할 수 있다.

## 3. VAO API

### `glGenVertexArrays(GLsizei n, GLuint* arrays)`
- **역할:** 미사용 VAO 이름(ID) `n`개 생성.
- `n`: 생성할 이름 수.
- `arrays`: 결과 ID 배열 주소.
- **주의:** 이 시점에는 아직 실제 VAO 객체가 생성되지 않는다. 최초 `glBindVertexArray()`에서 생성된다.

### `glBindVertexArray(GLuint array)`
- **역할:** Context의 현재 VAO를 선택.
- `array`: VAO ID. `0`은 바인딩 해제.
- **주의:** DSA로 VAO를 설정하더라도 일반 `glDrawElements()` 렌더링 시 VAO 바인딩이 필요하다.

### `glCreateVertexArrays(GLsizei n, GLuint* arrays)` — OpenGL 4.5 DSA
- **역할:** VAO ID 발급과 객체 생성/초기화를 함께 수행.
- **특징:** 생성 직후 바인딩하지 않고 `glVertexArray*()`로 설정할 수 있다.

### `glDeleteVertexArrays(GLsizei n, const GLuint* arrays)`
- **역할:** VAO 객체 삭제.
- `n`: 삭제할 ID 수. `arrays`: ID 배열 주소.

## 4. 버퍼 객체 API

### `glGenBuffers(GLsizei n, GLuint* buffers)` / `glBindBuffer(GLenum target, GLuint buffer)`
- `glGenBuffers`: 버퍼 이름만 발급.
- `glBindBuffer`: 지정된 `target`에 해당 버퍼를 바인딩. 최초 바인딩 시 실제 객체 생성.
- `target`: `GL_ARRAY_BUFFER`, `GL_ELEMENT_ARRAY_BUFFER` 등.
- `buffer`: 버퍼 ID. `0`은 바인딩 해제.
- **중요한 차이:** `GL_ARRAY_BUFFER`의 현재 바인딩 그 자체는 VAO 상태가 아니다. 반면 `GL_ELEMENT_ARRAY_BUFFER` 바인딩은 현재 VAO의 EBO 연결 상태다.

### `glCreateBuffers(GLsizei n, GLuint* buffers)` — OpenGL 4.5 DSA
- **역할:** 버퍼 객체를 즉시 생성하지만 **저장소는 아직 할당하지 않는다**.
- VBO/EBO는 별도 객체 종류가 아니며, 버퍼를 어디에 사용하는지에 따라 붙이는 이름이다.

### `glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)`
- **역할:** 바인딩된 버퍼의 **가변(mutable)** 저장소를 새로 할당하고 초기 데이터를 복사.
- `target`: 현재 버퍼를 찾을 바인딩 타깃.
- `size`: 총 바이트 수 (`vertices.size() * sizeof(Vertex)` 등).
- `data`: CPU 데이터 시작 주소. `nullptr`이면 내용은 초기화되지 않는다.
- `usage`: 사용 패턴 힌트 (`GL_STATIC_DRAW`, `GL_DYNAMIC_DRAW` 등). `0`은 유효한 usage 값이 아니다.

### `glNamedBufferData(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage)` — DSA
- **역할:** 바인딩 없이 `buffer` ID를 직접 지정해 **가변** 저장소 (재)할당.

### `glBufferStorage(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags)` / `glNamedBufferStorage(GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags)`
- **역할:** **불변(immutable)** 저장소를 한 번 할당하고 선택적으로 초기 데이터 복사.
- 전자는 바인딩된 타깃 기준, 후자는 버퍼 ID 직접 지정(DSA).
- `flags = 0`: 초기 데이터는 업로드 가능하지만 이후 `glBufferSubData`/`glNamedBufferSubData` 방식의 쓰기 업데이트는 허용되지 않는다.
- `flags`에 `GL_DYNAMIC_STORAGE_BIT`를 포함하면 부분 갱신 가능. Mapping 관련 허용 범위는 별도 map flags로 지정한다.
- **주의:** 불변 저장소는 `glBufferData`로 다시 할당할 수 없다. 실제 과제 A1의 정적인 Mesh는 `flags = 0` 사용 예시.

### `glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)` / `glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data)`
- **역할:** 기존 저장소 일부를 새 데이터로 덮어쓴다(바이트 단위 offset/size).
- **주의:** 불변 저장소의 경우 `GL_DYNAMIC_STORAGE_BIT`로 생성되어야 해당 경로의 갱신이 허용된다.

### `glDeleteBuffers(GLsizei n, const GLuint* buffers)`
- **역할:** 버퍼 객체를 삭제 대상으로 지정.
- **주의:** GPU 리소스 삭제는 유효한 OpenGL Context가 존재할 때 처리한다.

## 5. Legacy 정점 속성 설정

### `glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)`
- **역할:** 현재 VAO의 `index` Attribute가 **현재 `GL_ARRAY_BUFFER`에 바인딩된 VBO**를 어떻게 읽을지 설정한다.
- `index`: Attribute Index (`0`=position, `1`=color).
- `size`: 정점당 구성 요소 수 (`2`, `3` 등).
- `type`: 구성 요소 자료형 (`GL_FLOAT` 등).
- `normalized`: 정수형 속성의 정규화 여부 (`GL_FALSE`는 **법선 벡터와 무관**).
- `stride`: 다음 정점의 동일 속성까지 바이트 간격.
- `pointer`: VBO 시작 지점 기준 속성의 **바이트 오프셋**(클라이언트 포인터로 읽는 의미가 아님).
- **주의:** 이 함수를 호출할 때 VBO 연결을 VAO의 Attribute 상태에 캡처한다. 나중에 다른 `GL_ARRAY_BUFFER`를 바인딩해도 이전 Attribute의 소스 VBO가 자동 교체되지는 않는다.

### `glEnableVertexAttribArray(GLuint index)`
- **역할:** 현재 VAO의 지정 Attribute 읽기를 활성화.

## 6. DSA 정점 속성 설정

### `glVertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)`
- **역할:** VAO의 `Binding[bindingindex]`에 버퍼, 시작 바이트 offset, stride를 설정.
- 같은 Binding을 여러 Attribute가 참조할 수 있다.

### `glVertexArrayAttribFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)`
- **역할:** Attribute의 구성 요소 수·형식·정규화·정점 내부 상대 오프셋 설정.
- `relativeoffset`: `offsetof(Vertex, x)` 또는 `offsetof(Vertex, r)`처럼 바이트 단위.
- 정수 속성을 정수 입력으로 받으려면 `glVertexArrayAttribIFormat()`을 고려한다.

### `glVertexArrayAttribBinding(GLuint vaobj, GLuint attribindex, GLuint bindingindex)`
- **역할:** `Attribute[attribindex]`가 어느 `Binding[bindingindex]`를 사용할지 연결.
- **구분:** Attribute Index = Shader 입력 슬롯, Binding Index = VBO 연결 슬롯.

### `glEnableVertexArrayAttrib(GLuint vaobj, GLuint index)`
- **역할:** 지정한 VAO의 Attribute 활성화. 바인딩 불필요.

### `glVertexArrayElementBuffer(GLuint vaobj, GLuint buffer)`
- **역할:** 지정 VAO에 인덱스 버퍼(EBO)를 직접 연결.

## 7. 렌더링

### `glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)`
- **역할:** 현재 VAO의 EBO에서 인덱스를 읽고, 그 인덱스가 가리키는 정점 속성으로 Primitive를 그린다.
- `mode`: `GL_TRIANGLES` 등 Primitive 조립 방식.
- `count`: **인덱스 개수**(삼각형 개수가 아님).
- `type`: EBO의 인덱스 자료형 (`GL_UNSIGNED_INT`, `GL_UNSIGNED_SHORT` 등).
- `indices`: EBO의 **시작 바이트 오프셋**. `nullptr`은 0바이트.

```cpp
glBindVertexArray(mesh.vao);
glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
```

예: EBO가 `0,1,2, 0,2,3, 0,1,4`일 때, `count=6`, `indices=(void*)(3*sizeof(unsigned))`이면 두 번째·세 번째 삼각형을 그리는 인덱스 6개를 읽는다.

## 8. Legacy ↔ DSA 요약

| 작업 | Legacy (바인딩 중심) | DSA (객체 직접 지정) |
| --- | --- | --- |
| VAO 생성 | `glGenVertexArrays` + 최초 `glBindVertexArray` | `glCreateVertexArrays` |
| Buffer 생성 | `glGenBuffers` + 최초 `glBindBuffer` | `glCreateBuffers` |
| 가변 저장소 | `glBufferData` | `glNamedBufferData` |
| 불변 저장소 | `glBufferStorage` | `glNamedBufferStorage` |
| VBO → 속성 | `glBindBuffer(GL_ARRAY_BUFFER)` + `glVertexAttribPointer` | `glVertexArrayVertexBuffer` + `glVertexArrayAttribFormat` + `glVertexArrayAttribBinding` |
| Attribute 활성화 | `glEnableVertexAttribArray` | `glEnableVertexArrayAttrib` |
| EBO 연결 | VAO 바인딩 후 `glBindBuffer(GL_ELEMENT_ARRAY_BUFFER)` | `glVertexArrayElementBuffer` |
| 부분 갱신 | `glBufferSubData` | `glNamedBufferSubData` |
| 그리기 | `glBindVertexArray` + `glDrawElements` | **동일** |
| 자원 해제 | `glDeleteVertexArrays`, `glDeleteBuffers` | **동일** |

> **환경:** OpenGL 4.5 Core API의 DSA 함수는 4.5 지원 Context 및 함수 로더가 필요하다. macOS 기본 OpenGL은 최대 4.1이므로 위 4.5 DSA 코드를 그대로 실행할 수 없다.

## 9. 개인 메모 / 다시 확인할 질문

- [ ] Attribute Index와 Binding Index가 다른 이유를 자신만의 예제로 설명하기
- [ ] `GL_ARRAY_BUFFER` 바인딩이 VAO 상태가 아닌 이유 복습하기
- [ ] `GL_DYNAMIC_STORAGE_BIT`와 동적 데이터 업데이트 구분하기

**공식 문서:** [Khronos OpenGL 4 Reference Pages](https://registry.khronos.org/OpenGL-Refpages/gl4/)
