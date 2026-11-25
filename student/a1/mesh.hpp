#pragma once
#include "course.hpp"

using namespace course;
namespace Wiki
{
    // ============================================================================
    // 0. 용어 사전
    // ============================================================================
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
    //
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

    // ============================================================================
    // 1. VAO 생성 및 선택
    // ============================================================================
    // glGenVertexArrays(GLsizei n, GLuint* arrays)
    //   역할: 미사용 VAO 이름(ID) n개를 발급받아 arrays가 가리키는 메모리에 기록한다.
    //   n      = 발급받을 VAO ID 개수.
    //   arrays = 생성된 ID(들)를 받을 GLuint 배열의 시작 주소.
    //   주의: 이 시점은 이름 예약이며, 실제 VAO 객체는 최초 glBindVertexArray 시 생성.
    //   예: glGenVertexArrays(1, &mesh.vao);
    //
    // glBindVertexArray(GLuint array)
    //   역할: 현재 OpenGL Context에서 사용할 VAO를 선택(바인딩)한다.
    //   array = 선택할 VAO ID. 0을 전달하면 VAO 바인딩 해제(Core Profile에서는
    //           VAO 없이 정점 배열 관련 설정/그리기를 할 수 없으므로 주의).
    //   주의: glGenVertexArrays로 발급된 ID를 처음 바인딩할 때 VAO 객체가 생성된다.
    //   예: glBindVertexArray(mesh.vao);
    //
    // glCreateVertexArrays(GLsizei n, GLuint* arrays) [OpenGL 4.5 DSA]
    //   역할: VAO ID n개를 발급하고, 실제 VAO 객체까지 즉시 생성/초기화한다.
    //   n      = 생성할 VAO 개수.
    //   arrays = ID(들)를 받을 GLuint 배열 주소.
    //   주의: VAO를 현재 바인딩하지 않는다. DSA 설정에는 바인딩이 불필요하지만
    //         glDrawElements 등 일반 그리기에는 glBindVertexArray가 여전히 필요.
    //   예: glCreateVertexArrays(1, &mesh.vao);

    // ============================================================================
    // 2. 버퍼 객체 생성 / 바인딩
    // ============================================================================
    // glGenBuffers(GLsizei n, GLuint* buffers)
    //   역할: 버퍼 이름(ID) n개를 예약하여 buffers 배열에 기록한다.
    //   n       = 버퍼 ID 개수.
    //   buffers = ID를 받을 GLuint 배열 주소.
    //   주의: 최초 glBindBuffer로 실제 버퍼 객체가 생성된다.
    //
    // glBindBuffer(GLenum target, GLuint buffer)
    //   역할: 현재 Context의 target 바인딩 지점에 버퍼 ID를 연결한다.
    //   target = GL_ARRAY_BUFFER(일반 정점 버퍼 설정에 사용),
    //            GL_ELEMENT_ARRAY_BUFFER(현재 VAO의 인덱스 버퍼 연결 상태) 등.
    //   buffer = 연결할 버퍼 ID. 0은 해당 타깃의 연결을 해제한다.
    //   주의: GL_ARRAY_BUFFER 바인딩 자체는 VAO 상태가 아니다.
    //         glVertexAttribPointer가 호출될 때 현재 바인딩된 VBO 연결을 VAO에 기록한다.
    //         반면 GL_ELEMENT_ARRAY_BUFFER 바인딩은 현재 VAO 상태에 저장된다.
    //
    // glCreateBuffers(GLsizei n, GLuint* buffers) [OpenGL 4.5 DSA]
    //   역할: 버퍼 ID n개를 발급하면서 버퍼 객체를 즉시 생성/초기화한다.
    //   n       = 버퍼 객체 개수.
    //   buffers = ID를 받을 GLuint 배열 주소.
    //   주의: 생성만 수행한다. 실제 데이터 저장소는 별도 할당이 필요하다.
    //         이 함수 자체는 VBO/EBO 용도를 고정하지 않는다.

    // ============================================================================
    // 3. 버퍼 저장소 할당 / 갱신
    // ============================================================================
    // glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
    //   역할: target에 현재 바인딩된 버퍼의 가변 저장소를 (재)할당하고 초기 데이터를 복사.
    //   target = GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER 등 버퍼 바인딩 대상.
    //   size   = 저장소 크기(Byte), 예: vertices.size()*sizeof(Vertex).
    //   data   = 초기 데이터 CPU 메모리 주소. nullptr이면 초기 내용을 미지정 상태로 둔다.
    //   usage  = 예상 사용 패턴 힌트. GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW 등.
    //   주의: usage는 삼각형/선 같은 그리기 도형 종류를 정하지 않는다.
    //
    // glNamedBufferData(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage)
    //   [OpenGL 4.5 DSA]
    //   역할: 지정한 buffer의 가변 저장소를 (재)할당하고 초기 데이터를 복사한다.
    //   buffer = 대상 버퍼 ID. 버퍼 바인딩 불필요.
    //   size   = 저장소 크기(Byte).
    //   data   = 초기 데이터 CPU 주소(또는 nullptr).
    //   usage  = GL_STATIC_DRAW 등 올바른 사용 패턴 enum. '0'은 유효하지 않다!
    //   예: glNamedBufferData(mesh.vbo, bytes, ptr, GL_STATIC_DRAW);
    //
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
    //
    // glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size,
    //                 const void* data)
    //   역할: 현재 target에 바인딩된 버퍼 저장소의 일부 바이트를 갱신한다.
    //   target = 대상 버퍼 바인딩 지점.
    //   offset = 갱신 시작 바이트 오프셋.
    //   size   = 갱신 바이트 수.
    //   data   = 새 CPU 데이터 주소.
    //   주의: 저장소가 미리 있어야 하고 [offset, offset+size) 구간이 범위 안이어야 한다.
    //
    // glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size,
    //                      const void* data) [OpenGL 4.5 DSA]
    //   역할: 바인딩 없이 지정한 버퍼의 데이터 일부를 갱신한다.
    //   buffer = 대상 버퍼 ID.
    //   offset = 갱신 시작 바이트 오프셋.
    //   size   = 갱신 바이트 수.
    //   data   = 새 CPU 데이터 주소.
    //   주의: 불변 저장소는 GL_DYNAMIC_STORAGE_BIT가 있어야 이 함수로 갱신 가능.

