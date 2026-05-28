#include "Staircase.h"

#include "../Dependencies/include/stb/stb_image.h"
#include "../Dependencies/include/glm/gtc/matrix_transform.hpp"
#include <iostream>


// ==========================================
// 단위 큐브 정점 데이터 (모든 단이 공유)
// ------------------------------------------
// - 한 변이 1인 큐브. 중심이 (0,0,0) 이 아니라
//   '바닥면이 y=0, 윗면이 y=1' 이 되도록 잡았다.
//   (계단은 바닥에서 위로 쌓는 거라 이게 계산이 편함)
// - x,z 는 -0.5 ~ +0.5,  y 는 0 ~ 1
// - 6면 × 2삼각형 × 3정점 = 36개 정점
// - 각 정점: 위치(3) + 노멀(3) + UV(2) = 8 float
//   (Room의 quad와 같은 레이아웃이라 같은 셰이더를 쓸 수 있다)
// ==========================================
static float cubeVertices[] = {
    // ----- 아랫면 (y=0, 노멀 -Y) -----
    -0.5f, 0.0f, -0.5f,  0.0f,-1.0f, 0.0f,  0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 0.0f,
     0.5f, 0.0f,  0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 1.0f,
     0.5f, 0.0f,  0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, 0.0f,  0.5f,  0.0f,-1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, 0.0f, -0.5f,  0.0f,-1.0f, 0.0f,  0.0f, 0.0f,

    // ----- 윗면 (y=1, 노멀 +Y) ← 관람객이 밟고 보는 면 -----
    -0.5f, 1.0f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
     0.5f, 1.0f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
     0.5f, 1.0f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
     0.5f, 1.0f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, 1.0f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, 1.0f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,

    // ----- 앞면 (-Z, 노멀 -Z) -----
    -0.5f, 0.0f, -0.5f,  0.0f, 0.0f,-1.0f,  0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,  0.0f, 0.0f,-1.0f,  1.0f, 0.0f,
     0.5f, 1.0f, -0.5f,  0.0f, 0.0f,-1.0f,  1.0f, 1.0f,
     0.5f, 1.0f, -0.5f,  0.0f, 0.0f,-1.0f,  1.0f, 1.0f,
    -0.5f, 1.0f, -0.5f,  0.0f, 0.0f,-1.0f,  0.0f, 1.0f,
    -0.5f, 0.0f, -0.5f,  0.0f, 0.0f,-1.0f,  0.0f, 0.0f,

    // ----- 뒷면 (+Z, 노멀 +Z) -----
    -0.5f, 0.0f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
     0.5f, 0.0f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
     0.5f, 1.0f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
     0.5f, 1.0f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
    -0.5f, 1.0f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
    -0.5f, 0.0f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,

    // ----- 왼면 (-X, 노멀 -X) -----
    -0.5f, 1.0f,  0.5f, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    -0.5f, 1.0f, -0.5f, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, 0.0f, -0.5f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, 0.0f, -0.5f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, 0.0f,  0.5f, -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    -0.5f, 1.0f,  0.5f, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

    // ----- 오른면 (+X, 노멀 +X) -----
     0.5f, 1.0f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
     0.5f, 1.0f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
     0.5f, 0.0f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f, 0.0f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f, 0.0f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
     0.5f, 1.0f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
};


