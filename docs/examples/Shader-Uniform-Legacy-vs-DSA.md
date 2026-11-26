# Shader / Uniform — Legacy vs DSA

> **학습 주제:** GLSL 읽기 → Shader 컴파일 → Program 링크 → Uniform 설정 → Draw Call.  
> **관련 Wiki:** [Shader / Uniform](../wiki/Shader-Uniform.md)

## 1. 전제

- 아래 코드는 `GLAD` 등으로 OpenGL 함수 로더 초기화와 Context 생성이 끝난 상황을 가정한다.
- 예제의 `Mesh`는 `vao`, `indexCount`를 가지고 있고, `glm`을 사용할 수 있다고 가정한다.
- 프로그램에서 `uMVP`/`uColor`를 실제 사용해야 `glGetUniformLocation()`이 유효한 위치를 반환한다.
- `.vert`, `.frag`는 **파일 확장자 관례**일 뿐 OpenGL이 직접 파일을 열지는 않는다.
- **중요:** Shader 생성/컴파일/링크는 Legacy와 DSA 모두 동일하다. 다른 부분은 `glUniform*()`과 `glProgramUniform*()`이다.

## A. Shader 파일 읽기 — 공통

```cpp
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

inline std::string readShaderFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Cannot open shader file: " + path);

    std::ostringstream text;
    text << file.rdbuf();
    return text.str();
}
```

## B. Shader 객체 생성·컴파일 — 공통

```cpp
inline GLuint compileShader(GLenum type, const std::string& source)
{
    // 1. Shader 객체 생성
    GLuint shader = glCreateShader(type);
    if (shader == 0)
        throw std::runtime_error("glCreateShader failed");

    // 2. 소스 저장 (const char*의 주소가 필요함)
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);

    // 3. 컴파일
    glCompileShader(shader);

    // 4. 성공 여부 확인
    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success != GL_TRUE)
    {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> log(logLength > 0 ? logLength : 1, '\0');
        glGetShaderInfoLog(shader,
                           static_cast<GLsizei>(log.size()),
                           nullptr, log.data());

        std::string message(log.data());
        glDeleteShader(shader);
        throw std::runtime_error("Shader compile failed: " + message);
    }

    return shader;
}
```

## C. Shader Program 생성·링크 — 공통

```cpp
inline GLuint createShaderProgram(
    const std::string& vertexPath,
    const std::string& fragmentPath)
{
    // 1. CPU에서 GLSL 파일을 읽음
    const std::string vertSource = readShaderFile(vertexPath);
    const std::string fragSource = readShaderFile(fragmentPath);

    // 2. 각 Shader 단계 컴파일
    const GLuint vertex = compileShader(GL_VERTEX_SHADER, vertSource);
    GLuint fragment = 0;
    try {
        fragment = compileShader(GL_FRAGMENT_SHADER, fragSource);
    } catch (...) {
        glDeleteShader(vertex);
        throw;
    }

    // 3. Program 생성 및 Shader 첨부
    GLuint program = glCreateProgram();
    if (program == 0) {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        throw std::runtime_error("glCreateProgram failed");
    }

    glAttachShader(program, vertex);
    glAttachShader(program, fragment);

    // 4. Link
    glLinkProgram(program);

    // 5. Link 상태 및 로그 확인
    GLint success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success != GL_TRUE)
    {
        GLint logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> log(logLength > 0 ? logLength : 1, '\0');
        glGetProgramInfoLog(program,
                            static_cast<GLsizei>(log.size()),
                            nullptr, log.data());

        std::string message(log.data());
        glDeleteProgram(program);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        throw std::runtime_error("Program link failed: " + message);
    }

    // 6. 성공 후 원본 Shader 객체는 분리·삭제 가능
    glDetachShader(program, vertex);
    glDetachShader(program, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}
```

## D. 기존 방식 — 활성 Program에 Uniform 전달

```cpp
inline void setUniformLegacy(
    GLuint program, const glm::mat4& MVP, const glm::vec4& color)
{
    // 1. 대상을 현재 Program으로 선택
    glUseProgram(program);

    // 2. Program별 Uniform Location 조회
    GLint mvpLocation   = glGetUniformLocation(program, "uMVP");
    GLint colorLocation = glGetUniformLocation(program, "uColor");

    // 3. 현재 활성 Program의 Uniform 값 설정
    if (mvpLocation != -1)
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(MVP));

    if (colorLocation != -1)
        glUniform4f(colorLocation, color.r, color.g, color.b, color.a);
}
```