    // ============================================================================
    // 4. 정점 속성 설정: 기존 방식
    // ============================================================================
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
    //
    // glEnableVertexAttribArray(GLuint index)
    //   역할: 현재 바인딩된 VAO의 Attribute[index] 정점 배열을 활성화한다.
    //   index = 활성화할 Attribute 번호.
    //   주의: 형식 설정과 활성화는 별도 작업이다.

    // ============================================================================
    // 5. 정점 속성 설정: DSA 방식
    // ============================================================================
    // glVertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer,
    //                           GLintptr offset, GLsizei stride)
    //   역할: vaobj의 Binding[bindingindex] 슬롯에 VBO, 시작 위치와 stride 설정.
    //   vaobj        = 설정할 VAO ID.
    //   bindingindex = VAO 내부 정점 버퍼 바인딩 슬롯 번호(속성 번호가 아님).
    //   buffer       = 연결할 VBO ID.
    //   offset       = VBO 저장소 기준 시작 바이트 오프셋.
    //   stride       = 연속된 정점 사이의 바이트 간격.
    //   주의: 이 함수만으로 Attribute가 특정 Binding을 참조하지는 않는다.
    //
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
    //
    // glVertexArrayAttribBinding(GLuint vaobj, GLuint attribindex,
    //                            GLuint bindingindex)
    //   역할: Attribute[attribindex] -> Binding[bindingindex] 관계를 기록한다.
    //   vaobj        = 설정할 VAO ID.
    //   attribindex  = 속성 인덱스. '무엇을 읽는가'와 형식을 식별.
    //   bindingindex = VBO 바인딩 슬롯 인덱스. '어디서 읽는가'를 선택.
    //   주의: 여러 Attribute가 같은 Binding을 공유할 수 있다.
    //         Binding 슬롯의 VBO를 바꾸면 이를 참조하는 모든 Attribute가 영향을 받는다.
    //
    // glEnableVertexArrayAttrib(GLuint vaobj, GLuint index)
    //   역할: vaobj의 Attribute[index] 배열을 활성화한다(미리 VAO 바인딩 불필요).
    //   vaobj = 설정할 VAO ID.
    //   index = 활성화할 Attribute 번호.
    //
    // glVertexArrayElementBuffer(GLuint vaobj, GLuint buffer)
    //   역할: vaobj에 EBO를 직접 연결한다.
    //   vaobj  = EBO 연결을 저장할 VAO ID.
    //   buffer = 연결할 EBO ID.
    //   주의: EBO용 별도 Binding Index/Stride는 없다. 인덱스 자료형과 읽기 시작
    //         오프셋은 glDrawElements 호출에서 제공한다.

    // ============================================================================
    // 6. 렌더링 및 자원 해제
    // ============================================================================
    // glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
    //   역할: 현재 VAO의 EBO에서 인덱스를 읽고 정점 속성을 취합해 그리기를 수행.
    //   mode    = 기본 도형 조립 방식. 예: GL_TRIANGLES.
    //   count   = 읽을 인덱스 '원소' 개수(삼각형 개수가 아님).
    //   type    = 인덱스 원소 자료형. 예: GL_UNSIGNED_INT.
    //   indices = EBO가 연결되어 있으면 EBO 내부의 시작 '바이트 오프셋'.
    //             nullptr은 0Byte에서 시작한다는 뜻.
    //   주의: glDrawElements를 위한 VAO는 glBindVertexArray로 현재 바인딩해야 한다.
    //
    // glDeleteVertexArrays(GLsizei n, const GLuint* arrays)
    //   역할: 지정한 VAO 객체를 삭제한다.
    //   n      = 삭제할 VAO ID 개수.
    //   arrays = 삭제할 ID 배열의 주소.
    //
    // glDeleteBuffers(GLsizei n, const GLuint* buffers)
    //   역할: 지정한 버퍼 객체들을 삭제 대상으로 지정한다.
    //   n       = 삭제할 버퍼 ID 개수.
    //   buffers = 삭제할 ID 배열 주소.
    //   주의: OpenGL Context가 살아있는 동안 호출되어야 한다.
    //
    // C++ 보조 코드: mesh.indexCount = GLsizei(indices.size());
    //   역할: drawMesh에 원본 indices 벡터를 넘기지 않고도 그릴 인덱스 개수를 기억.
    //   GLsizei = glDrawElements의 count 매개변수와 호환되는 정수 자료형.
    //
    // 과제에서 쓰인 require(mesh.vao == 0, "...")는 OpenGL API가 아니라
    // course.hpp가 제공하는 검증 유틸리티이다.

