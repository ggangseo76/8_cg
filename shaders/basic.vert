#version 330 core
layout (location = 0) in vec3 aPos;

// 프래그먼트 쉐이더로 위치 데이터를 보내기 위한 변수 선언
out vec3 FragPos; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // 바닥의 원래(Local) x, y, z 좌표를 그대로 넘겨줍니다.
    FragPos = aPos; 
}