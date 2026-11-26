# OpenGL Graphics Study

OpenGL을 이용하여 컴퓨터 그래픽스의 기본 원리와 렌더링 파이프라인을 학습하고, C++로 직접 구현한 내용을 기록하는 학습용 리포지토리입니다.

단순히 코드를 구현하는 것뿐만 아니라, **OpenGL API의 동작 원리, Legacy 방식과 DSA 방식의 차이, 학습 과정에서 발생한 의문점과 해결 방법**을 기록하는 것을 목표로 합니다.

## Assignment Guide

본 프로젝트는 총 4단계(A1~A4)의 2D OpenGL 과제를 기반으로 학습을 진행합니다.

과제별 요구사항, 진행 방법, 빌드 및 실행 방법 등은 아래 문서를 참고할 수 있습니다.

[**2D OpenGL Assignment Guide**](ASSIGNMENT_GUIDE.md)

## 학습 목표

- OpenGL 렌더링 파이프라인 이해
    
- VAO, VBO, EBO 및 정점 데이터 관리
    
- Vertex Shader와 Fragment Shader 이해
    
- Shader Program과 Uniform 관리
    
- Legacy OpenGL과 Direct State Access(DSA) 비교
    
- OpenGL 디버깅 및 문제 해결 능력 향상
    

## Study Notes

### Wiki — OpenGL 이론 및 API 정리

OpenGL API의 역할, 매개변수, 주의사항과 내부 동작 원리를 정리합니다.

|주제|내용|
|---|---|
|[VAO / VBO / EBO](docs/wiki/VAO-VBO-EBO.md)|정점 데이터 관리와 버퍼 연결|
|[Shader / Uniform](docs/wiki/Shader-Uniform.md)|Shader 컴파일, Program 링크, Uniform 관리|

### Examples — Legacy vs DSA

동일한 기능을 기존 OpenGL 방식과 DSA 방식으로 각각 구현하고 비교합니다.

|주제|내용|
|---|---|
|[Mesh 생성](docs/examples/Mesh-Legacy-vs-DSA.md)|glGen/glBind 방식과 DSA 방식 비교|
|[Shader / Uniform](docs/examples/Shader-Uniform-Legacy-vs-DSA.md)|Shader 구성 및 Uniform 설정 방식 비교|

### Troubleshooting — 오류 및 해결 기록

학습 중 발생한 문제와 원인 분석, 해결 과정을 정리합니다.

[OpenGL 오류 및 해결 기록](docs/troubleshooting/OpenGL-Errors.md)

기록 형식:

- 문제 상황 및 오류 메시지
    
- 문제가 발생한 코드
    
- 원인 분석
    
- 해결 방법
    
- 새롭게 알게 된 내용
    

## 학습 진행 상황

- OpenGL 기본 개념 및 렌더링 파이프라인 학습
    
- VAO / VBO / EBO 기본 개념 학습
    
- Legacy 및 DSA 방식 비교
    
- Shader 및 Uniform 기본 개념 학습
    
- Texture 및 Sampler 학습
    
- Framebuffer 학습
    

## Development Environment

- Language: C++
    
- Graphics API: OpenGL
    
- Build System: CMake
    
- Graphics Debugging: RenderDoc
    

## 참고 사항

이 리포지토리는 OpenGL을 학습하며 작성한 개인 학습 기록입니다.

일부 설명과 예제는 학습 과정에 따라 수정될 수 있으며, OpenGL 버전과 플랫폼에 따라 지원되는 기능이 다를 수 있습니다.