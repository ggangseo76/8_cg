#ifndef STAIRCASE_H
#define STAIRCASE_H

#include <glad/glad.h>
#include "../Dependencies/include/glm/glm.hpp"
#include "shader.h"

// ==========================================
// Staircase 클래스 (전시관 단차 구조: 안내방 + 계단 + 메인방)
// ------------------------------------------
// Room과 같은 철학:
//   - 기본 도형 하나(1x1x1 단위 큐브)를 만들어 두고
//   - model 행렬로 위치/크기를 바꿔가며 여러 번 그린다
//
// 이번 구조 (--\__ 단면, 왼쪽 높고 오른쪽으로 내려감):
//   - 안내방 B : 왼쪽 넓은 평지 (높음)
//   - 계단      : 가운데 짧은 계단 (내려감)
//   - 메인방 A : 오른쪽은 방 바닥(y=0) 그대로 → 박스 안 만듦
//
// 평지든 계단이든 결국 "x범위 + 윗면 높이"를 가진 박스 하나일 뿐이라,
// 박스 목록(Block 배열)으로 통일해서 관리한다.
// ==========================================
class Staircase {
public:
    Staircase();
    ~Staircase();

    // 매 프레임 호출 - 모든 블록(평지/계단)을 그린다
    void Draw(Shader& shader);

    // ----- 카메라가 물어보는 함수 (높이 보정의 핵심) -----
    // (x, z) 위치의 '바닥 높이'를 돌려준다.
    //   - 어떤 블록 위에 있으면 그 블록의 윗면 높이
    //   - 어디에도 안 걸리면 0 (방 바닥 = 메인방)
    float GetFloorHeightAt(float x, float z) const;

private:
    // ----- 블록 하나 = 평지 또는 계단 한 칸 -----
    // 박스는 항상 "바닥(y=0)부터 topY까지 꽉 찬" 형태.
    //   minX, maxX : X축 범위 (가로)
    //   topY       : 윗면 높이 (이 박스를 밟으면 이 높이)
    //   width      : Z축 너비 (세로 깊이)
    struct Block {
        float minX, maxX;
        float topY;
        float width;
        glm::mat4 model;   // 단위 큐브를 이 박스 위치/크기로 보내는 변환
    };

    Block* blocks;     // 블록 배열 (동적 할당)
    int    blockCount; // 블록 개수

    // 모든 블록이 공유하는 큐브 한 벌의 GPU 리소스
    unsigned int VAO;
    unsigned int VBO;
    unsigned int texture;

    // 헬퍼 함수
    void setupCube();
    unsigned int loadTexture(const char* path);
};

#endif