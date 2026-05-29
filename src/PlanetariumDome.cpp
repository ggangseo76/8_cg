#include "PlanetariumDome.h"
#include "../Dependencies/include/stb/stb_image.h"
#include "../Dependencies/include/glm/glm.hpp"
#include "../Dependencies/include/glm/gtc/matrix_transform.hpp"
#include <iostream>
#include <cmath>

const float PI = 3.14159265359f;

PlanetariumDome::PlanetariumDome(float centerX, float centerZ, float radius, float height) {
    this->centerX = centerX;   
    this->centerZ = centerZ;   
    this->radius = radius;    
    this->height = height;    
    
    // 바닥과 우주 텍스처를 각각 따로 로드합니다.
    floorTextureID = loadTexture("textures/floor.jpg");
    spaceTextureID = loadTexture("textures/wall.png");
    setupDome(centerX, centerZ, radius, height);
}

bool PlanetariumDome::Contains(float x, float z, float margin) const {
    float dx = x - centerX;
    float dz = z - centerZ;
    float r = radius - margin;
    return (dx * dx + dz * dz) <= (r * r);
}

PlanetariumDome::~PlanetariumDome() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    // 텍스처 메모리도 각각 해제합니다.
    glDeleteTextures(1, &floorTextureID);
    glDeleteTextures(1, &spaceTextureID);
}

void PlanetariumDome::setupDome(float cx, float cz, float r, float h) {
    std::vector<float> vertices;
    int segments = 360;

    // 1. 바닥 (Floor) 생성 (Y = 0.01f 로 띄움)
    float floorY = 0.01f;
    for (int i = 0; i < segments; i++) {
        float theta1 = (2.0f * PI * i) / segments;
        float theta2 = (2.0f * PI * (i + 1)) / segments;

        vertices.insert(vertices.end(), { cx, floorY, cz,  0.0f, 1.0f, 0.0f,  0.5f, 0.5f });
        vertices.insert(vertices.end(), { cx + r * cos(theta1), floorY, cz + r * sin(theta1),  0.0f, 1.0f, 0.0f,  0.5f + 0.5f * cos(theta1), 0.5f + 0.5f * sin(theta1) });
        vertices.insert(vertices.end(), { cx + r * cos(theta2), floorY, cz + r * sin(theta2),  0.0f, 1.0f, 0.0f,  0.5f + 0.5f * cos(theta2), 0.5f + 0.5f * sin(theta2) });
    }

    // 2. 둥근 벽 (Wall) 생성
    float corridorHeight = 11.0f; // ★ 복도의 천장 높이 설정

    for (int i = 0; i < segments; i++) {
        float theta1 = (2.0f * PI * i) / segments;
        float theta2 = (2.0f * PI * (i + 1)) / segments;

        float bottomY = 0.0f;
        float bottomV = 0.0f; // 텍스처 UV V좌표 하단

        // ★ [수정됨] 입구(167도~192도) 구간 처리
        if (i >= 167 && i <= 192) {
            // 이 구간은 바닥(0)부터 복도 높이(11)까지는 사람이 지나다니게 비워두고,
            // 복도 높이(11)부터 돔 천장(20)까지만 벽을 그려서 위쪽 구멍을 메꿉니다!
            bottomY = corridorHeight;
            bottomV = corridorHeight / h; // 텍스처가 위아래로 눌리지 않도록 비율 맞춤
        }

        float nx1 = -cos(theta1), nz1 = -sin(theta1);
        float nx2 = -cos(theta2), nz2 = -sin(theta2);

        float u1 = (float)i / segments * 20.0f;
        float u2 = (float)(i + 1) / segments * 20.0f;

        // 벽을 bottomY 부터 천장(h) 까지만 생성
        vertices.insert(vertices.end(), { cx + r * cos(theta1), bottomY, cz + r * sin(theta1),  nx1, 0.0f, nz1,  u1, bottomV });
        vertices.insert(vertices.end(), { cx + r * cos(theta1), h,       cz + r * sin(theta1),  nx1, 0.0f, nz1,  u1, 1.0f });
        vertices.insert(vertices.end(), { cx + r * cos(theta2), h,       cz + r * sin(theta2),  nx2, 0.0f, nz2,  u2, 1.0f });

        vertices.insert(vertices.end(), { cx + r * cos(theta1), bottomY, cz + r * sin(theta1),  nx1, 0.0f, nz1,  u1, bottomV });
        vertices.insert(vertices.end(), { cx + r * cos(theta2), h,       cz + r * sin(theta2),  nx2, 0.0f, nz2,  u2, 1.0f });
        vertices.insert(vertices.end(), { cx + r * cos(theta2), bottomY, cz + r * sin(theta2),  nx2, 0.0f, nz2,  u2, bottomV });
    }

    // 3. 천장 (Ceiling) 뚜껑 덮기
    for (int i = 0; i < segments; i++) {
        float theta1 = (2.0f * PI * i) / segments;
        float theta2 = (2.0f * PI * (i + 1)) / segments;

        vertices.insert(vertices.end(), { cx, h, cz,  0.0f, -1.0f, 0.0f,  0.5f, 0.5f });
        vertices.insert(vertices.end(), { cx + r * cos(theta2), h, cz + r * sin(theta2),  0.0f, -1.0f, 0.0f,  0.5f + 0.5f * cos(theta2), 0.5f + 0.5f * sin(theta2) });
        vertices.insert(vertices.end(), { cx + r * cos(theta1), h, cz + r * sin(theta1),  0.0f, -1.0f, 0.0f,  0.5f + 0.5f * cos(theta1), 0.5f + 0.5f * sin(theta1) });
    }

    vertexCount = vertices.size() / 8;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

unsigned int PlanetariumDome::loadTexture(const char* path) {
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
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
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



void PlanetariumDome::Draw(Shader& shader) {
    shader.use();
    shader.setInt("texture1", 0);

    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);

    glBindVertexArray(VAO);

    // ★ 1. 바닥 렌더링 파트
    // 바닥 정점은 segments(360) * 꼭짓점(3) = 1080개 입니다.
    int floorVertexCount = 360 * 3;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTextureID);
    // 배열의 처음 0번째부터 1080개까지만 잘라서 바닥 텍스처로 그립니다.
    glDrawArrays(GL_TRIANGLES, 0, floorVertexCount);

    // ★ 2. 둥근 벽과 천장 렌더링 파트
    // 배열의 1080번째 이후 정점들은 모두 우주 텍스처로 그립니다.
    glBindTexture(GL_TEXTURE_2D, spaceTextureID);
    glDrawArrays(GL_TRIANGLES, floorVertexCount, vertexCount - floorVertexCount);

    glBindVertexArray(0);
}