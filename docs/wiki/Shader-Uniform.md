# Shader / Uniform — OpenGL API Wiki

> **학습 범위:** GLSL Shader 컴파일, Shader Program 링크, Uniform 값 관리 및 DSA 스타일 설정.  
> **관련 예제:** [Shader / Uniform — Legacy vs DSA](../examples/Shader-Uniform-Legacy-vs-DSA.md)

## 1. Shader가 필요한 이유

VAO/VBO/EBO는 정점과 인덱스를 준비하지만 **그 정점을 어디에 출력하고 Fragment에 어떤 색상을 부여할지** 설명하지 않는다. 현대 OpenGL Core Profile에서는 프로그래밍 가능한 Shader 단계가 이 계산을 담당한다.

```text
VAO + VBO + EBO → Vertex Shader → Primitive 조립 / Rasterization
                                   → Fragment Shader → 후처리 테스트/Framebuffer
```

| 용어 | 의미 |
| --- | --- |
| **Shader Object** | 한 Shader 단계의 GLSL 소스와 컴파일 결과를 담는 객체 |
| **Vertex Shader** | 정점별 처리 및 `gl_Position` 출력 |
| **Fragment Shader** | Fragment별 색상/출력 계산 |
| **Shader Program** | Shader 단계들을 첨부·링크한 프로그램 객체. 링크 결과와 Uniform 상태 등을 관리 |
| **Uniform** | GLSL의 프로그램 공통 입력 변수. 프로그램 객체에 설정된 값을 Shader 실행이 읽는다 |
| **Uniform Location** | 각 Program에서 활성 Uniform을 식별하는 정수 위치. CPU/GPU 메모리 주소가 아님 |

**차이:** Attribute는 정점마다 달라질 수 있지만, 일반 Uniform은 한 Draw Call 중 변하지 않는 공통 입력값이다.

## 2. GLSL 최소 예시

`scene.vert`:

```glsl
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;

uniform mat4 uMVP;
out vec3 vColor;

void main()
{
    gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}
```

`scene.frag`:

```glsl
#version 450 core
in vec3 vColor;
out vec4 FragColor;

uniform vec4 uColor;

void main()
{
    FragColor = vec4(vColor, 1.0) * uColor;
}
```

- `layout(location = 0/1)`은 VAO의 Attribute Index에 대응한다.
- `uMVP`는 모든 정점에 적용할 변환 행렬이다.
- `uColor`는 Fragment 색상에 곱하는 공통 색상이다.
- `vColor`는 Vertex Shader 출력과 Fragment Shader 입력 사이에서 보간되어 전달된다.
- 예시는 **OpenGL/GLSL 4.5 환경**을 가정한다. 프로젝트의 Shader 버전과 맞춰야 한다.

## 3. Shader 객체 API

### `GLuint glCreateShader(GLenum type)`
- **역할:** 지정 종류의 Shader 객체 생성 후 ID 반환.
- `type`: `GL_VERTEX_SHADER`, `GL_FRAGMENT_SHADER`, `GL_GEOMETRY_SHADER`, `GL_COMPUTE_SHADER` 등.
- 오류 발생 시 `0`이 반환될 수 있다. 생성만 하며 컴파일은 하지 않는다.

### `glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)`
- **역할:** 하나 이상의 GLSL 문자열을 Shader 객체에 복사.
- `shader`: 대상 Shader ID.
- `count`: 소스 문자열 개수.
- `string`: **문자열 포인터들로 이루어진 배열**의 주소. 문자열 하나라면 `&sourcePtr`.
- `length`: 각 문자열의 길이 배열; `nullptr`이면 null 종료 문자열로 길이 판정.
- **주의:** 파일 경로를 OpenGL에 직접 전달하지 않는다. C++에서 `.vert`/`.frag` 파일 내용을 읽어서 문자열로 넘긴다.

```cpp
std::string sourceText = readShaderFile("scene.vert");
const char* sourcePtr = sourceText.c_str();
glShaderSource(shader, 1, &sourcePtr, nullptr);
```

### `glCompileShader(GLuint shader)`
- **역할:** Shader 객체에 저장된 GLSL 코드를 컴파일.
- **주의:** 호출만으로 성공이 보장되지 않는다. 컴파일 상태 확인 필요.

### `glGetShaderiv(GLuint shader, GLenum pname, GLint* params)`
- **역할:** Shader 상태 조회.
- `pname`: `GL_COMPILE_STATUS`, `GL_INFO_LOG_LENGTH`, `GL_SHADER_TYPE` 등.
- `params`: 결과를 받을 변수 주소.

