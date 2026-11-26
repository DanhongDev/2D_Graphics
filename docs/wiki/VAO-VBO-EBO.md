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


---

## 원본 Wiki 상세 레퍼런스 — 매개변수 및 주의사항 전체 기록

> `mesh.hpp`에 작성했던 원본 학습 주석을 함수 단위로 보존한 상세 참고 자료입니다. 위쪽의 요약 설명과 함께 사용하세요.

### 0. 용어 사전

```cpp
// VAO (Vertex Array Object)
//   정점 입력 상태: 정점 속성의 형식/활성화, 속성->버퍼 바인딩 연결,
//   버퍼 바인딩 슬롯의 VBO/offset/stride, EBO 연결 정보를 관리한다.
// VBO (Vertex Buffer Object)
//   정점 위치/색상 등의 데이터가 저장된 버퍼 객체를 부르는 관용적 명칭이다.
// EBO (Element Buffer Object)
//   glDrawElements에서 사용할 인덱스가 저장된 버퍼 객체를 부르는 명칭이다.
// Attribute Index
//   정점 입력 속성의 번호. 예: 0=position, 1=color.
//   Vertex Shader의 layout(location = n)과 대응한다.
// Binding Index
//   VAO 안에 있는 '정점 버퍼 연결 슬롯'의 번호.
//   Binding[0] -> vboA 처럼 VBO ID, 시작 offset, stride를 기억한다.
//   Attribute[n] -> Binding[k]로 연결하며 여러 Attribute가 같은 Binding을 써도 된다.
// Stride
//   같은 속성의 연속된 정점 사이의 바이트 간격이다. 다음 VBO까지의 간격이 아니다.
// Relative Offset
//   정점 버퍼 Binding Offset을 기준으로 한 속성 시작 바이트 거리이다.
//   정점 i의 속성 시작 바이트 = bindingOffset + i*stride + relativeOffset.
```

```cpp
// 개념상 VAO 내부 상태 (실제 드라이버 구현을 뜻하는 것은 아님):
// struct VAOConcept {
//     Binding bindings[MAX_BINDINGS];
//     Attribute attributes[MAX_ATTRIBUTES];
//     GLuint elementBuffer;
// };
// Shader
//   입력받은 정점 데이터 등을 어떻게 처리할지 정의하는 GPU 프로그램이다.
//   VertexShader는 주로 정점 변환을, FragmentShader는 주로 색상 계산을 수행한다.
// Shader Program
//   여러 Shader 단계의 객체를 첨부하고 링크하여 구성한 프로그램이다.
//   실행 코드와 Uniform 상태 등을 관리한다.
// Shader와 Shader Program의 관계
//   하나의 Shader Program에는 서로 다른 단계의 Shader들을 첨부할 수 있다. (단, 같은 Shader 단계에 속하는 Shader 객체 여러개를 동시에 첨부할 수는 없다.)
//   하나의 Shader 객체를 여러 Shader Program에서 재사용할 수도 있다.
```

### 1. VAO 생성 및 선택

#### `glGenVertexArrays(GLsizei n, GLuint* arrays)`

```cpp
// glGenVertexArrays(GLsizei n, GLuint* arrays)
//   역할: 미사용 VAO 이름(ID) n개를 발급받아 arrays가 가리키는 메모리에 기록한다.
//   n      = 발급받을 VAO ID 개수.
//   arrays = 생성된 ID(들)를 받을 GLuint 배열의 시작 주소.
//   주의: 이 시점은 이름 예약이며, 실제 VAO 객체는 최초 glBindVertexArray 시 생성.
//   예: glGenVertexArrays(1, &mesh.vao);
```

#### `glBindVertexArray(GLuint array)`

```cpp
// glBindVertexArray(GLuint array)
//   역할: 현재 OpenGL Context에서 사용할 VAO를 선택(바인딩)한다.
//   array = 선택할 VAO ID. 0을 전달하면 VAO 바인딩 해제(Core Profile에서는
//           VAO 없이 정점 배열 관련 설정/그리기를 할 수 없으므로 주의).
//   주의: glGenVertexArrays로 발급된 ID를 처음 바인딩할 때 VAO 객체가 생성된다.
//   예: glBindVertexArray(mesh.vao);
```

#### `glCreateVertexArrays(GLsizei n, GLuint* arrays) [OpenGL 4.5 DSA]`

