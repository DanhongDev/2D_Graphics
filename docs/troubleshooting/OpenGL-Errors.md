# OpenGL Troubleshooting — 오류 및 의문점 기록

> OpenGL 학습 과정에서 발생한 오류, 원인 분석, 해결 방법을 축적하는 문서.  
> **기록 원칙:** 실제로 확인한 결과와 아직 검증하지 않은 원인 후보를 구분한다.

## 1. 오류 코드 빠른 참조

| 오류 | 16진수 | 의미 | 우선 확인할 내용 |
| --- | --- | --- | --- |
| `GL_INVALID_ENUM` | `0x0500` (`1280`) | 허용되지 않는 enum 인자를 넘김 | API의 `GL_*` 상수 이름과 지원 값 |
| `GL_INVALID_VALUE` | `0x0501` (`1281`) | 값의 범위/크기 등이 잘못됨 | 개수, 오프셋, 범위 |
| `GL_INVALID_OPERATION` | `0x0502` (`1282`) | 현재 상태에서 허용되지 않는 연산 | VAO/Program 상태, 객체 종류, API 선행조건 |
| `GL_OUT_OF_MEMORY` | `0x0505` (`1285`) | 메모리 부족 | 버퍼/텍스처 크기, 리소스 누수 |
| `GL_INVALID_FRAMEBUFFER_OPERATION` | `0x0506` (`1286`) | 불완전한 Framebuffer 상태에서 연산 | FBO 첨부 이미지 및 완성 상태 |

> 오류 코드는 **원인을 확정하지 않는다**. 오류가 처음 발생한 함수와 해당 인자를 함께 확인해야 한다.

## 2. 실제 학습 사례: OpenGL Error 1280

**상태: 원인 후보 확인 / 수정 후 재검증 필요**

### 문제 상황

- A1 `mesh.hpp`에서 VAO/VBO/EBO 설정 및 Attribute 형식을 구현하는 과정에서 `OpenGL error: 1280`을 경험함.
- `1280`은 `GL_INVALID_ENUM`으로 해석된다.

### 코드에서 확인된 점

제공되었던 `mesh.hpp`의 색상 Attribute 설정에 다음 인자가 들어 있었다.

```cpp
glVertexAttribPointer(
    1, 3, GL_FLAT, GL_FALSE, sizeof(Vertex),
    reinterpret_cast<void*>(offsetof(Vertex, r))
);
```

`glVertexAttribPointer()`의 `type`에는 `GL_FLOAT`, `GL_INT` 등 **허용되는 데이터 자료형 enum**을 전달해야 한다. `GL_FLAT`은 그 자리에 사용할 타입 enum이 아니다.

### 수정 후보

```cpp
glVertexAttribPointer(
    1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
    reinterpret_cast<void*>(offsetof(Vertex, r))
);
```

### 검증 단계

1. 문제의 `GL_FLAT`을 `GL_FLOAT`로 수정한다.
2. 오류 체크가 **해당 함수 직후** 실행되도록 하여 `GL_INVALID_ENUM`이 다시 나타나는지 확인한다.
3. `GL_FLOAT` 수정 후 렌더링 결과와 `GL_NO_ERROR` 확인 결과를 기록한다.
4. 다른 오류가 나타나면 새로운 오류 코드와 해당 호출을 별도로 기록한다.

**주의:** 위 내용은 제공된 코드에서 확인되는 구체적인 잘못된 인자와 수정 방향이다. 실제 실행에서 이것이 유일한 오류 원인이었는지는 별도 검증이 필요하다.

## 3. 오류 위치 찾기: `glGetError()`

```cpp
#include <cstdio>

inline void checkOpenGLErrors(const char* step)
{
    GLenum error = GL_NO_ERROR;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        std::fprintf(stderr,
                     "[OpenGL] %s: error 0x%04X (%u)\n",
                     step,
                     static_cast<unsigned>(error),
                     static_cast<unsigned>(error));
    }
}
```

사용 예:

```cpp
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                      sizeof(Vertex),
                      reinterpret_cast<void*>(offsetof(Vertex, r)));
checkOpenGLErrors("color attribute");
```

- `glGetError()`는 **이전에 누적된 오류**를 반환할 수도 있으므로, 확인하려는 구간 앞에서 기존 오류를 비우고 작은 단위로 검사하면 원인 추적에 도움이 된다.
- 하나의 호출 뒤에 `glGetError()`를 확인하면, 어느 호출이 문제인지 범위를 좁힐 수 있다.
- OpenGL Debug Context에서 `glDebugMessageCallback`를 사용할 수 있다면 보다 자세한 오류 메시지를 얻을 수 있다.
- RenderDoc으로 Draw Call의 입력 버퍼, VAO 상태, Shader/Uniform 등을 살펴보는 것도 유용하다.