    // ============================================================================
    // 7. Shader 객체 및 Shader Program
    // ============================================================================
    //
    // [기본 개념]
    //
    // Shader Object:
    //   특정 Shader 단계의 GLSL 소스 코드 및 컴파일 결과를 관리하는 객체.
    //   예: Vertex Shader, Fragment Shader.
    //
    // Shader Program Object:
    //   Shader 객체들을 Attach한 뒤 Link하여 구성하는 프로그램 객체.
    //   링크된 실행 코드와 Uniform 값 등의 상태를 관리한다.
    //
    // Shader 생성 과정:
    //   1. GLSL 소스 코드 준비
    //   2. glCreateShader()     -> Shader 객체 생성
    //   3. glShaderSource()     -> GLSL 소스 저장
    //   4. glCompileShader()    -> Shader 컴파일
    //   5. glCreateProgram()    -> Program 객체 생성
    //   6. glAttachShader()     -> Shader 객체 첨부
    //   7. glLinkProgram()      -> Program 링크
    //   8. glUseProgram()       -> Program 활성화
    //
    // 주의:
    //   Compile과 Link는 반드시 성공 여부를 확인해야 한다.
    //   Attach는 연결할 Shader를 등록하는 과정이고,
    //   Link는 실제로 Shader들을 연결하고 프로그램을 완성하는 과정이다.