```cpp
// glCreateVertexArrays(GLsizei n, GLuint* arrays) [OpenGL 4.5 DSA]
//   역할: VAO ID n개를 발급하고, 실제 VAO 객체까지 즉시 생성/초기화한다.
//   n      = 생성할 VAO 개수.
//   arrays = ID(들)를 받을 GLuint 배열 주소.
//   주의: VAO를 현재 바인딩하지 않는다. DSA 설정에는 바인딩이 불필요하지만
//         glDrawElements 등 일반 그리기에는 glBindVertexArray가 여전히 필요.
//   예: glCreateVertexArrays(1, &mesh.vao);
```

### 2. 버퍼 객체 생성 / 바인딩

#### `glGenBuffers(GLsizei n, GLuint* buffers)`

```cpp
// glGenBuffers(GLsizei n, GLuint* buffers)
//   역할: 버퍼 이름(ID) n개를 예약하여 buffers 배열에 기록한다.
//   n       = 버퍼 ID 개수.
//   buffers = ID를 받을 GLuint 배열 주소.
//   주의: 최초 glBindBuffer로 실제 버퍼 객체가 생성된다.
```

#### `glBindBuffer(GLenum target, GLuint buffer)`

```cpp
// glBindBuffer(GLenum target, GLuint buffer)
//   역할: 현재 Context의 target 바인딩 지점에 버퍼 ID를 연결한다.
//   target = GL_ARRAY_BUFFER(일반 정점 버퍼 설정에 사용),
//            GL_ELEMENT_ARRAY_BUFFER(현재 VAO의 인덱스 버퍼 연결 상태) 등.
//   buffer = 연결할 버퍼 ID. 0은 해당 타깃의 연결을 해제한다.
//   주의: GL_ARRAY_BUFFER 바인딩 자체는 VAO 상태가 아니다.
//         glVertexAttribPointer가 호출될 때 현재 바인딩된 VBO 연결을 VAO에 기록한다.
//         반면 GL_ELEMENT_ARRAY_BUFFER 바인딩은 현재 VAO 상태에 저장된다.
```

#### `glCreateBuffers(GLsizei n, GLuint* buffers) [OpenGL 4.5 DSA]`

```cpp
// glCreateBuffers(GLsizei n, GLuint* buffers) [OpenGL 4.5 DSA]
//   역할: 버퍼 ID n개를 발급하면서 버퍼 객체를 즉시 생성/초기화한다.
//   n       = 버퍼 객체 개수.
//   buffers = ID를 받을 GLuint 배열 주소.
//   주의: 생성만 수행한다. 실제 데이터 저장소는 별도 할당이 필요하다.
//         이 함수 자체는 VBO/EBO 용도를 고정하지 않는다.
```

### 3. 버퍼 저장소 할당 / 갱신

#### `glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)`

```cpp
// glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
//   역할: target에 현재 바인딩된 버퍼의 가변 저장소를 (재)할당하고 초기 데이터를 복사.
//   target = GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER 등 버퍼 바인딩 대상.
//   size   = 저장소 크기(Byte), 예: vertices.size()*sizeof(Vertex).
//   data   = 초기 데이터 CPU 메모리 주소. nullptr이면 초기 내용을 미지정 상태로 둔다.
//   usage  = 예상 사용 패턴 힌트. GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW 등.
//   주의: usage는 삼각형/선 같은 그리기 도형 종류를 정하지 않는다.
```

#### `glNamedBufferData(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage)`

```cpp
// glNamedBufferData(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage)
//   [OpenGL 4.5 DSA]
//   역할: 지정한 buffer의 가변 저장소를 (재)할당하고 초기 데이터를 복사한다.
//   buffer = 대상 버퍼 ID. 버퍼 바인딩 불필요.
//   size   = 저장소 크기(Byte).
//   data   = 초기 데이터 CPU 주소(또는 nullptr).
//   usage  = GL_STATIC_DRAW 등 올바른 사용 패턴 enum. '0'은 유효하지 않다!
//   예: glNamedBufferData(mesh.vbo, bytes, ptr, GL_STATIC_DRAW);
```

#### `glNamedBufferStorage(GLuint buffer, GLsizeiptr size, const void* data,`

