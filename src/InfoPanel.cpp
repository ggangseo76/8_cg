#include "InfoPanel.h"

#include "../Dependencies/include/stb/stb_image.h"
#include "../Dependencies/include/glm/gtc/matrix_transform.hpp"
#include <iostream>


// ==========================================
// 단위 큐브 정점 (Staircase의 것과 동일: 바닥 y=0, 윗면 y=1)
// 위치(3)+노멀(3)+UV(2) = 8 float
// ==========================================
static float cubeVertices[] = {
    // 아랫면 (-Y)
    -0.5f, 0.0f, -0.5f,  0.0f,-1.0f, 0.0f,  0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 0.0f,
     0.5f, 0.0f,  0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 1.0f,
     0.5f, 0.0f,  0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, 0.0f,  0.5f,  0.0f,-1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, 0.0f, -0.5f,  0.0f,-1.0f, 0.0f,  0.0f, 0.0f,
    // 윗면 (+Y)
    -0.5f, 1.0f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
     0.5f, 1.0f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
     0.5f, 1.0f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
     0.5f, 1.0f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, 1.0f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, 1.0f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    // 앞면 (-Z) - 패널에서 텍스트가 보일 면
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

static float quadVertices[] = {
    // 위치              노멀            UV
    -0.5f, -0.5f, 0.0f,  0.0f,0.0f,1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,  0.0f,0.0f,1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, 0.0f,  0.0f,0.0f,1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, 0.0f,  0.0f,0.0f,1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, 0.0f,  0.0f,0.0f,1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, 0.0f,  0.0f,0.0f,1.0f,  0.0f, 0.0f,
};


// ==========================================
// 생성자: 받침대 + 패널의 변환 행렬 계산
// ==========================================
InfoPanel::InfoPanel(float posX, float posZ, float floorY,
    const char* infoTexturePath, float facingYaw) {
    setupCube();
    baseTexture  = loadTexture("textures/panel2.jpg");   // 받침대: 벽 텍스처(회색 느낌)
    panelTexture = loadTexture("textures/panel2.jpg");  // 패널

    // ----- 받침대 (똑바로 선 기둥) -----
    // 바닥(floorY)에서 위로 baseH 만큼 솟은 박스
    float baseW = 1.3f;   // 가로 폭 (넓게)
    float baseH = 0.9f;   // 높이 (낮게)
    float baseD = 0.1f;   // 세로 두께 (도톰하게)
    {
        glm::mat4 m = glm::mat4(1.0f);
        // 안내판 위치로 이동 (바닥 floorY 위에 세움)
        m = glm::translate(m, glm::vec3(posX, floorY, posZ));
        m = glm::rotate(m, glm::radians(facingYaw), glm::vec3(0, 1, 0));
        m = glm::scale(m, glm::vec3(baseW, baseH, baseD));
        baseModel = m;
    }

    // ----- 패널 (비스듬히 기운 판) -----
    // 받침대 윗부분에서 시작해, X축 기준으로 뒤로 기울인 얇은 판
    float panelW = 1.3f;     // 가로 폭 (받침대와 비슷)
    float panelLen = 0.9f;   // 패널 길이 (받침대 높이와 비슷하게, 작게)
    float panelThick = 0.08f; // 두께 (그대로 얇게)
    float tilt = 55.0f;      // 기울임 더 크게 → 더 눕힘 (30→55도)
    {
        glm::mat4 m = glm::mat4(1.0f);
        // 1) 받침대 꼭대기 근처로 이동
        m = glm::translate(m, glm::vec3(posX, floorY + baseH, posZ));
        // 2) X축으로 기울임 (앞면이 위를 보도록 비스듬히)
        m = glm::rotate(m, glm::radians(facingYaw), glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::rotate(m, glm::radians(tilt), glm::vec3(1.0f, 0.0f, 0.0f));
        // 3) 패널 크기로 늘림 (얇은 판)
        m = glm::scale(m, glm::vec3(panelW, panelLen, panelThick));
        panelModel = m;

        setupQuad();
        infoTexture = loadTexture(infoTexturePath);

        // 정보판 quad: 패널과 같은 위치·각도, 앞면에 살짝 띄워 얹음
        {
            glm::mat4 m = glm::mat4(1.0f);

            // 1. 패널과 동일한 기준점(힌지)으로 이동
            m = glm::translate(m, glm::vec3(posX, floorY + baseH, posZ));

            m = glm::rotate(m, glm::radians(facingYaw), glm::vec3(0.0f, 1.0f, 0.0f));

            // 2. 패널과 똑같이 X축으로 기울임 (이제 로컬 축도 함께 기울어짐)
            m = glm::rotate(m, glm::radians(tilt), glm::vec3(1.0f, 0.0f, 0.0f));

            // 3. 패널의 '앞면 중앙'으로 이동
            m = glm::translate(m, glm::vec3(0.0f, panelLen / 2.0f, -(panelThick / 2.0f + 0.01f)));
            m = glm::rotate(m, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            m = glm::scale(m, glm::vec3(panelW * 0.9f, panelLen * 0.9f, 1.0f));

            infoQuadModel = m;
        }
    }
}


// ==========================================
// 소멸자
// ==========================================
InfoPanel::~InfoPanel() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &baseTexture);
    glDeleteTextures(1, &panelTexture);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteTextures(1, &infoTexture);
}


// ==========================================
// setupCube (Staircase와 동일)
// ==========================================
void InfoPanel::setupCube() {
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

void InfoPanel::setupQuad() {
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

// ==========================================
// loadTexture (Room/Staircase와 동일)
// ==========================================
unsigned int InfoPanel::loadTexture(const char* path) {
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
// Draw: 받침대 + 패널 그리기
// ==========================================
void InfoPanel::Draw(Shader& shader) {
    shader.use();
    glBindVertexArray(VAO);

    // 받침대
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, baseTexture);
    shader.setInt("texture1", 0);
    shader.setMat4("model", baseModel);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // 패널
    glBindTexture(GL_TEXTURE_2D, panelTexture);
    shader.setMat4("model", panelModel);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // 정보판 quad
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, infoTexture);
    shader.setMat4("model", infoQuadModel);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
}