    // ----------------------------------------------------------------------------
    // glCreateShader(GLenum type)
    //
    //   역할: 지정한 종류의 Shader 객체를 생성하고 Shader ID를 반환.
    //
    //   type = 생성할 Shader의 종류.
    //          GL_VERTEX_SHADER   : 정점 처리
    //          GL_FRAGMENT_SHADER : Fragment 색상 등의 계산
    //          GL_GEOMETRY_SHADER : Primitive 처리
    //          GL_COMPUTE_SHADER  : 범용 GPU 계산
    //
    //   반환값:
    //     GLuint 형태의 Shader 객체 ID.
    //     생성에 실패하면 0을 반환한다.
    //
    //   주의:
    //     Shader 객체만 생성하며 GLSL 소스 코드가 자동으로 저장되거나
    //     컴파일되는 것은 아니다.
    //
    // 예시:
    //   GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    //   GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    
    // ----------------------------------------------------------------------------
    // glShaderSource(
    //     GLuint shader,
    //     GLsizei count,
    //     const GLchar* const* string,
    //     const GLint* length
    // )
    //
    //   역할: GLSL 소스 코드를 지정한 Shader 객체에 복사하여 저장.
    //
    //   shader = GLSL 소스를 저장할 Shader 객체 ID.
    //
    //   count  = 전달할 소스 문자열의 개수.
    //            문자열 하나라면 1을 사용한다.
    //
    //   string = 소스 문자열 포인터들을 담은 배열의 시작 주소.
    //            문자열 하나만 전달하더라도 문자열 포인터의 주소가 필요하다.
    //
    //   length = 각 문자열의 길이를 저장한 배열의 주소.
    //            nullptr이면 각 문자열의 null 종료 문자('\0')를 기준으로
    //            길이를 판단한다.
    //
    //   주의:
    //     glShaderSource()는 GLSL 코드를 컴파일하지 않는다.
    //     실제 컴파일은 glCompileShader()에서 수행한다.
    //
    //     std::string을 사용할 경우 c_str()을 통해
    //     const char* 형태의 문자열 포인터를 얻을 수 있다.
    //
    // 예시:
    //   std::string sourceText = readShaderFile("scene.vert");
    //   const char* source = sourceText.c_str();
    //
    //   glShaderSource(vertexShader, 1, &source, nullptr);
    
    
    // ----------------------------------------------------------------------------
    // glCompileShader(GLuint shader)
    //
    //   역할: Shader 객체에 저장된 GLSL 소스 코드를 컴파일.
    //
    //   shader = 컴파일할 Shader 객체 ID.
    //
    //   주의:
    //     함수를 호출했다고 반드시 컴파일에 성공하는 것은 아니다.
    //     glGetShaderiv()로 GL_COMPILE_STATUS를 확인해야 한다.
    //
    // 예시:
    //   glCompileShader(vertexShader);
    
    
    // ----------------------------------------------------------------------------
    // glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
    //
    //   역할: Shader 객체의 특정 상태 정보를 조회.
    //
    //   shader = 상태를 조회할 Shader 객체 ID.
    //
    //   pname  = 조회할 상태의 종류.
    //            GL_COMPILE_STATUS : 컴파일 성공 여부
    //            GL_INFO_LOG_LENGTH: 진단 로그의 길이
    //            GL_SHADER_TYPE    : Shader 종류
    //
    //   params = 조회한 결과를 저장할 변수의 메모리 주소.
    //
    //   주의:
    //     GL_COMPILE_STATUS는 GL_TRUE 또는 GL_FALSE를 반환한다.
    //
    // 예시:
    //   GLint success = GL_FALSE;
    //   glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    //
    //   if (success != GL_TRUE) {
    //       // 컴파일 실패 처리
    //   }
    
    
    // ----------------------------------------------------------------------------
    // glGetShaderInfoLog(
    //     GLuint shader,
    //     GLsizei maxLength,
    //     GLsizei* length,
    //     GLchar* infoLog
    // )
    //
    //   역할: Shader 컴파일 과정에서 생성된 진단 로그를 조회.
    //
    //   shader    = 조회할 Shader 객체 ID.
    //
    //   maxLength = infoLog 버퍼에 저장할 수 있는 최대 문자 수(Byte).
    //
    //   length    = 실제 기록된 문자 수를 저장할 변수의 주소.
    //               필요하지 않으면 nullptr 사용 가능.
    //
    //   infoLog   = 진단 로그 문자열을 저장할 메모리 버퍼.
    //
    //   주의:
    //     컴파일 실패 시 오류 원인을 확인하는 데 사용한다.
    //     컴파일 성공 여부는 GL_COMPILE_STATUS로 판단한다.
    //
    // 예시:
    //   char infoLog[1024] = {};
    //   glGetShaderInfoLog(vertexShader, 1024, nullptr, infoLog);
    
    
    // ----------------------------------------------------------------------------
    // glCreateProgram()
    //
    //   역할: 새로운 Shader Program 객체를 생성하고 ID를 반환.
    //
    //   매개변수: 없음.
    //
    //   반환값:
    //     GLuint 형태의 Shader Program ID.
    //     생성에 실패하면 0을 반환한다.
    //
    //   주의:
    //     빈 Shader Program 객체를 생성하는 함수이다.
    //     Shader를 자동으로 첨부하거나 링크하지 않는다.
    //
    // 예시:
    //   GLuint program = glCreateProgram();
    
    
    // ----------------------------------------------------------------------------
    // glAttachShader(GLuint program, GLuint shader)
    //
    //   역할: Shader 객체를 Shader Program 객체에 첨부.
    //
    //   program = Shader를 첨부할 Shader Program 객체 ID.
    //
    //   shader  = 첨부할 Shader 객체 ID.
    //
    //   주의:
    //     Attach는 Shader 객체를 Program에 등록하는 작업이다.
    //     실제 링크는 glLinkProgram()을 호출해야 수행된다.
    //
    //     하나의 Shader 객체를 여러 Program에 첨부할 수 있다.
    //     일반적인 프로그램에서는 동일 Shader 단계의 객체를
    //     하나의 Program에 중복 첨부할 수 없다.
    //
    // 예시:
    //   glAttachShader(program, vertexShader);
    //   glAttachShader(program, fragmentShader);
    
    
    // ----------------------------------------------------------------------------
    // glLinkProgram(GLuint program)
    //
    //   역할: Program에 첨부된 Shader 객체들을 링크하여
    //         실행 가능한 Shader Program을 구성.
    //
    //   program = 링크할 Shader Program 객체 ID.
    //
    //   주요 작업:
    //     - Shader 단계 간 입력/출력 인터페이스 검사
    //     - Attribute 및 Uniform 등의 리소스 정보 결정
    //     - Shader 단계들을 연결하여 실행 준비
    //
    //   주의:
    //     Link에 실패하면 해당 프로그램을 정상적으로 사용할 수 없다.
    //     GL_LINK_STATUS로 성공 여부를 확인해야 한다.
    //
    //     성공적으로 다시 링크하면 일반 Uniform 값이 초기화되고,
    //     Uniform Location도 변경될 수 있다.
    //
    // 예시:
    //   glLinkProgram(program);
    
    
    // ----------------------------------------------------------------------------
    // glGetProgramiv(GLuint program, GLenum pname, GLint* params)
    //
    //   역할: Shader Program 객체의 상태 정보를 조회.
    //
    //   program = 상태를 조회할 Shader Program ID.
    //
    //   pname   = 조회할 상태의 종류.
    //             GL_LINK_STATUS      : 링크 성공 여부
    //             GL_INFO_LOG_LENGTH  : 진단 로그 길이
    //             GL_ATTACHED_SHADERS : 첨부된 Shader 개수
    //
    //   params  = 조회한 결과를 저장할 변수의 주소.
    //
    // 예시:
    //   GLint success = GL_FALSE;
    //   glGetProgramiv(program, GL_LINK_STATUS, &success);
    
    
    // ----------------------------------------------------------------------------
    // glGetProgramInfoLog(
    //     GLuint program,
    //     GLsizei maxLength,
    //     GLsizei* length,
    //     GLchar* infoLog
    // )
    //
    //   역할: Shader Program의 링크 관련 진단 로그를 조회.
    //
    //   program   = 조회할 Shader Program ID.
    //
    //   maxLength = 로그를 저장할 버퍼의 최대 크기(Byte).
    //
    //   length    = 실제 기록된 문자 수를 저장할 변수의 주소.
    //               불필요하면 nullptr 가능.
    //
    //   infoLog   = 로그를 저장할 메모리 버퍼.
    //
    // 예시:
    //   char infoLog[1024] = {};
    //   glGetProgramInfoLog(program, 1024, nullptr, infoLog);
    
    
    // ----------------------------------------------------------------------------
    // glUseProgram(GLuint program)
    //
    //   역할: 현재 OpenGL Context에서 사용할 Shader Program을 선택.
    //
    //   program = 활성화할 Shader Program ID.
    //             0을 전달하면 현재 Program 사용을 해제한다.
    //
    //   주의:
    //     성공적으로 링크된 Program을 사용해야 한다.
    //     glUseProgram()은 Program 객체를 삭제하지 않는다.
    //
    //     다른 Program을 활성화하더라도 이전 Program의 Uniform 값은 유지된다.
    //     실제 Shader 실행은 이후 Draw Call 등에서 이루어진다.
    //
    // 예시:
    //   glUseProgram(program);
    
    
    // ----------------------------------------------------------------------------
    // glDetachShader(GLuint program, GLuint shader)
    //
    //   역할: Program에 첨부된 Shader 객체를 분리.
    //
    //   program = 대상 Shader Program ID.
    //   shader  = 분리할 Shader 객체 ID.
    //
    //   주의:
    //     링크에 성공한 Program에서는 Shader를 분리해도
    //     이미 링크된 실행 코드를 계속 사용할 수 있다.
    //
    // 예시:
    //   glDetachShader(program, vertexShader);
    
    
    // ----------------------------------------------------------------------------
    // glDeleteShader(GLuint shader)
    //
    //   역할: Shader 객체를 삭제 대상으로 지정.
    //
    //   shader = 삭제할 Shader 객체 ID.
    //
    //   주의:
    //     Shader가 Program에 첨부되어 있다면 실제 삭제는
    //     해당 Shader가 모든 Program에서 분리된 이후 이루어진다.
    //
    // 예시:
    //   glDeleteShader(vertexShader);
    
    
    // ----------------------------------------------------------------------------
    // glDeleteProgram(GLuint program)
    //
    //   역할: Shader Program 객체를 삭제 대상으로 지정.
    //
    //   program = 삭제할 Shader Program ID.
    //
    //   주의:
    //     현재 사용 중인 Program은 사용이 해제될 때 실제 삭제된다.
    //     Program 삭제는 Buffer나 VAO 삭제와 별개의 작업이다.
    //
    // 예시:
    //   glDeleteProgram(program);