```cpp
// glNamedBufferStorage(GLuint buffer, GLsizeiptr size, const void* data,
//                      GLbitfield flags) [OpenGL 4.5 DSA]
//   역할: 지정한 buffer에 불변(immutable) 저장소를 한 번 생성하고 초기 데이터 복사.
//   buffer = 대상 버퍼 ID. 버퍼 바인딩 불필요.
//   size   = 저장소 크기(Byte).
//   data   = 초기 데이터 CPU 주소(또는 nullptr).
//   flags  = 저장소 접근 권한/동작 옵션의 비트 조합.
//            0                      = 옵션 없이 생성.
//            GL_DYNAMIC_STORAGE_BIT = glNamedBufferSubData 등을 통한 명시적 갱신 허용.
//            GL_MAP_READ_BIT        = 읽기 매핑 허용.
//            GL_MAP_WRITE_BIT       = 쓰기 매핑 허용.
//            옵션 결합 예: GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT.
//   주의: 불변은 '저장소 재할당 불가'라는 뜻이지, 모든 GPU 수정 금지는 아니다.
//         flags=0으로 생성한 저장소에 glNamedBufferSubData를 호출하면 오류.
//         같은 버퍼에 glNamedBufferStorage를 재호출해 저장소를 다시 만들 수도 없다.
```

#### `glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size,`

```cpp
// glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size,
//                 const void* data)
//   역할: 현재 target에 바인딩된 버퍼 저장소의 일부 바이트를 갱신한다.
//   target = 대상 버퍼 바인딩 지점.
//   offset = 갱신 시작 바이트 오프셋.
//   size   = 갱신 바이트 수.
//   data   = 새 CPU 데이터 주소.
//   주의: 저장소가 미리 있어야 하고 [offset, offset+size) 구간이 범위 안이어야 한다.
```

#### `glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size,`

```cpp
// glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size,
//                      const void* data) [OpenGL 4.5 DSA]
//   역할: 바인딩 없이 지정한 버퍼의 데이터 일부를 갱신한다.
//   buffer = 대상 버퍼 ID.
//   offset = 갱신 시작 바이트 오프셋.
//   size   = 갱신 바이트 수.
//   data   = 새 CPU 데이터 주소.
//   주의: 불변 저장소는 GL_DYNAMIC_STORAGE_BIT가 있어야 이 함수로 갱신 가능.
```

### 4. 정점 속성 설정: 기존 방식

#### `glVertexAttribPointer(GLuint index, GLint size, GLenum type,`

```cpp
// glVertexAttribPointer(GLuint index, GLint size, GLenum type,
//                       GLboolean normalized, GLsizei stride, const void* pointer)
//   역할: 현재 VAO의 Attribute[index]에 형식, stride, 상대적 시작 위치 및
//         '현재 GL_ARRAY_BUFFER에 바인딩된 VBO' 연결을 기록한다.
//   index      = Attribute Index. 셰이더 layout(location=n)과 대응.
//   size       = 속성 하나의 성분 개수(보통 1~4).
//   type       = 성분 자료형, 예: GL_FLOAT.
//   normalized = 정수형 데이터를 부동소수점 입력으로 바꿀 때 정규화 여부.
//                GL_FLOAT인 경우 사실상 의미 없음. 법선 벡터(Normal)와 무관.
//   stride     = 다음 정점의 동일 속성까지의 바이트 간격(0이면 밀집 배열로 해석).
//   pointer    = VBO가 연결된 경우 'VBO 내부 시작 바이트 오프셋'을 포인터로 표현.
//                예: reinterpret_cast<void*>(offsetof(Vertex, r)).
//   주의: GL_ARRAY_BUFFER를 다른 VBO로 바인딩하기만 해서는 VAO 속성 연결이
//         변경되지 않고, 이 함수를 다시 호출해야 변경된다.
```

#### `glEnableVertexAttribArray(GLuint index)`

```cpp
// glEnableVertexAttribArray(GLuint index)
//   역할: 현재 바인딩된 VAO의 Attribute[index] 정점 배열을 활성화한다.
//   index = 활성화할 Attribute 번호.
//   주의: 형식 설정과 활성화는 별도 작업이다.
```

### 5. 정점 속성 설정: DSA 방식

#### `glVertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer,`

```cpp
// glVertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer,
//                           GLintptr offset, GLsizei stride)
//   역할: vaobj의 Binding[bindingindex] 슬롯에 VBO, 시작 위치와 stride 설정.
//   vaobj        = 설정할 VAO ID.
//   bindingindex = VAO 내부 정점 버퍼 바인딩 슬롯 번호(속성 번호가 아님).
//   buffer       = 연결할 VBO ID.
//   offset       = VBO 저장소 기준 시작 바이트 오프셋.
//   stride       = 연속된 정점 사이의 바이트 간격.
//   주의: 이 함수만으로 Attribute가 특정 Binding을 참조하지는 않는다.
```

#### `glVertexArrayAttribFormat(GLuint vaobj, GLuint attribindex, GLint size,`

