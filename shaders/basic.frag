#version 330 core
// 버텍스 쉐이더에서 보낸 위치 데이터를 받습니다.
in vec3 FragPos; 

out vec4 FragColor;

void main() {
    // 1. 격자 선의 간격과 두께 설정
    float gridSize = 1.0; 
    float thickness = 0.05; // 0.05 정도면 적당한 두께의 선이 나옵니다.

    // 2. 현재 좌표를 간격으로 나눈 나머지(0.0 ~ 1.0)를 구함
    // fract()는 소수점 이하만 남기는 함수입니다.
    float xPos = fract(FragPos.x / gridSize);
    float zPos = fract(FragPos.z / gridSize);

    // 3. X나 Z 좌표의 소수점이 두께보다 작으면 선을 그리고, 아니면 바닥을 칠함
    if(xPos < thickness || zPos < thickness) {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0); // 하얀색 선
    } else {
        FragColor = vec4(1.0, 0.5, 0.2, 1.0); // 주황색 바닥
    }
}