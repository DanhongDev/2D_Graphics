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
