#version 330 core

// 정점에서 받는 입력 (3가지)
layout (location = 0) in vec3 aPos;        // 위치
layout (location = 1) in vec3 aNormal;     // 노멀
layout (location = 2) in vec2 aTexCoord;   // UV

// Fragment Shader로 넘길 값
out vec3 FragPos;     // 월드 공간에서의 픽셀 위치
out vec3 Normal;      // 월드 공간에서의 노멀
out vec2 TexCoord;    // 텍스처 좌표

// CPU에서 받는 변환 행렬
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // 최종 화면 좌표 계산
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // 조명 계산용: 월드 공간에서의 픽셀 위치
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // 노멀도 월드 공간으로 변환 (비균등 스케일 대응)
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // UV는 그대로 전달
    TexCoord = aTexCoord;
}
