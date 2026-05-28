#ifndef STAIRCASE_H
#define STAIRCASE_H

#include <glad/glad.h>
#include "../Dependencies/include/glm/glm.hpp"
#include "shader.h"

// ==========================================
// Staircase 클래스 (계단)
// ------------------------------------------
// Room과 같은 철학:
//   - 기본 도형 하나(여기선 1x1x1 단위 큐브)를 만들어 두고
//   - model 행렬로 위치/크기를 바꿔가며 여러 번 그린다
// Room은 평면(quad)을 6번, Staircase는 큐브를 '단 개수'만큼 그린다.
//
// X축을 따라 한 단씩 올라가는 계단:
//   - 단마다 y(높이)가 stepHeight 씩 증가
//   - 단마다 x(가로 위치)가 stepDepth 만큼 이동
// ==========================================
class Staircase {
public:
    // 생성자: 계단의 세 가지 핵심 치수를 받는다
    //   stepHeight : 한 단의 높이(y 격차)    예) 0.5
    //   stepDepth  : 한 단의 깊이(X축 폭)     예) 2.0
    //   stepCount  : 단 개수                  예) 6
    //   width      : 계단의 가로 너비(Z축 폭) 예) 8.0  (방 depth=12 안에 들어가게)
    Staircase(float stepHeight, float stepDepth, int stepCount, float width);
    ~Staircase();

    // 매 프레임 호출 - 모든 단을 그린다 (Room::Draw 와 같은 역할)
    void Draw(Shader& shader);

    // ----- 카메라가 물어보는 함수 (높이 보정의 핵심) -----
    // 어떤 (x, z) 위치가 주어지면, 그 자리의 '바닥 높이'를 돌려준다.
    //   - 계단 위에 있으면 그 단의 윗면 높이
    //   - 계단 밖이면 0 (방 바닥)
    // 카메라는 이 값에 눈높이를 더해서 자기 y를 맞춘다.
    float GetFloorHeightAt(float x, float z) const;

private:
    // 계단의 치수들 (생성자에서 저장 → GetFloorHeightAt 에서 다시 사용)
    float stepHeight;
    float stepDepth;
    int   stepCount;
    float width;

    // 계단이 X축에서 시작하는 위치 (맨 아래 단의 왼쪽 끝)
    // 계단 전체를 방 중앙 근처에 놓기 위해 생성자에서 계산한다.
    float startX;

    // 한 단을 표현하는 GPU 리소스 + 변환 행렬
    // (Room의 Face 구조체와 같은 발상)
    struct Step {
        glm::mat4 model;   // 단위 큐브를 이 단의 위치/크기로 보내는 변환
    };

    // 단들을 담는 배열 (개수는 생성자에서 정해짐)
    // 고정 크기 대신 동적 배열을 쓰려고 포인터로 둔다.
    Step* steps;

    // 모든 단이 공유하는 큐브 한 벌의 GPU 리소스
    unsigned int VAO;
    unsigned int VBO;
    unsigned int texture;

    // 헬퍼 함수
    void setupCube();                       // 큐브 VAO/VBO 생성
    unsigned int loadTexture(const char* path);
};

#endif