### `glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog)`
- **역할:** Shader의 컴파일/진단 메시지 조회.
- `maxLength`: 출력 버퍼 길이(바이트).
- `length`: 실제 기록된 문자 수 반환 주소, 필요 없으면 `nullptr`.
- `infoLog`: 출력 버퍼 주소.
- **주의:** 로그 유무와 컴파일 성공 여부는 동일하지 않다. 성공 여부는 `GL_COMPILE_STATUS`로 확인.

## 4. Shader Program API

### `GLuint glCreateProgram()`
- **역할:** 비어 있는 Program 객체 생성, Program ID 반환.

### `glAttachShader(GLuint program, GLuint shader)`
- **역할:** Shader를 Program에 첨부. 이 단계에서 링크되지는 않는다.
- `program`: Program ID, `shader`: Shader ID.
- **주의:** 일반적인 Program에서는 동일 종류의 Shader 객체를 여러 개 동시에 첨부할 수 없다. 한 Shader 객체를 여러 Program에 첨부할 수는 있다.

### `glLinkProgram(GLuint program)`
- **역할:** 첨부된 Shader 단계들의 인터페이스를 검증하고 프로그램을 링크.
- **중요:** 성공적으로 **다시** 링크하면 Uniform 값은 초기화되고 Location을 다시 조회해야 한다. 기존 숫자가 같더라도 재조회가 안전하다.

### `glGetProgramiv(GLuint program, GLenum pname, GLint* params)`
- **역할:** Program 상태 조회.
- `pname`: `GL_LINK_STATUS`, `GL_INFO_LOG_LENGTH`, `GL_ATTACHED_SHADERS` 등.

### `glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog)`
- **역할:** 링크 실패 등의 진단 메시지 조회. 매개변수 형식은 Shader 로그 함수와 유사.

### `glUseProgram(GLuint program)`
- **역할:** 현재 OpenGL Context에서 **렌더링에 사용할 프로그램** 선택.
- `program = 0`이면 현재 Program 사용 해제.
- `glUseProgram(programB)`를 해도 `programA`의 Uniform 값은 사라지지 않는다.
- `glProgramUniform*()`로 다른 Program 값을 설정하더라도 현재 활성 Program은 바뀌지 않는다.

### `glDetachShader(GLuint program, GLuint shader)` / `glDeleteShader(GLuint shader)`
- **역할:** Program에서 Shader 분리 / Shader 객체 삭제 대상으로 지정.
- **주의:** 링크 완료 후 소스 Shader 객체를 분리·삭제해도 링크된 Program은 계속 사용할 수 있다. 첨부된 Shader는 즉시 완전 삭제되지 않을 수 있다.

### `glDeleteProgram(GLuint program)`
- **역할:** Program 삭제 대상으로 지정.
- **주의:** 사용 중인 Program은 사용 해제 후 최종 삭제될 수 있다.

## 5. Uniform API

### `GLint glGetUniformLocation(GLuint program, const GLchar* name)`
- **역할:** 지정한 **성공적으로 링크된** Program의 활성 Uniform 위치 조회.
- `program`: Program ID.
- `name`: GLSL 변수명 (`"uMVP"` 등).
- **반환:** `GLint` Location, 없거나 최적화로 제거되는 등 비활성이면 `-1`.
- **주의:** Location은 Program별로 독립적이며, 링크 이후 다시 조회해야 한다.

### `glUniform1f(GLint location, GLfloat v0)`
- **역할:** 현재 활성 Program의 `float` Uniform 설정.
- `v0`: float 값.

### `glUniform2f(GLint location, GLfloat v0, GLfloat v1)`
- **역할:** 현재 활성 Program의 `vec2` Uniform 설정.

### `glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)`
- **역할:** 현재 활성 Program의 `vec3` Uniform 설정.

### `glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)`
- **역할:** 현재 활성 Program의 `vec4` Uniform 설정. `mat4`에 사용할 수 없다.

### `glUniform1i(GLint location, GLint v0)`
- **역할:** 현재 활성 Program의 `int`, `bool`, sampler 계열 Uniform 등에 정수 설정.

### `glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)`
- **역할:** 현재 활성 Program의 `mat4` Uniform 설정.
- `location`: 현재 Program의 Uniform Location.
- `count`: 행렬 개수(한 개면 `1`).
- `transpose`: 행렬 데이터 전치 여부(`GL_FALSE`는 전치하지 않음, 일반적인 GLM 사용 시 `GL_FALSE`).
- `value`: 연속된 float 행렬 데이터 주소(`glm::value_ptr(MVP)`).

