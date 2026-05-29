#include "Room.h"

// stb_image 구현부 활성화 - 프로젝트 전체에서 단 한 번만!
#define STB_IMAGE_IMPLEMENTATION
#include "../Dependencies/include/stb/stb_image.h"

#include "../Dependencies/include/glm/gtc/matrix_transform.hpp"
#include <iostream>


// ==========================================
// 단위 사각형 정점 데이터 (모든 면이 공유)
// ------------------------------------------
// - XZ 평면에 놓인 1x1 사각형, 노멀은 +Y (위쪽)
// - 각 면은 이 단위 사각형을 model 행렬로 회전/이동/스케일해서 사용
// ------------------------------------------
//   B(-0.5,0,-0.5) ┌────┐ C(0.5,0,-0.5)
//                  │ \  │
//                  │  \ │
//   A(-0.5,0, 0.5) └────┘ D(0.5,0, 0.5)
// ==========================================
static float quadVertices[] = {
    // 위치              노멀              UV
    -0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,  // A
     0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,  // D
     0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,  // C

    -0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,  // A
     0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,  // C
    -0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,  // B
};
// 참고: UV는 0~1 기본. 텍스처 반복은 model 행렬과 별개로
// 면마다 다른 반복 횟수 원하면 셰이더에 uniform float texScale 추가하면 됨.
// 일단 0~1로 두고 텍스처 자체를 큰 해상도로 쓰는 전략.


// ==========================================
// 생성자: 6개 면의 model 행렬 + GPU 리소스 준비
// ==========================================
Room::Room(float w, float d, float h)
    : width(w), depth(d), height(h)
{
    // ----- 6개 면 각각 VAO/VBO 생성 -----
    setupFace(floor);
    setupFace(ceiling);
    setupFace(wallFront);
    setupFace(wallBack);
    setupFace(wallLeft);
    setupFace(wallRight);

    // ----- 6개 면 model 행렬 계산 -----
    // 패턴: "위치 → 회전 → 크기" 순서

    // 바닥: 원점에 width × depth 평면
    floor.model = glm::mat4(1.0f);
    floor.model = glm::scale(floor.model, glm::vec3(width, 1.0f, depth));

    // 천장: y=height 위에, 180도 뒤집어서 노멀이 아래로 향하게
    ceiling.model = glm::mat4(1.0f);
    ceiling.model = glm::translate(ceiling.model, glm::vec3(0.0f, height, 0.0f));
    ceiling.model = glm::rotate(ceiling.model, glm::radians(180.0f), glm::vec3(1, 0, 0));
    ceiling.model = glm::scale(ceiling.model, glm::vec3(width, 1.0f, depth));

    // 앞벽 (-Z쪽): 누운 사각형을 X축으로 -90도 회전하면 +Y 노멀이 +Z 방향이 됨
    wallFront.model = glm::mat4(1.0f);
    wallFront.model = glm::translate(wallFront.model, glm::vec3(0.0f, height / 2.0f, -depth / 2.0f));
    wallFront.model = glm::rotate(wallFront.model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    wallFront.model = glm::scale(wallFront.model, glm::vec3(width, 1.0f, height));

    // 뒷벽 (+Z쪽): X축으로 +90도 회전
    wallBack.model = glm::mat4(1.0f);
    wallBack.model = glm::translate(wallBack.model, glm::vec3(0.0f, height / 2.0f, depth / 2.0f));
    wallBack.model = glm::rotate(wallBack.model, glm::radians(90.0f), glm::vec3(1, 0, 0));
    wallBack.model = glm::scale(wallBack.model, glm::vec3(width, 1.0f, height));

    // 왼벽 (-X쪽): Z축으로 -90도 회전하면 +Y 노멀이 +X 방향이 됨
    wallLeft.model = glm::mat4(1.0f);
    wallLeft.model = glm::translate(wallLeft.model, glm::vec3(-width / 2.0f, height / 2.0f, 0.0f));
    wallLeft.model = glm::rotate(wallLeft.model, glm::radians(-90.0f), glm::vec3(0, 0, 1));
    wallLeft.model = glm::scale(wallLeft.model, glm::vec3(height, 1.0f, depth));

    // 오른벽 (+X쪽): Z축으로 +90도 회전
    wallRight.model = glm::mat4(1.0f);
    wallRight.model = glm::translate(wallRight.model, glm::vec3(width / 2.0f, height / 2.0f, 0.0f));
    wallRight.model = glm::rotate(wallRight.model, glm::radians(90.0f), glm::vec3(0, 0, 1));
    wallRight.model = glm::scale(wallRight.model, glm::vec3(height, 1.0f, depth));

    // ----- 텍스처 로딩 -----
    // 일단 모든 면을 같은 텍스처로. 나중에 면마다 다르게 줄 수 있음.
    floor.texture = loadTexture("textures/floor.jpg");
    ceiling.texture = loadTexture("textures/whiteWall.jpg");
    wallFront.texture = loadTexture("textures/whiteWall.jpg");
    wallBack.texture = loadTexture("textures/whiteWall.jpg");
    wallLeft.texture = loadTexture("textures/whiteWall.jpg");
    wallRight.texture = loadTexture("textures/wall.png");
}


// ==========================================
// 소멸자: 모든 면의 GPU 리소스 정리
// ==========================================
Room::~Room() {
    Face* faces[] = { &floor, &ceiling, &wallFront, &wallBack, &wallLeft, &wallRight };
    for (Face* f : faces) {
        glDeleteVertexArrays(1, &f->VAO);
        glDeleteBuffers(1, &f->VBO);
        glDeleteTextures(1, &f->texture);
    }
}

bool Room::Contains(float x, float z, float margin) const {
    float halfW = width / 2.0f;   // 20
    float halfD = depth / 2.0f;   // 6
    // 오른쪽(+X) 면은 통로(벽 제거)라 margin을 주지 않는다 → 돔으로 넘어갈 수 있게
    return (x >= -halfW + margin) && (x <= halfW)
        && (z >= -halfD + margin) && (z <= halfD - margin);
}

// ==========================================
// setupFace: 한 면의 VAO/VBO 생성
// (모든 면이 같은 정점 데이터를 쓰지만 VAO는 각자 따로)
// ==========================================
void Room::setupFace(Face& face) {
    glGenVertexArrays(1, &face.VAO);
    glGenBuffers(1, &face.VBO);

    glBindVertexArray(face.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, face.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // location 0: position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // location 1: normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // location 2: texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}


// ==========================================
// loadTexture: 이미지 → GPU 텍스처
// ==========================================
unsigned int Room::loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 1)      format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
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
// drawFace: 한 면 그리기
// ==========================================
void Room::drawFace(const Face& face, Shader& shader) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, face.texture);
    shader.setInt("texture1", 0);

    shader.setMat4("model", face.model);

    glBindVertexArray(face.VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}


// ==========================================
// Draw: 매 프레임 호출 - 6개 면 모두 그림
// ==========================================
void Room::Draw(Shader& shader) {
    shader.use();

    drawFace(floor, shader);
    drawFace(ceiling, shader);
    drawFace(wallFront, shader);
    drawFace(wallBack, shader);
    drawFace(wallLeft, shader);
    //drawFace(wallRight, shader);
}