    // ============================================================================
    // 8. Uniform
    // ============================================================================
    //
    // [기본 개념]
    //
    // Uniform:
    //   GLSL에서 선언하는 프로그램 공통 입력 변수.
    //
    //   CPU 측 OpenGL 코드에서 값을 설정하고,
    //   Shader가 실행될 때 해당 값을 읽어서 계산에 사용한다.
    //
    // 예시 (GLSL):
    //   uniform mat4 uMVP;
    //   uniform vec4 uColor;
    //   uniform float uTime;
    //
    // 특징:
    //   - Vertex Attribute와 달리 정점마다 자동으로 값이 바뀌지 않는다.
    //   - 동일 Draw Call 동안 모든 Shader 실행에서 같은 값을 사용한다.
    //   - Uniform 값은 Shader Program 객체에 저장된다.
    //   - Program을 비활성화해도 Uniform 값이 유지된다.
    //   - 성공적으로 다시 링크하면 일반 Uniform 값이 초기화된다.
    //
    // Uniform 설정 과정:
    //   1. Shader Program 링크
    //   2. glGetUniformLocation()으로 Uniform 위치 조회
    //   3. glUseProgram()으로 Program 활성화
    //   4. glUniform*()으로 Uniform 값 설정
    //
    // DSA 스타일에서는 glProgramUniform*()을 이용하여
    // Program을 활성화하지 않고도 특정 Program의 값을 설정할 수 있다.
    
    
    // ----------------------------------------------------------------------------
    // glGetUniformLocation(GLuint program, const GLchar* name)
    //
    //   역할: 지정한 Shader Program 내부에서 Uniform Location 조회.
    //
    //   program = Uniform을 조회할 Shader Program 객체 ID.
    //
    //   name    = Uniform 변수의 이름.
    //             GLSL에 선언한 변수 이름과 일치해야 한다.
    //
    //   반환값:
    //     GLint 형태의 Uniform Location.
    //
    //     -1 : 해당 이름의 활성 Uniform이 존재하지 않거나
    //          최적화 등에 의해 제거되어 위치를 조회할 수 없는 경우.
    //
    //   주의:
    //     Uniform Location은 Program마다 독립적으로 관리된다.
    //     서로 다른 Program에서 같은 이름을 사용해도 Location은 다를 수 있다.
    //
    //     Program을 다시 링크하면 Location을 다시 조회해야 한다.
    //
    //     Location은 CPU 메모리 주소나 GPU 메모리 주소가 아니라
    //     Program 내부의 Uniform을 식별하는 번호이다.
    //
    // 예시:
    //   GLint location = glGetUniformLocation(program, "uMVP");
    
    
    // ----------------------------------------------------------------------------
    // glUniform1f(GLint location, GLfloat v0)
    //
    //   역할: 현재 활성화된 Program의 float Uniform에 값 하나 설정.
    //
    //   location = Uniform Location.
    //   v0       = 전달할 float 값.
    //
    // 예시 (GLSL):
    //   uniform float uTime;
    //
    // 예시 (C++):
    //   glUniform1f(location, 1.5f);
    
    
    // ----------------------------------------------------------------------------
    // glUniform2f(GLint location, GLfloat v0, GLfloat v1)
    //
    //   역할: 현재 활성화된 Program의 vec2 Uniform 값 설정.
    //
    //   location = Uniform Location.
    //   v0       = 첫 번째 float 성분.
    //   v1       = 두 번째 float 성분.
    //
    // 예시 (GLSL):
    //   uniform vec2 uPosition;
    //
    // 예시 (C++):
    //   glUniform2f(location, 0.5f, 0.25f);
    
    
    // ----------------------------------------------------------------------------
    // glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
    //
    //   역할: 현재 활성화된 Program의 vec3 Uniform 값 설정.
    //
    //   location = Uniform Location.
    //   v0       = 첫 번째 float 성분.
    //   v1       = 두 번째 float 성분.
    //   v2       = 세 번째 float 성분.
    //
    // 예시 (GLSL):
    //   uniform vec3 uColor;
    //
    // 예시 (C++):
    //   glUniform3f(location, 1.0f, 0.0f, 0.0f);
    
    
    // ----------------------------------------------------------------------------
    // glUniform4f(
    //     GLint location,
    //     GLfloat v0,
    //     GLfloat v1,
    //     GLfloat v2,
    //     GLfloat v3
    // )
    //
    //   역할: 현재 활성화된 Program의 vec4 Uniform 값 설정.
    //
    //   location = Uniform Location.
    //   v0       = 첫 번째 float 성분.
    //   v1       = 두 번째 float 성분.
    //   v2       = 세 번째 float 성분.
    //   v3       = 네 번째 float 성분.
    //
    //   주의:
    //     glUniform4f()는 float 성분 네 개를 전달하는 함수다.
    //     mat4 Uniform을 설정하는 함수가 아니다.
    //
    // 예시 (GLSL):
    //   uniform vec4 uColor;
    //
    // 예시 (C++):
    //   glUniform4f(location, 1.0f, 0.0f, 0.0f, 1.0f);
    
    
    // ----------------------------------------------------------------------------
    // glUniform1i(GLint location, GLint v0)
    //
    //   역할: 현재 활성화된 Program의 int Uniform 값 설정.
    //
    //   location = Uniform Location.
    //   v0       = 전달할 정수 값.
    //
    //   주의:
    //     bool, sampler 계열 Uniform을 설정할 때도 사용한다.
    //
    // 예시 (GLSL):
    //   uniform int uMode;
    //
    // 예시 (C++):
    //   glUniform1i(location, 2);
    
    
    // ----------------------------------------------------------------------------
    // glUniformMatrix4fv(
    //     GLint location,
    //     GLsizei count,
    //     GLboolean transpose,
    //     const GLfloat* value
    // )
    //
    //   역할: 현재 활성화된 Shader Program의 mat4 Uniform에
    //         4x4 float 행렬 데이터를 전달.
    //
    //   location  = Uniform Location.
    //
    //   count     = 전달할 4x4 행렬의 개수.
    //               mat4 하나를 전달할 경우 1.
    //
    //   transpose = 행렬 전치 여부.
    //               GL_FALSE : 전치하지 않음.
    //               GL_TRUE  : 전치하여 전달.
    //
    //   value     = 행렬 데이터가 저장된 메모리의 시작 주소.
    //               float 배열 또는 glm::value_ptr() 등을 사용한다.
    //
    //   주의:
    //     GLSL의 mat4 Uniform에 사용할 수 있다.
    //     GL_FALSE는 정규화 여부가 아니라 전치 여부를 의미한다.
    //
    //     GLM의 일반적인 행렬 메모리 배치에서는 GL_FALSE를 사용한다.
    //
    // 예시 (GLSL):
    //   uniform mat4 uMVP;
    //
    // 예시 (C++):
    //   glUniformMatrix4fv(
    //       location,
    //       1,
    //       GL_FALSE,
    //       glm::value_ptr(MVP)
    //   );
    
    
    // ----------------------------------------------------------------------------
    // glProgramUniformMatrix4fv(
    //     GLuint program,
    //     GLint location,
    //     GLsizei count,
    //     GLboolean transpose,
    //     const GLfloat* value
    // )
    //
    //   역할: 지정한 Shader Program의 mat4 Uniform 값을 직접 설정.
    //
    //   program   = Uniform을 설정할 Shader Program ID.
    //
    //   location  = 해당 Program에서 조회한 Uniform Location.
    //
    //   count     = 전달할 행렬 개수.
    //
    //   transpose = 행렬 전치 여부.
    //
    //   value     = 행렬 데이터가 저장된 메모리의 시작 주소.
    //
    //   특징:
    //     - glUseProgram()을 호출하지 않아도 Uniform을 설정할 수 있다.
    //     - 현재 활성화된 Shader Program은 변경되지 않는다.
    //     - 지정한 Program의 Uniform 상태만 변경한다.
    //
    //   주의:
    //     location은 반드시 해당 Program에서 조회한 값을 사용해야 한다.
    //     Program은 성공적으로 링크된 상태여야 한다.
    //
    // 예시:
    //   GLint locB = glGetUniformLocation(programB, "uMVP");
    //
    //   glProgramUniformMatrix4fv(
    //       programB,
    //       locB,
    //       1,
    //       GL_FALSE,
    //       glm::value_ptr(MVP)
    //   );
    
    
    // ----------------------------------------------------------------------------
    // glProgramUniform4f(
    //     GLuint program,
    //     GLint location,
    //     GLfloat v0,
    //     GLfloat v1,
    //     GLfloat v2,
    //     GLfloat v3
    // )
    //
    //   역할: 지정한 Shader Program의 vec4 Uniform 값을 직접 설정.
    //
    //   program  = Uniform 값을 설정할 Program ID.
    //   location = 해당 Program의 Uniform Location.
    //   v0       = 첫 번째 float 성분.
    //   v1       = 두 번째 float 성분.
    //   v2       = 세 번째 float 성분.
    //   v3       = 네 번째 float 성분.
    //
    // 예시:
    //   glProgramUniform4f(
    //       program,
    //       location,
    //       1.0f, 0.0f, 0.0f, 1.0f
    //   );
}

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
namespace FunctionExample
{
    // ============================================================================
    // VAO, VBO, EBO
    // ============================================================================