## 4. 자주 하는 실수와 점검 포인트

| 증상 | 가능한 원인 | 확인 방법 |
| --- | --- | --- |
| 아무것도 그려지지 않음 | Shader 링크 실패, VAO 미바인딩, indexCount 0, 잘못된 좌표/행렬 등 | Compile/Link Status, 바인딩, viewport, MVP 확인 |
| 색상이 이상함 | Attribute 1 형식/offset 오류, `GL_FLAT` 같은 잘못된 enum | `GL_FLOAT`, `offsetof(Vertex, r)`, Shader `layout(location=1)` 확인 |
| 인덱스로 그리기 오류 | EBO 연결 누락, 자료형/개수 불일치 | 현재 VAO의 EBO, `GL_UNSIGNED_INT`, `indexCount` 확인 |
| Uniform 값이 적용 안 됨 | `glUseProgram` 대상 불일치, `location = -1`, 다른 Program Location 사용 | 현재 Program, 조회 Program, GLSL의 활성 Uniform 확인 |
| DSA 함수가 실행 안 됨 | OpenGL Context 버전 또는 로더에서 함수 미지원 | 런타임 OpenGL 버전과 확장/함수 포인터 확인 |
| 불변 VBO 업데이트 실패 | `glNamedBufferStorage(..., flags=0)` 후 부분 갱신 시도 | 생성 시 `GL_DYNAMIC_STORAGE_BIT` 필요 여부 점검 |

## 5. 개념적 의문점 정리

### Q1. VAO만 바인딩하면 VBO/EBO를 찾을 수 있는가?

**답:** 적절하게 설정된 VAO라면 가능하다. VAO에는 Attribute 읽기 형식, VBO 연결, EBO 연결 정보가 저장된다. 단, `GL_ARRAY_BUFFER` 바인딩 자체가 VAO 상태인 것은 아니다.

### Q2. Legacy에서 `glBindBuffer(GL_ARRAY_BUFFER, otherVbo)`만 하면 이전 VAO 속성의 VBO가 바뀌는가?

**답:** 아니다. `glVertexAttribPointer()`가 호출될 때 연결이 기록되며, 단순히 나중에 다른 `GL_ARRAY_BUFFER`를 바인딩한다고 자동 변경되지 않는다.

### Q3. Attribute Index와 Binding Index는 같은 번호여야 하는가?

**답:** 아니다. Attribute Index는 Shader 입력 번호이고, Binding Index는 VBO를 연결하는 슬롯 번호다. 여러 Attribute가 Binding 0 하나를 공유할 수도 있다.

### Q4. Shader가 없으면 VAO/VBO/EBO만으로 화면을 그릴 수 있는가?

**답:** 현대 OpenGL Core Profile에서 프로그래밍 가능한 렌더링은 링크된 적절한 Shader Program이 필요하다. Buffer와 VAO는 데이터·입력 설정을 제공할 뿐, 정점 위치 처리나 Fragment 출력 계산을 정의하지 않는다.

### Q5. Uniform은 VAO에 저장되는가?

**답:** 아니다. Uniform 값은 Shader Program의 상태다. VAO를 바꾸거나 다른 Program을 잠시 사용하더라도 해당 Program의 값은 유지된다. 단, 성공적으로 재링크하면 초기화된다.

### Q6. `glProgramUniformMatrix4fv` 호출 후 자동으로 그 Program으로 렌더링하는가?

**답:** 아니다. Uniform 값만 설정한다. 그리기 대상 Program은 `glUseProgram()`으로 선택해야 한다.

## 6. 새 문제 기록 템플릿

아래 블록을 복사해 사례를 추가한다.

````markdown
## [날짜] 오류/질문 제목

**상태:** 조사 중 / 원인 확인 / 해결 및 검증 완료

### 1. 상황
- 과제 단계(A1~A4):
- 파일 및 함수:
- OpenGL / 운영체제 / 실행 환경:

### 2. 증상과 재현 방법
- 오류 코드 또는 출력:
- 재현 단계:

### 3. 원인 후보 및 근거
- 후보 A:
- 후보 B:

### 4. 수정 내용
```cpp
// 수정 전과 수정 후 코드
```

### 5. 검증 결과
- 재실행 결과:
- 배운 점:
- 관련 커밋/공식 문서:
````

## 7. 참고 문서

- [VAO / VBO / EBO Wiki](../wiki/VAO-VBO-EBO.md)
- [Shader / Uniform Wiki](../wiki/Shader-Uniform.md)
- [Mesh Legacy vs DSA](../examples/Mesh-Legacy-vs-DSA.md)
- [Khronos OpenGL 4 Reference Pages](https://registry.khronos.org/OpenGL-Refpages/gl4/)
