#version 450 core

layout(location=0) in vec2 aPosition;
layout(location=1) in vec3 aColor;

uniform mat3 uModel;

out vec3 vColor;

void main()
{
    // TODO A2.6: apply model transform to homogeneous local position.
    // Hint: Use GLSL vec3(aPosition, 1.0) and mat3 * vec3. A final component of 0 would discard translation.
    vec3 position = uModel * vec3(aPosition, 1.0);

    gl_Position=vec4(position.xy, 0.0, 1.0); // 동차좌표의 의미 유지 vec4(position, 1.0)을 해버리면 [x,y,1]에서의 w=1이 z값으로 바뀌어 [x,y,1,1]이 되어버린다.
    vColor=aColor;
}