## E. DSA 스타일 — Program ID를 지정해 Uniform 전달

```cpp
inline void setUniformDSA(
    GLuint program, const glm::mat4& MVP, const glm::vec4& color)
{
    // 1. Program을 활성화하지 않고 Location 조회
    GLint mvpLocation   = glGetUniformLocation(program, "uMVP");
    GLint colorLocation = glGetUniformLocation(program, "uColor");

    // 2. Program ID를 명시적으로 지정해 Uniform 값 설정
    if (mvpLocation != -1)
        glProgramUniformMatrix4fv(
            program, mvpLocation, 1, GL_FALSE, glm::value_ptr(MVP));

    if (colorLocation != -1)
        glProgramUniform4f(
            program, colorLocation,
            color.r, color.g, color.b, color.a);
}
```

## F. 두 방식에서 공통인 그리기 과정

```cpp
inline void drawWithProgram(GLuint program, const Mesh& mesh)
{
    if (mesh.vao == 0) return;

    // DSA로 Uniform 값을 설정했더라도 Draw Call용 Program은 선택해야 함
    glUseProgram(program);
    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
}
```

## G. 사용 흐름

```cpp
GLuint program = createShaderProgram("scene.vert", "scene.frag");

// 실제 사용 시 MVP와 색상을 원하는 값으로 계산
const glm::mat4 MVP(1.0f);
const glm::vec4 color(1.0f, 0.0f, 0.0f, 1.0f);

// 아래 중 한 가지 방식 사용
setUniformLegacy(program, MVP, color);
// setUniformDSA(program, MVP, color);

drawWithProgram(program, house);

// 종료 시 (유효한 OpenGL Context 존재 필요)
glDeleteProgram(program);
```

## H. 기존 ↔ DSA 매칭 요약

| 작업 | Legacy | DSA 스타일 |
| --- | --- | --- |
| 파일 읽기 | C++ `ifstream` | **동일** |
| Shader 생성 | `glCreateShader` | **동일** |
| 소스/컴파일 | `glShaderSource`, `glCompileShader` | **동일** |
| Shader 상태 검사 | `glGetShaderiv`, `glGetShaderInfoLog` | **동일** |
| Program 생성·첨부·링크 | `glCreateProgram`, `glAttachShader`, `glLinkProgram` | **동일** |
| 링크 상태 검사 | `glGetProgramiv`, `glGetProgramInfoLog` | **동일** |
| Uniform 위치 조회 | `glGetUniformLocation` | **동일** |
| `mat4` 전달 | `glUseProgram` + `glUniformMatrix4fv` | `glProgramUniformMatrix4fv` |
| `vec4` 전달 | `glUseProgram` + `glUniform4f` | `glProgramUniform4f` |
| `float` 전달 | `glUseProgram` + `glUniform1f` | `glProgramUniform1f` |
| `int` 전달 | `glUseProgram` + `glUniform1i` | `glProgramUniform1i` |
| Draw Call | `glUseProgram` + `glBindVertexArray` + `glDrawElements` | **동일** |
| 자원 해제 | `glDetachShader`, `glDeleteShader`, `glDeleteProgram` | **동일** |

## I. 자주 헷갈리는 상태

```cpp
glUseProgram(programA);
glProgramUniformMatrix4fv(programB, locB, 1, GL_FALSE,
                          glm::value_ptr(MVP_B));

glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
// Draw는 여전히 programA 사용. programB의 Uniform 설정은 programA를 활성화하지 않음.
```

- **Uniform Location은 Program마다 다를 수 있다.** `locB`는 `programB`에서 조회해야 한다.
- Uniform은 VAO에 저장되는 값이 아니라 Program의 상태다.
- 같은 Program에서 Draw Call을 여러 번 해도 변경하지 않은 Uniform은 유지된다.
- Program을 성공적으로 **재링크**하면 일반 Uniform 값은 초기화되고 Location도 다시 조회해야 한다.
- 이 예제에서 `glProgramUniform*()`는 OpenGL 4.1+에 해당한다. VAO/VBO의 OpenGL 4.5 DSA와 요구 버전이 다르다.