```cpp
// glVertexArrayAttribFormat(GLuint vaobj, GLuint attribindex, GLint size,
//                           GLenum type, GLboolean normalized,
//                           GLuint relativeoffset)
//   역할: vaobj의 Attribute[attribindex] 자료형/성분 수/정규화/상대 오프셋 설정.
//   vaobj          = 설정할 VAO ID.
//   attribindex    = 정점 속성 번호(셰이더 layout(location=n)).
//   size           = 성분 개수.
//   type           = 성분 자료형, 예: GL_FLOAT.
//   normalized     = 정수형 데이터 정규화 여부.
//   relativeoffset = Binding Offset을 기준으로 한 속성 시작 바이트 거리.
//   주의: 실제 시작 위치 = bindingOffset + vertexIndex*stride + relativeoffset.
```

#### `glVertexArrayAttribBinding(GLuint vaobj, GLuint attribindex,`

```cpp
// glVertexArrayAttribBinding(GLuint vaobj, GLuint attribindex,
//                            GLuint bindingindex)
//   역할: Attribute[attribindex] -> Binding[bindingindex] 관계를 기록한다.
//   vaobj        = 설정할 VAO ID.
//   attribindex  = 속성 인덱스. '무엇을 읽는가'와 형식을 식별.
//   bindingindex = VBO 바인딩 슬롯 인덱스. '어디서 읽는가'를 선택.
//   주의: 여러 Attribute가 같은 Binding을 공유할 수 있다.
//         Binding 슬롯의 VBO를 바꾸면 이를 참조하는 모든 Attribute가 영향을 받는다.
```

#### `glEnableVertexArrayAttrib(GLuint vaobj, GLuint index)`

```cpp
// glEnableVertexArrayAttrib(GLuint vaobj, GLuint index)
//   역할: vaobj의 Attribute[index] 배열을 활성화한다(미리 VAO 바인딩 불필요).
//   vaobj = 설정할 VAO ID.
//   index = 활성화할 Attribute 번호.
```

#### `glVertexArrayElementBuffer(GLuint vaobj, GLuint buffer)`

```cpp
// glVertexArrayElementBuffer(GLuint vaobj, GLuint buffer)
//   역할: vaobj에 EBO를 직접 연결한다.
//   vaobj  = EBO 연결을 저장할 VAO ID.
//   buffer = 연결할 EBO ID.
//   주의: EBO용 별도 Binding Index/Stride는 없다. 인덱스 자료형과 읽기 시작
//         오프셋은 glDrawElements 호출에서 제공한다.
```

### 6. 렌더링 및 자원 해제

#### `glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)`

```cpp
// glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
//   역할: 현재 VAO의 EBO에서 인덱스를 읽고 정점 속성을 취합해 그리기를 수행.
//   mode    = 기본 도형 조립 방식. 예: GL_TRIANGLES.
//   count   = 읽을 인덱스 '원소' 개수(삼각형 개수가 아님).
//   type    = 인덱스 원소 자료형. 예: GL_UNSIGNED_INT.
//   indices = EBO가 연결되어 있으면 EBO 내부의 시작 '바이트 오프셋'.
//             nullptr은 0Byte에서 시작한다는 뜻.
//   주의: glDrawElements를 위한 VAO는 glBindVertexArray로 현재 바인딩해야 한다.
```

#### `glDeleteVertexArrays(GLsizei n, const GLuint* arrays)`

```cpp
// glDeleteVertexArrays(GLsizei n, const GLuint* arrays)
//   역할: 지정한 VAO 객체를 삭제한다.
//   n      = 삭제할 VAO ID 개수.
//   arrays = 삭제할 ID 배열의 주소.
```

#### `glDeleteBuffers(GLsizei n, const GLuint* buffers)`

```cpp
// glDeleteBuffers(GLsizei n, const GLuint* buffers)
//   역할: 지정한 버퍼 객체들을 삭제 대상으로 지정한다.
//   n       = 삭제할 버퍼 ID 개수.
//   buffers = 삭제할 ID 배열 주소.
//   주의: OpenGL Context가 살아있는 동안 호출되어야 한다.
```

#### `C++ 보조 코드: mesh.indexCount = GLsizei(indices.size());`

```cpp
// C++ 보조 코드: mesh.indexCount = GLsizei(indices.size());
//   역할: drawMesh에 원본 indices 벡터를 넘기지 않고도 그릴 인덱스 개수를 기억.
//   GLsizei = glDrawElements의 count 매개변수와 호환되는 정수 자료형.
```

```cpp
// 과제에서 쓰인 require(mesh.vao == 0, "...")는 OpenGL API가 아니라
// course.hpp가 제공하는 검증 유틸리티이다.
```