### `glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)` — DSA 스타일
- **역할:** **지정한 Program의** `mat4` Uniform을 직접 설정.
- `program`: 변경할 Program ID. 다른 매개변수는 위 함수와 동일.
- **특징:** `glUseProgram()` 없이 값 설정 가능. **활성 프로그램은 바꾸지 않는다.**

### `glProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)` — DSA 스타일
- **역할:** 지정한 Program의 `vec4` Uniform 직접 설정.

## 6. Uniform 상태의 생명주기

```cpp
glUseProgram(programA);
glUniformMatrix4fv(locA, 1, GL_FALSE, glm::value_ptr(MVP_A));

glUseProgram(programB);           // A의 Uniform 값은 유지됨
glProgramUniformMatrix4fv(programA, locA, 1, GL_FALSE,
                         glm::value_ptr(MVP_A2)); // 현재 Program은 여전히 B

glUseProgram(programA);           // 재선택하면 MVP_A2를 사용
```

- **같은 Program + VAO 변경:** Uniform 값은 그대로 유지.
- **같은 Program + Draw Call 반복:** 별도 변경하지 않으면 Uniform은 유지.
- **Program 변경:** 각 Program에 저장된 Uniform은 독립적으로 유지.
- **Program 링크 성공:** Uniform 값 초기화, Location 재조회 필수.
- **Location = -1:** Uniform 설정 함수에 전달하면 그 호출은 무시된다(다른 오류 없는 상황).
- Uniform은 `.vert`/`.frag`의 변수 선언이고, `glUniform*()`는 **그 변수에 값을 지정하는 OpenGL 함수**다.

## 7. Legacy ↔ DSA 요약

| 작업 | 기존 방식 | DSA 스타일 |
| --- | --- | --- |
| Shader 생성·소스·컴파일 | `glCreateShader` → `glShaderSource` → `glCompileShader` | **동일** |
| Program 생성·첨부·링크 | `glCreateProgram` → `glAttachShader` → `glLinkProgram` | **동일** |
| Location 조회 | `glGetUniformLocation` | **동일** |
| `mat4` 값 설정 | `glUseProgram` + `glUniformMatrix4fv` | `glProgramUniformMatrix4fv` |
| `vec4` 값 설정 | `glUseProgram` + `glUniform4f` | `glProgramUniform4f` |
| 그리기에 사용할 Program 선택 | `glUseProgram` | **동일** |

> **참고:** Shader/Program 생성 자체가 DSA 전용 API로 바뀌는 것은 아니다. 여기서 DSA 비교는 **Uniform 값을 설정할 대상을 지정하는 방식**을 뜻한다. `glProgramUniform*()`는 OpenGL 4.1 이상에서 제공되지만, 정점 배열 DSA의 `glCreateVertexArrays()` 등은 OpenGL 4.5 API다.