// ==========================================
// 생성자: 치수 저장 + 단들의 model 행렬 계산
// ==========================================
Staircase::Staircase(float sH, float sD, int sC, float w)
    : stepHeight(sH), stepDepth(sD), stepCount(sC), width(w)
{
    // ----- 큐브 GPU 리소스 1벌 생성 (모든 단이 공유) -----
    setupCube();
    texture = loadTexture("textures/floor.jpg");

    // ----- 계단을 X축에서 어디서 시작할지 -----
    // 계단 전체 가로폭 = stepCount * stepDepth (예: 6 * 2.0 = 12)
    // 그 폭의 절반만큼 왼쪽으로 옮겨서 'x=0 근처를 중심'으로 둔다.
    float totalRun = stepCount * stepDepth;
    startX = -totalRun / 2.0f;     // 예) -6.0  → 계단은 x=-6 ~ +6

    // ----- 단 개수만큼 배열 할당 -----
    steps = new Step[stepCount];

    // ----- 각 단의 model 행렬 계산 -----
    // i번째 단:
    //   - 가로 위치 x : startX 에서 i칸만큼 +X 이동 (왼쪽이 낮고 오른쪽이 높음)
    //   - 높이      y : i가 커질수록 (i+1)단 만큼 쌓임
    //   - 크기        : 가로 stepDepth, 높이 (i+1)*stepHeight, 세로 width
    //
    // 핵심 트릭: 각 단을 '바닥(y=0)부터 그 단 윗면까지' 통째로 채운 박스로 만든다.
    //   → 0번 단은 높이 0.5짜리 한 칸,
    //     1번 단은 높이 1.0짜리(아래까지 꽉 찬) 박스 …
    //   이렇게 하면 옆에서 봤을 때 빈틈 없는 계단 모양이 된다.
    for (int i = 0; i < stepCount; i++) {
        float h = (i + 1) * stepHeight;                 // 이 단 윗면 높이
        float cx = startX + (i + 0.5f) * stepDepth;     // 이 단 가로 중심

        glm::mat4 m = glm::mat4(1.0f);
        // 1) 위치로 이동: 가로 중심 cx, 세로 중심 0, 높이는 0(바닥)에서 시작
        m = glm::translate(m, glm::vec3(cx, 0.0f, 0.0f));
        // 2) 크기 조절: 큐브(밑면 y=0~윗면 y=1)를 이 단 크기로 늘림
        m = glm::scale(m, glm::vec3(stepDepth, h, width));
        //    - x: stepDepth (한 단 폭)
        //    - y: h        (바닥부터 이 단 윗면까지)  ← 큐브 y가 0~1이라 그대로 높이가 됨
        //    - z: width    (계단 세로 너비)

        steps[i].model = m;
    }
}


// ==========================================
// 소멸자: GPU 리소스 + 배열 정리
// ==========================================
Staircase::~Staircase() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &texture);
    delete[] steps;     // new[] 로 잡았으니 delete[] 로 해제
}


// ==========================================
// setupCube: 큐브 VAO/VBO 생성
// (Room::setupFace 와 같은 정점 레이아웃: 위치3 + 노멀3 + UV2)
// ==========================================
void Staircase::setupCube() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}


// ==========================================
// loadTexture: Room의 것과 동일 (이미지 → GPU 텍스처)
// ==========================================
unsigned int Staircase::loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int w, h, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &w, &h, &nrChannels, 0);

    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 1)      format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0,
            format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        std::cout << "Failed to load texture: " << path << std::endl;
    }

    stbi_image_free(data);
    return textureID;
}


// ==========================================
// Draw: 모든 단을 그린다
// (큐브 VAO는 하나, model 행렬만 단마다 바꿔 끼우며 그림)
// ==========================================
void Staircase::Draw(Shader& shader) {
    shader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader.setInt("texture1", 0);

    glBindVertexArray(VAO);
    for (int i = 0; i < stepCount; i++) {
        shader.setMat4("model", steps[i].model);     // 이 단의 변환으로 교체
        glDrawArrays(GL_TRIANGLES, 0, 36);            // 큐브는 36개 정점
    }
    glBindVertexArray(0);
}


// ==========================================
// GetFloorHeightAt: 카메라가 물어보는 함수 (높이 보정의 핵심!)
// ------------------------------------------
// (x, z) 위치의 바닥 높이를 돌려준다.
//   1) 계단의 가로 범위(startX ~ startX+totalRun) 밖이면 → 0 (방 바닥)
//   2) 세로 범위(±width/2) 밖이면 → 0 (계단 옆을 벗어남)
//   3) 안에 있으면 → x가 몇 번째 단인지 계산해서 그 단 윗면 높이 반환
// ==========================================
float Staircase::GetFloorHeightAt(float x, float z) const {
    float totalRun = stepCount * stepDepth;

    // 1) 가로(X) 범위 밖이면 방 바닥
    if (x < startX || x > startX + totalRun)
        return 0.0f;

    // 2) 세로(Z) 범위 밖이면 방 바닥
    if (z < -width / 2.0f || z > width / 2.0f)
        return 0.0f;

    // 3) x가 startX에서 얼마나 떨어졌는지 → 몇 번째 단인지
    float offset = x - startX;              // 0 ~ totalRun
    int index = (int)(offset / stepDepth);  // 0 ~ stepCount-1

    // 안전장치: 경계에서 index가 범위를 넘지 않게 조정
    if (index < 0) index = 0;
    if (index >= stepCount) index = stepCount - 1;

    // 그 단의 윗면 높이 = (index+1) * stepHeight
    return (index + 1) * stepHeight;
}