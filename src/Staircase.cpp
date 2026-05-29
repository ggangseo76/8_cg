#include "Staircase.h"

#include "../Dependencies/include/stb/stb_image.h"
#include "../Dependencies/include/glm/gtc/matrix_transform.hpp"
#include <iostream>


// ==========================================
// 단위 큐브 정점 데이터 (모든 블록이 공유)
// - 바닥면 y=0, 윗면 y=1 (바닥에서 위로 자라게)
// - x,z 는 -0.5 ~ +0.5
// - 각 정점: 위치(3) + 노멀(3) + UV(2) = 8 float (Room의 quad와 같은 레이아웃)
// ==========================================
static float cubeVertices[] = {
    // 아랫면 (-Y)
    -0.5f, 0.0f, -0.5f,  0.0f,-1.0f, 0.0f,  0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 0.0f,
     0.5f, 0.0f,  0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 1.0f,
     0.5f, 0.0f,  0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, 0.0f,  0.5f,  0.0f,-1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, 0.0f, -0.5f,  0.0f,-1.0f, 0.0f,  0.0f, 0.0f,
    // 윗면 (+Y) - 밟는 면
    -0.5f, 1.0f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
     0.5f, 1.0f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
     0.5f, 1.0f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
     0.5f, 1.0f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, 1.0f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, 1.0f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    // 앞면 (-Z)
    -0.5f, 0.0f, -0.5f,  0.0f, 0.0f,-1.0f,  0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,  0.0f, 0.0f,-1.0f,  1.0f, 0.0f,
     0.5f, 1.0f, -0.5f,  0.0f, 0.0f,-1.0f,  1.0f, 1.0f,
     0.5f, 1.0f, -0.5f,  0.0f, 0.0f,-1.0f,  1.0f, 1.0f,
    -0.5f, 1.0f, -0.5f,  0.0f, 0.0f,-1.0f,  0.0f, 1.0f,
    -0.5f, 0.0f, -0.5f,  0.0f, 0.0f,-1.0f,  0.0f, 0.0f,
    // 뒷면 (+Z)
    -0.5f, 0.0f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
     0.5f, 0.0f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
     0.5f, 1.0f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
     0.5f, 1.0f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
    -0.5f, 1.0f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
    -0.5f, 0.0f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
    // 왼면 (-X)
    -0.5f, 1.0f,  0.5f, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    -0.5f, 1.0f, -0.5f, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, 0.0f, -0.5f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, 0.0f, -0.5f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, 0.0f,  0.5f, -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    -0.5f, 1.0f,  0.5f, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    // 오른면 (+X)
     0.5f, 1.0f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
     0.5f, 1.0f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
     0.5f, 0.0f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f, 0.0f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f, 0.0f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
     0.5f, 1.0f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
};


// ==========================================
// 생성자: 박스 목록 정의 + 각 박스의 model 행렬 계산
// ------------------------------------------
// 구조 (--\__):  안내방(높음) -> 계단3단(내려감) -> 메인방(바닥, 박스없음)
// ==========================================
Staircase::Staircase() {
    setupCube();
    texture = loadTexture("textures/floor.jpg");

    // ----- 박스 목록 직접 정의 -----
    // {minX, maxX, topY, width}
    //   안내방 B : x -18~-4, 윗면 2.0, 세로폭 8
    //   계단 3단 : 2.0 -> 1.5 -> 1.0 -> 0.5 로 내려감
    //   메인방 A : y=0 (방 바닥 그대로라 박스 없음)
    blockCount = 4;
    blocks = new Block[blockCount];

    blocks[0] = { -20.0f, -4.0f, 2.0f, 12.0f, glm::mat4(1.0f) };  // 안내방
    blocks[1] = { -4.0f, -2.0f, 1.5f, 12.0f, glm::mat4(1.0f) };   // 계단1
    blocks[2] = { -2.0f,  0.0f, 1.0f, 12.0f, glm::mat4(1.0f) };   // 계단2
    blocks[3] = { 0.0f,  2.0f, 0.5f, 12.0f, glm::mat4(1.0f) };   // 계단3

    // ----- 각 박스의 model 행렬 계산 -----
    // 큐브(밑면 y=0~윗면 y=1)를 박스 크기로 늘리고 위치로 옮긴다.
    for (int i = 0; i < blockCount; i++) {
        float cx = (blocks[i].minX + blocks[i].maxX) / 2.0f;  // 가로 중심
        float sx = blocks[i].maxX - blocks[i].minX;           // 가로 폭
        float sy = blocks[i].topY;                            // 높이(바닥~윗면)
        float sz = blocks[i].width;                           // 세로 너비

        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, glm::vec3(cx, 0.0f, 0.0f));     // 가로 중심으로 이동
        m = glm::scale(m, glm::vec3(sx, sy, sz));             // 박스 크기로 늘림
        blocks[i].model = m;
    }
}


// ==========================================
// 소멸자
// ==========================================
Staircase::~Staircase() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &texture);
    delete[] blocks;
}


// ==========================================
// setupCube: 큐브 VAO/VBO 생성 (Room::setupFace 와 같은 레이아웃)
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
// loadTexture: 이미지 -> GPU 텍스처 (Room의 것과 동일)
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
// Draw: 모든 블록(평지/계단)을 그린다
// ==========================================
void Staircase::Draw(Shader& shader) {
    shader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader.setInt("texture1", 0);

    glBindVertexArray(VAO);
    for (int i = 0; i < blockCount; i++) {
        shader.setMat4("model", blocks[i].model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
}


// ==========================================
// GetFloorHeightAt: 카메라가 물어보는 함수 (높이 보정의 핵심)
// ------------------------------------------
// (x, z)가 어떤 블록의 범위 안에 들어가는지 하나씩 검사.
//   - 걸리는 블록이 있으면 그 블록의 윗면 높이 반환
//   - 어디에도 안 걸리면 0 (방 바닥 = 메인방)
// 여러 블록이 겹치는 일은 없게 설계했으므로 첫 번째로 걸리는 걸 쓴다.
// ==========================================
float Staircase::GetFloorHeightAt(float x, float z) const {
    for (int i = 0; i < blockCount; i++) {
        const Block& b = blocks[i];
        bool inX = (x >= b.minX && x <= b.maxX);
        bool inZ = (z >= -b.width / 2.0f && z <= b.width / 2.0f);
        if (inX && inZ)
            return b.topY;
    }
    return 0.0f;  // 어떤 블록에도 안 걸림 -> 방 바닥
}