**공식 문서:** [Khronos OpenGL 4 Reference Pages](https://registry.khronos.org/OpenGL-Refpages/gl4/)


---

## 원본 Wiki 상세 레퍼런스 — 매개변수 및 주의사항 전체 기록

> `mesh.hpp`에 작성했던 원본 학습 주석을 함수 단위로 보존한 상세 참고 자료입니다. 위쪽의 요약 설명과 함께 사용하세요.

### 7. Shader 객체 및 Shader Program

```cpp
// [기본 개념]
```

```cpp
// Shader Object:
//   특정 Shader 단계의 GLSL 소스 코드 및 컴파일 결과를 관리하는 객체.
//   예: Vertex Shader, Fragment Shader.
```

```cpp
// Shader Program Object:
//   Shader 객체들을 Attach한 뒤 Link하여 구성하는 프로그램 객체.
//   링크된 실행 코드와 Uniform 값 등의 상태를 관리한다.
```

```cpp
// Shader 생성 과정:
//   1. GLSL 소스 코드 준비
//   2. glCreateShader()     -> Shader 객체 생성
//   3. glShaderSource()     -> GLSL 소스 저장
//   4. glCompileShader()    -> Shader 컴파일
//   5. glCreateProgram()    -> Program 객체 생성
//   6. glAttachShader()     -> Shader 객체 첨부
//   7. glLinkProgram()      -> Program 링크
//   8. glUseProgram()       -> Program 활성화
```

```cpp
// 주의:
//   Compile과 Link는 반드시 성공 여부를 확인해야 한다.
//   Attach는 연결할 Shader를 등록하는 과정이고,
//   Link는 실제로 Shader들을 연결하고 프로그램을 완성하는 과정이다.
```

#### `glCreateShader(GLenum type)`

```cpp
// glCreateShader(GLenum type)
```

```cpp
//   역할: 지정한 종류의 Shader 객체를 생성하고 Shader ID를 반환.
```

```cpp
//   type = 생성할 Shader의 종류.
//          GL_VERTEX_SHADER   : 정점 처리
//          GL_FRAGMENT_SHADER : Fragment 색상 등의 계산
//          GL_GEOMETRY_SHADER : Primitive 처리
//          GL_COMPUTE_SHADER  : 범용 GPU 계산
```

```cpp
//   반환값:
//     GLuint 형태의 Shader 객체 ID.
//     생성에 실패하면 0을 반환한다.
```

```cpp
//   주의:
//     Shader 객체만 생성하며 GLSL 소스 코드가 자동으로 저장되거나
//     컴파일되는 것은 아니다.
```

```cpp
// 예시:
//   GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//   GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
```

#### `glShaderSource(`

```cpp
// glShaderSource(
//     GLuint shader,
//     GLsizei count,
//     const GLchar* const* string,
//     const GLint* length
// )
```

```cpp
//   역할: GLSL 소스 코드를 지정한 Shader 객체에 복사하여 저장.
```

```cpp
//   shader = GLSL 소스를 저장할 Shader 객체 ID.
```

```cpp
//   count  = 전달할 소스 문자열의 개수.
//            문자열 하나라면 1을 사용한다.
```

```cpp
//   string = 소스 문자열 포인터들을 담은 배열의 시작 주소.
//            문자열 하나만 전달하더라도 문자열 포인터의 주소가 필요하다.
```

```cpp
//   length = 각 문자열의 길이를 저장한 배열의 주소.
//            nullptr이면 각 문자열의 null 종료 문자('\0')를 기준으로
//            길이를 판단한다.
```

```cpp
//   주의:
```

#### `glShaderSource()는 GLSL 코드를 컴파일하지 않는다.`

```cpp
//     glShaderSource()는 GLSL 코드를 컴파일하지 않는다.
//     실제 컴파일은 glCompileShader()에서 수행한다.
```

```cpp
//     std::string을 사용할 경우 c_str()을 통해
//     const char* 형태의 문자열 포인터를 얻을 수 있다.
```

```cpp
// 예시:
//   std::string sourceText = readShaderFile("scene.vert");
//   const char* source = sourceText.c_str();
```

#### `glShaderSource(vertexShader, 1, &source, nullptr);`

```cpp
//   glShaderSource(vertexShader, 1, &source, nullptr);
```

#### `glCompileShader(GLuint shader)`

```cpp
// glCompileShader(GLuint shader)
```

```cpp
//   역할: Shader 객체에 저장된 GLSL 소스 코드를 컴파일.
```

```cpp
//   shader = 컴파일할 Shader 객체 ID.
```

```cpp
//   주의:
//     함수를 호출했다고 반드시 컴파일에 성공하는 것은 아니다.
```

#### `glGetShaderiv()로 GL_COMPILE_STATUS를 확인해야 한다.`

```cpp
//     glGetShaderiv()로 GL_COMPILE_STATUS를 확인해야 한다.
```

```cpp
// 예시:
```

#### `glCompileShader(vertexShader);`

```cpp
//   glCompileShader(vertexShader);
```

#### `glGetShaderiv(GLuint shader, GLenum pname, GLint* params)`

```cpp
// glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
```

```cpp
//   역할: Shader 객체의 특정 상태 정보를 조회.
```

```cpp
//   shader = 상태를 조회할 Shader 객체 ID.
```

```cpp
//   pname  = 조회할 상태의 종류.
//            GL_COMPILE_STATUS : 컴파일 성공 여부
//            GL_INFO_LOG_LENGTH: 진단 로그의 길이
//            GL_SHADER_TYPE    : Shader 종류
```

```cpp
//   params = 조회한 결과를 저장할 변수의 메모리 주소.
```

```cpp
//   주의:
//     GL_COMPILE_STATUS는 GL_TRUE 또는 GL_FALSE를 반환한다.
```

```cpp
// 예시:
//   GLint success = GL_FALSE;
```

#### `glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);`

```cpp
//   glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
```

```cpp
//   if (success != GL_TRUE) {
//       // 컴파일 실패 처리
//   }
```

#### `glGetShaderInfoLog(`

```cpp
// glGetShaderInfoLog(
//     GLuint shader,
//     GLsizei maxLength,
//     GLsizei* length,
//     GLchar* infoLog
// )
```

```cpp
//   역할: Shader 컴파일 과정에서 생성된 진단 로그를 조회.
```

```cpp
//   shader    = 조회할 Shader 객체 ID.
```

```cpp
//   maxLength = infoLog 버퍼에 저장할 수 있는 최대 문자 수(Byte).
```

```cpp
//   length    = 실제 기록된 문자 수를 저장할 변수의 주소.
//               필요하지 않으면 nullptr 사용 가능.
```

```cpp
//   infoLog   = 진단 로그 문자열을 저장할 메모리 버퍼.
```

```cpp
//   주의:
//     컴파일 실패 시 오류 원인을 확인하는 데 사용한다.
//     컴파일 성공 여부는 GL_COMPILE_STATUS로 판단한다.
```

```cpp
// 예시:
//   char infoLog[1024] = {};
```

#### `glGetShaderInfoLog(vertexShader, 1024, nullptr, infoLog);`

```cpp
//   glGetShaderInfoLog(vertexShader, 1024, nullptr, infoLog);
```

#### `glCreateProgram()`

```cpp
// glCreateProgram()
```

```cpp
//   역할: 새로운 Shader Program 객체를 생성하고 ID를 반환.
```

```cpp
//   매개변수: 없음.
```

```cpp
//   반환값:
//     GLuint 형태의 Shader Program ID.
//     생성에 실패하면 0을 반환한다.
```

```cpp
//   주의:
//     빈 Shader Program 객체를 생성하는 함수이다.
//     Shader를 자동으로 첨부하거나 링크하지 않는다.
```

```cpp
// 예시:
//   GLuint program = glCreateProgram();
```

#### `glAttachShader(GLuint program, GLuint shader)`

```cpp
// glAttachShader(GLuint program, GLuint shader)
```

```cpp
//   역할: Shader 객체를 Shader Program 객체에 첨부.
```

```cpp
//   program = Shader를 첨부할 Shader Program 객체 ID.
```

```cpp
//   shader  = 첨부할 Shader 객체 ID.
```

```cpp
//   주의:
//     Attach는 Shader 객체를 Program에 등록하는 작업이다.
//     실제 링크는 glLinkProgram()을 호출해야 수행된다.
```

```cpp
//     하나의 Shader 객체를 여러 Program에 첨부할 수 있다.
//     일반적인 프로그램에서는 동일 Shader 단계의 객체를
//     하나의 Program에 중복 첨부할 수 없다.
```

```cpp
// 예시:
```

#### `glAttachShader(program, vertexShader);`

```cpp
//   glAttachShader(program, vertexShader);
```

#### `glAttachShader(program, fragmentShader);`

```cpp
//   glAttachShader(program, fragmentShader);
```

#### `glLinkProgram(GLuint program)`

```cpp
// glLinkProgram(GLuint program)
```

```cpp
//   역할: Program에 첨부된 Shader 객체들을 링크하여
//         실행 가능한 Shader Program을 구성.
```

```cpp
//   program = 링크할 Shader Program 객체 ID.
```

```cpp
//   주요 작업:
//     - Shader 단계 간 입력/출력 인터페이스 검사
//     - Attribute 및 Uniform 등의 리소스 정보 결정
//     - Shader 단계들을 연결하여 실행 준비
```

```cpp
//   주의:
//     Link에 실패하면 해당 프로그램을 정상적으로 사용할 수 없다.
//     GL_LINK_STATUS로 성공 여부를 확인해야 한다.
```

```cpp
//     성공적으로 다시 링크하면 일반 Uniform 값이 초기화되고,
//     Uniform Location도 변경될 수 있다.
```

```cpp
// 예시:
```

#### `glLinkProgram(program);`

```cpp
//   glLinkProgram(program);
```

#### `glGetProgramiv(GLuint program, GLenum pname, GLint* params)`

```cpp
// glGetProgramiv(GLuint program, GLenum pname, GLint* params)
```

```cpp
//   역할: Shader Program 객체의 상태 정보를 조회.
```

```cpp
//   program = 상태를 조회할 Shader Program ID.
```

```cpp
//   pname   = 조회할 상태의 종류.
//             GL_LINK_STATUS      : 링크 성공 여부
//             GL_INFO_LOG_LENGTH  : 진단 로그 길이
//             GL_ATTACHED_SHADERS : 첨부된 Shader 개수
```

```cpp
//   params  = 조회한 결과를 저장할 변수의 주소.
```

```cpp
// 예시:
//   GLint success = GL_FALSE;
```

#### `glGetProgramiv(program, GL_LINK_STATUS, &success);`

```cpp
//   glGetProgramiv(program, GL_LINK_STATUS, &success);
```

#### `glGetProgramInfoLog(`

```cpp
// glGetProgramInfoLog(
//     GLuint program,
//     GLsizei maxLength,
//     GLsizei* length,
//     GLchar* infoLog
// )
```

```cpp
//   역할: Shader Program의 링크 관련 진단 로그를 조회.
```

```cpp
//   program   = 조회할 Shader Program ID.
```

```cpp
//   maxLength = 로그를 저장할 버퍼의 최대 크기(Byte).
```

```cpp
//   length    = 실제 기록된 문자 수를 저장할 변수의 주소.
//               불필요하면 nullptr 가능.
```

```cpp
//   infoLog   = 로그를 저장할 메모리 버퍼.
```

```cpp
// 예시:
//   char infoLog[1024] = {};
```

#### `glGetProgramInfoLog(program, 1024, nullptr, infoLog);`

```cpp
//   glGetProgramInfoLog(program, 1024, nullptr, infoLog);
```

#### `glUseProgram(GLuint program)`

```cpp
// glUseProgram(GLuint program)
```

```cpp
//   역할: 현재 OpenGL Context에서 사용할 Shader Program을 선택.
```

```cpp
//   program = 활성화할 Shader Program ID.
//             0을 전달하면 현재 Program 사용을 해제한다.
```

```cpp
//   주의:
//     성공적으로 링크된 Program을 사용해야 한다.
```

#### `glUseProgram()은 Program 객체를 삭제하지 않는다.`

```cpp
//     glUseProgram()은 Program 객체를 삭제하지 않는다.
```

```cpp
//     다른 Program을 활성화하더라도 이전 Program의 Uniform 값은 유지된다.
//     실제 Shader 실행은 이후 Draw Call 등에서 이루어진다.
```

```cpp
// 예시:
```

#### `glUseProgram(program);`

```cpp
//   glUseProgram(program);
```

#### `glDetachShader(GLuint program, GLuint shader)`

```cpp
// glDetachShader(GLuint program, GLuint shader)
```

```cpp
//   역할: Program에 첨부된 Shader 객체를 분리.
```

```cpp
//   program = 대상 Shader Program ID.
//   shader  = 분리할 Shader 객체 ID.
```

```cpp
//   주의:
//     링크에 성공한 Program에서는 Shader를 분리해도
//     이미 링크된 실행 코드를 계속 사용할 수 있다.
```

```cpp
// 예시:
```

#### `glDetachShader(program, vertexShader);`

```cpp
//   glDetachShader(program, vertexShader);
```

#### `glDeleteShader(GLuint shader)`

```cpp
// glDeleteShader(GLuint shader)
```

```cpp
//   역할: Shader 객체를 삭제 대상으로 지정.
```

```cpp
//   shader = 삭제할 Shader 객체 ID.
```

```cpp
//   주의:
//     Shader가 Program에 첨부되어 있다면 실제 삭제는
//     해당 Shader가 모든 Program에서 분리된 이후 이루어진다.
```

```cpp
// 예시:
```

#### `glDeleteShader(vertexShader);`

```cpp
//   glDeleteShader(vertexShader);
```

#### `glDeleteProgram(GLuint program)`

```cpp
// glDeleteProgram(GLuint program)
```

```cpp
//   역할: Shader Program 객체를 삭제 대상으로 지정.
```

```cpp
//   program = 삭제할 Shader Program ID.
```

```cpp
//   주의:
//     현재 사용 중인 Program은 사용이 해제될 때 실제 삭제된다.
//     Program 삭제는 Buffer나 VAO 삭제와 별개의 작업이다.
```

```cpp
// 예시:
```

#### `glDeleteProgram(program);`

```cpp
//   glDeleteProgram(program);
```

### 8. Uniform

```cpp
// [기본 개념]
```

```cpp
// Uniform:
//   GLSL에서 선언하는 프로그램 공통 입력 변수.
```

```cpp
//   CPU 측 OpenGL 코드에서 값을 설정하고,
//   Shader가 실행될 때 해당 값을 읽어서 계산에 사용한다.
```

```cpp
// 예시 (GLSL):
//   uniform mat4 uMVP;
//   uniform vec4 uColor;
//   uniform float uTime;
```

```cpp
// 특징:
//   - Vertex Attribute와 달리 정점마다 자동으로 값이 바뀌지 않는다.
//   - 동일 Draw Call 동안 모든 Shader 실행에서 같은 값을 사용한다.
//   - Uniform 값은 Shader Program 객체에 저장된다.
//   - Program을 비활성화해도 Uniform 값이 유지된다.
//   - 성공적으로 다시 링크하면 일반 Uniform 값이 초기화된다.
```

```cpp
// Uniform 설정 과정:
//   1. Shader Program 링크
//   2. glGetUniformLocation()으로 Uniform 위치 조회
//   3. glUseProgram()으로 Program 활성화
//   4. glUniform*()으로 Uniform 값 설정
```

```cpp
// DSA 스타일에서는 glProgramUniform*()을 이용하여
// Program을 활성화하지 않고도 특정 Program의 값을 설정할 수 있다.
```

#### `glGetUniformLocation(GLuint program, const GLchar* name)`

```cpp
// glGetUniformLocation(GLuint program, const GLchar* name)
```

```cpp
//   역할: 지정한 Shader Program 내부에서 Uniform Location 조회.
```

```cpp
//   program = Uniform을 조회할 Shader Program 객체 ID.
```

```cpp
//   name    = Uniform 변수의 이름.
//             GLSL에 선언한 변수 이름과 일치해야 한다.
```

```cpp
//   반환값:
//     GLint 형태의 Uniform Location.
```

```cpp
//     -1 : 해당 이름의 활성 Uniform이 존재하지 않거나
//          최적화 등에 의해 제거되어 위치를 조회할 수 없는 경우.
```

```cpp
//   주의:
//     Uniform Location은 Program마다 독립적으로 관리된다.
//     서로 다른 Program에서 같은 이름을 사용해도 Location은 다를 수 있다.
```

```cpp
//     Program을 다시 링크하면 Location을 다시 조회해야 한다.
```

```cpp
//     Location은 CPU 메모리 주소나 GPU 메모리 주소가 아니라
//     Program 내부의 Uniform을 식별하는 번호이다.
```

```cpp
// 예시:
//   GLint location = glGetUniformLocation(program, "uMVP");
```

#### `glUniform1f(GLint location, GLfloat v0)`

```cpp
// glUniform1f(GLint location, GLfloat v0)
```

```cpp
//   역할: 현재 활성화된 Program의 float Uniform에 값 하나 설정.
```

```cpp
//   location = Uniform Location.
//   v0       = 전달할 float 값.
```

```cpp
// 예시 (GLSL):
//   uniform float uTime;
```

```cpp
// 예시 (C++):
```

#### `glUniform1f(location, 1.5f);`

```cpp
//   glUniform1f(location, 1.5f);
```

#### `glUniform2f(GLint location, GLfloat v0, GLfloat v1)`

```cpp
// glUniform2f(GLint location, GLfloat v0, GLfloat v1)
```

```cpp
//   역할: 현재 활성화된 Program의 vec2 Uniform 값 설정.
```

```cpp
//   location = Uniform Location.
//   v0       = 첫 번째 float 성분.
//   v1       = 두 번째 float 성분.
```

```cpp
// 예시 (GLSL):
//   uniform vec2 uPosition;
```

```cpp
// 예시 (C++):
```

#### `glUniform2f(location, 0.5f, 0.25f);`

```cpp
//   glUniform2f(location, 0.5f, 0.25f);
```

#### `glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)`

```cpp
// glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
```

```cpp
//   역할: 현재 활성화된 Program의 vec3 Uniform 값 설정.
```

```cpp
//   location = Uniform Location.
//   v0       = 첫 번째 float 성분.
//   v1       = 두 번째 float 성분.
//   v2       = 세 번째 float 성분.
```

```cpp
// 예시 (GLSL):
//   uniform vec3 uColor;
```

```cpp
// 예시 (C++):
```

#### `glUniform3f(location, 1.0f, 0.0f, 0.0f);`

```cpp
//   glUniform3f(location, 1.0f, 0.0f, 0.0f);
```

#### `glUniform4f(`

```cpp
// glUniform4f(
//     GLint location,
//     GLfloat v0,
//     GLfloat v1,
//     GLfloat v2,
//     GLfloat v3
// )
```

```cpp
//   역할: 현재 활성화된 Program의 vec4 Uniform 값 설정.
```

```cpp
//   location = Uniform Location.
//   v0       = 첫 번째 float 성분.
//   v1       = 두 번째 float 성분.
//   v2       = 세 번째 float 성분.
//   v3       = 네 번째 float 성분.
```

```cpp
//   주의:
```

#### `glUniform4f()는 float 성분 네 개를 전달하는 함수다.`

```cpp
//     glUniform4f()는 float 성분 네 개를 전달하는 함수다.
//     mat4 Uniform을 설정하는 함수가 아니다.
```

```cpp
// 예시 (GLSL):
//   uniform vec4 uColor;
```

```cpp
// 예시 (C++):
```

#### `glUniform4f(location, 1.0f, 0.0f, 0.0f, 1.0f);`

```cpp
//   glUniform4f(location, 1.0f, 0.0f, 0.0f, 1.0f);
```

#### `glUniform1i(GLint location, GLint v0)`

```cpp
// glUniform1i(GLint location, GLint v0)
```

```cpp
//   역할: 현재 활성화된 Program의 int Uniform 값 설정.
```

```cpp
//   location = Uniform Location.
//   v0       = 전달할 정수 값.
```

```cpp
//   주의:
//     bool, sampler 계열 Uniform을 설정할 때도 사용한다.
```

```cpp
// 예시 (GLSL):
//   uniform int uMode;
```

```cpp
// 예시 (C++):
```

#### `glUniform1i(location, 2);`

```cpp
//   glUniform1i(location, 2);
```

#### `glUniformMatrix4fv(`

```cpp
// glUniformMatrix4fv(
//     GLint location,
//     GLsizei count,
//     GLboolean transpose,
//     const GLfloat* value
// )
```

```cpp
//   역할: 현재 활성화된 Shader Program의 mat4 Uniform에
//         4x4 float 행렬 데이터를 전달.
```

```cpp
//   location  = Uniform Location.
```

```cpp
//   count     = 전달할 4x4 행렬의 개수.
//               mat4 하나를 전달할 경우 1.
```

```cpp
//   transpose = 행렬 전치 여부.
//               GL_FALSE : 전치하지 않음.
//               GL_TRUE  : 전치하여 전달.
```

```cpp
//   value     = 행렬 데이터가 저장된 메모리의 시작 주소.
//               float 배열 또는 glm::value_ptr() 등을 사용한다.
```

```cpp
//   주의:
//     GLSL의 mat4 Uniform에 사용할 수 있다.
//     GL_FALSE는 정규화 여부가 아니라 전치 여부를 의미한다.
```

```cpp
//     GLM의 일반적인 행렬 메모리 배치에서는 GL_FALSE를 사용한다.
```

```cpp
// 예시 (GLSL):
//   uniform mat4 uMVP;
```

```cpp
// 예시 (C++):
```

#### `glUniformMatrix4fv(`

```cpp
//   glUniformMatrix4fv(
//       location,
//       1,
//       GL_FALSE,
//       glm::value_ptr(MVP)
//   );
```

#### `glProgramUniformMatrix4fv(`

```cpp
// glProgramUniformMatrix4fv(
//     GLuint program,
//     GLint location,
//     GLsizei count,
//     GLboolean transpose,
//     const GLfloat* value
// )
```

```cpp
//   역할: 지정한 Shader Program의 mat4 Uniform 값을 직접 설정.
```

```cpp
//   program   = Uniform을 설정할 Shader Program ID.
```

```cpp
//   location  = 해당 Program에서 조회한 Uniform Location.
```

```cpp
//   count     = 전달할 행렬 개수.
```

```cpp
//   transpose = 행렬 전치 여부.
```

```cpp
//   value     = 행렬 데이터가 저장된 메모리의 시작 주소.
```

```cpp
//   특징:
//     - glUseProgram()을 호출하지 않아도 Uniform을 설정할 수 있다.
//     - 현재 활성화된 Shader Program은 변경되지 않는다.
//     - 지정한 Program의 Uniform 상태만 변경한다.
```

```cpp
//   주의:
//     location은 반드시 해당 Program에서 조회한 값을 사용해야 한다.
//     Program은 성공적으로 링크된 상태여야 한다.
```

```cpp
// 예시:
//   GLint locB = glGetUniformLocation(programB, "uMVP");
```

#### `glProgramUniformMatrix4fv(`

```cpp
//   glProgramUniformMatrix4fv(
//       programB,
//       locB,
//       1,
//       GL_FALSE,
//       glm::value_ptr(MVP)
//   );
```

#### `glProgramUniform4f(`

```cpp
// glProgramUniform4f(
//     GLuint program,
//     GLint location,
//     GLfloat v0,
//     GLfloat v1,
//     GLfloat v2,
//     GLfloat v3
// )
```

```cpp
//   역할: 지정한 Shader Program의 vec4 Uniform 값을 직접 설정.
```

```cpp
//   program  = Uniform 값을 설정할 Program ID.
//   location = 해당 Program의 Uniform Location.
//   v0       = 첫 번째 float 성분.
//   v1       = 두 번째 float 성분.
//   v2       = 세 번째 float 성분.
//   v3       = 네 번째 float 성분.
```

```cpp
// 예시:
```

#### `glProgramUniform4f(`

```cpp
//   glProgramUniform4f(
//       program,
//       location,
//       1.0f, 0.0f, 0.0f, 1.0f
//   );
```