    // ============================================================================
    // A. 기존 방식 — 참고 예제 (기존 glGen*/glBind*/glVertexAttribPointer)
    // ============================================================================
    inline void createMeshLegacy(Mesh& mesh, const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices)
    {
        require(mesh.vao == 0, "Create each mesh only once");
        if (vertices.empty() || indices.empty()) return;

        // 1. VAO 생성 및 바인딩
        glGenVertexArrays(1, &mesh.vao);
        glBindVertexArray(mesh.vao);

        // 2. VBO 객체 생성 + 3. 저장소 할당
        glGenBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        // 4. Attribute 0 (위치), Attribute 1 (색상) 설정 + 활성화
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, x)));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, r)));
        glEnableVertexAttribArray(1);

        // 5. EBO 생성 + 저장소 할당 (현재 VAO에 EBO 연결도 저장)
        glGenBuffers(1, &mesh.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

        mesh.indexCount = GLsizei(indices.size());
    }

    // ============================================================================
    // B. DSA 방식 — 과제 A1.3~A1.5 예제 (OpenGL 4.5+)
    // ============================================================================
    inline void createMeshDSA(Mesh& mesh, const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices)
    {
        require(mesh.vao == 0, "Create each mesh only once");
        if (vertices.empty() || indices.empty()) return;

        // 1. VAO 생성 (바인딩 불필요)
        glCreateVertexArrays(1, &mesh.vao);

        // 2. VBO/EBO 생성 + 3. 불변 저장소 초기 데이터 복사
        glCreateBuffers(1, &mesh.vbo);
        glNamedBufferStorage(mesh.vbo, vertices.size() * sizeof(Vertex), vertices.data(), 0);

        glCreateBuffers(1, &mesh.ebo);
        glNamedBufferStorage(mesh.ebo, indices.size() * sizeof(unsigned), indices.data(), 0);

        // 4. Binding[0]에 VBO 연결 + VAO에 EBO 연결
        glVertexArrayVertexBuffer(mesh.vao, 0, mesh.vbo, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(mesh.vao, mesh.ebo);

        // 5. Attribute[0] = 위치, Binding[0] 사용
        glVertexArrayAttribFormat(mesh.vao, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, x));
        glVertexArrayAttribBinding(mesh.vao, 0, 0);
        glEnableVertexArrayAttrib(mesh.vao, 0);

        // 6. Attribute[1] = 색상, Binding[0] 사용
        glVertexArrayAttribFormat(mesh.vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, r));
        glVertexArrayAttribBinding(mesh.vao, 1, 0);
        glEnableVertexArrayAttrib(mesh.vao, 1);

        mesh.indexCount = GLsizei(indices.size());
    }

    // ============================================================================
    // C. 기존과 DSA에서 공통인 그리기 과정
    // ============================================================================
    inline void drawMeshExample(const Mesh& mesh)
    {
        if (!mesh.vao) return;
        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
    }

    // ============================================================================
    // D. 기존 방식 <-> DSA 매칭 요약
    // ============================================================================
    // VAO 생성      : glGenVertexArrays + 최초 glBindVertexArray <-> glCreateVertexArrays
    // Buffer 생성   : glGenBuffers + 최초 glBindBuffer <-> glCreateBuffers
    // 가변 저장소   : glBufferData(target,...,usage) <-> glNamedBufferData(buffer,...,usage)
    // 불변 저장소   : glBufferStorage(target,...,flags) <-> glNamedBufferStorage(buffer,...,flags)
    // VBO 속성 연결 : glBindBuffer(GL_ARRAY_BUFFER) + glVertexAttribPointer
    //                 <-> glVertexArrayVertexBuffer + glVertexArrayAttribFormat + glVertexArrayAttribBinding
    // 속성 활성화   : glEnableVertexAttribArray <-> glEnableVertexArrayAttrib
    // EBO 속성 연결 : glBindVertexArray + glBindBuffer(GL_ELEMENT_ARRAY_BUFFER)
    //                 <-> glVertexArrayElementBuffer
    // 부분 갱신     : glBufferSubData <-> glNamedBufferSubData
    // 실제 그리기   : 양쪽 동일, glBindVertexArray + glDrawElements
    // 자원 해제     : 양쪽 동일, glDeleteVertexArrays + glDeleteBuffers

    // ============================================================================
    // Shader / Uniform
    // ============================================================================
    
    // ============================================================================
    // A. Shader 파일 읽기 — 기존 방식과 DSA 공통
    // ============================================================================
    inline std::string readShaderFile(const std::string& path)
    {
        std::ifstream file(path);

        if (!file.is_open())
            throw std::runtime_error("Failed to open shader file: " + path);

        std::stringstream buffer;
        buffer << file.rdbuf();

        return buffer.str();
    }

    // ============================================================================
    // B. Shader 객체 생성 및 컴파일 — 기존 방식과 DSA 공통
    // ============================================================================
    inline GLuint compileShader(GLenum type, const std::string& source)
    {
        // 1. Shader 객체 생성
        GLuint shader = glCreateShader(type);

        // 2. GLSL 소스 코드 저장
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);

        // 3. Shader 컴파일
        glCompileShader(shader);

        // 4. 컴파일 성공 여부 확인
        GLint success = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (success != GL_TRUE)
        {
            // 5. 컴파일 실패 로그 조회
            GLint logLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

            std::string infoLog(logLength > 0 ? logLength : 1, '\0');
            glGetShaderInfoLog(
                shader,
                static_cast<GLsizei>(infoLog.size()),
                nullptr,
                infoLog.data()
            );

            glDeleteShader(shader);

            throw std::runtime_error("Shader compilation failed: " + infoLog);
        }

        return shader;
    }

    // ============================================================================
    // C. Shader Program 생성 및 링크 — 기존 방식과 DSA 공통
    // ============================================================================
    inline GLuint createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath)
    {
        // 1. GLSL 소스 파일 읽기
        std::string vertexSource = readShaderFile(vertexPath);
        std::string fragmentSource = readShaderFile(fragmentPath);

        // 2. Vertex Shader 및 Fragment Shader 컴파일
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
        GLuint fragmentShader = 0;

        try
        {
            fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
        }
        catch (...)
        {
            glDeleteShader(vertexShader);
            throw;
        }

        // 3. Shader Program 객체 생성
        GLuint program = glCreateProgram();

        // 4. 컴파일된 Shader 객체들을 Program에 첨부
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        // 5. Shader Program 링크
        glLinkProgram(program);

        // 6. 링크 성공 여부 확인
        GLint success = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &success);

        if (success != GL_TRUE)
        {
            GLint logLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

            std::string infoLog(logLength > 0 ? logLength : 1, '\0');
            glGetProgramInfoLog(
                program,
                static_cast<GLsizei>(infoLog.size()),
                nullptr,
                infoLog.data()
            );

            glDeleteProgram(program);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            throw std::runtime_error("Program linking failed: " + infoLog);
        }

        // 7. 링크 완료 후 Shader 객체 분리
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);

        // 8. Shader 객체 삭제 (링크된 Program은 유지)
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    // ============================================================================
    // D. 기존 방식 — Uniform 설정 (현재 활성화된 Program 사용)
    // ============================================================================
    inline void setUniformLegacy(GLuint program, const glm::mat4& MVP, const glm::vec4& color)
    {
        // 1. Shader Program 활성화
        glUseProgram(program);

        // 2. Uniform Location 조회
        GLint mvpLocation = glGetUniformLocation(program, "uMVP");
        GLint colorLocation = glGetUniformLocation(program, "uColor");

        // 3. MVP 행렬 Uniform 설정
        if (mvpLocation != -1)
        {
            glUniformMatrix4fv(
                mvpLocation,
                1,
                GL_FALSE,
                glm::value_ptr(MVP)
            );
        }

        // 4. 색상 Uniform 설정
        if (colorLocation != -1)
        {
            glUniform4f(
                colorLocation,
                color.r,
                color.g,
                color.b,
                color.a
            );
        }
    }

    // ============================================================================
    // E. DSA 스타일 — Uniform 설정 (Program 직접 지정)
    // ============================================================================
    inline void setUniformDSA(GLuint program, const glm::mat4& MVP, const glm::vec4& color)
    {
        // 1. Uniform Location 조회
        GLint mvpLocation = glGetUniformLocation(program, "uMVP");
        GLint colorLocation = glGetUniformLocation(program, "uColor");

        // 2. MVP 행렬 Uniform 설정 (Program 활성화 불필요)
        if (mvpLocation != -1)
        {
            glProgramUniformMatrix4fv(
                program,
                mvpLocation,
                1,
                GL_FALSE,
                glm::value_ptr(MVP)
            );
        }

        // 3. 색상 Uniform 설정 (Program 활성화 불필요)
        if (colorLocation != -1)
        {
            glProgramUniform4f(
                program,
                colorLocation,
                color.r,
                color.g,
                color.b,
                color.a
            );
        }
    }

    // ============================================================================
    // F. 기존 방식과 DSA 스타일에서 공통인 렌더링 과정
    // ============================================================================
    inline void drawWithProgram(GLuint program, const Mesh& mesh)
    {
        if (!mesh.vao) return;

        // 1. Shader Program 활성화
        glUseProgram(program);

        // 2. VAO 바인딩
        glBindVertexArray(mesh.vao);

        // 3. 인덱스 기반 렌더링
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
    }

    // ============================================================================
    // G. Shader Program 자원 해제
    // ============================================================================
    inline void deleteShaderProgram(GLuint program)
    {
        glDeleteProgram(program);
    }

    // ============================================================================
    // H. 기존 방식 <-> DSA 스타일 매칭 요약
    // ============================================================================
    //
    // Shader 생성    : 양쪽 동일, glCreateShader
    // GLSL 소스 저장 : 양쪽 동일, glShaderSource
    // Shader 컴파일  : 양쪽 동일, glCompileShader
    // 컴파일 확인    : 양쪽 동일, glGetShaderiv + glGetShaderInfoLog
    // Program 생성   : 양쪽 동일, glCreateProgram
    // Shader 첨부    : 양쪽 동일, glAttachShader
    // Program 링크   : 양쪽 동일, glLinkProgram
    // 링크 확인      : 양쪽 동일, glGetProgramiv + glGetProgramInfoLog
    // Uniform 조회   : 양쪽 동일, glGetUniformLocation
    // MVP 행렬 설정  : glUseProgram + glUniformMatrix4fv <-> glProgramUniformMatrix4fv
    // vec4 값 설정   : glUseProgram + glUniform4f <-> glProgramUniform4f
    // float 값 설정  : glUseProgram + glUniform1f <-> glProgramUniform1f
    // int 값 설정    : glUseProgram + glUniform1i <-> glProgramUniform1i
    // 실제 렌더링    : 양쪽 동일, glUseProgram + glBindVertexArray + glDrawElements
    // Shader 분리    : 양쪽 동일, glDetachShader
    // Shader 삭제    : 양쪽 동일, glDeleteShader
    // Program 삭제   : 양쪽 동일, glDeleteProgram
    //
    // ============================================================================
    // 핵심 개념
    // ============================================================================
    //
    // 1. Shader Object
    //    GLSL 소스 코드와 컴파일 결과를 관리하는 객체.
    //
    // 2. Shader Program Object
    //    여러 Shader 단계를 링크한 결과와 Uniform 상태를 관리하는 객체.
    //
    // 3. glUseProgram()
    //    현재 OpenGL Context에서 사용할 Shader Program을 선택하는 함수.
    //
    // 4. glGetUniformLocation()
    //    지정한 Shader Program 안에서 Uniform의 위치를 조회하는 함수.
    //
    // 5. glUniform*()
    //    현재 활성화된 Shader Program의 Uniform 값을 변경하는 함수.
    //
    // 6. glProgramUniform*()
    //    Program ID를 직접 지정하여 Uniform 값을 변경하는 함수.
    //    현재 활성화된 Program에는 영향을 주지 않는다.
    //
    // 7. Uniform 값은 Program 객체에 저장된다.
    //    Program을 변경하더라도 기존 값은 유지된다.
    //    단, 성공적으로 다시 링크하면 일반 Uniform 값이 초기화된다.
    //
    // 8. Uniform Location은 Program마다 독립적으로 관리된다.
    //    동일한 이름이라도 Program마다 Location이 다를 수 있다.
    //
    // 9. glProgramUniform*()을 사용하더라도,
    //    실제 렌더링할 때는 glUseProgram()으로 프로그램을 선택해야 한다.
}
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