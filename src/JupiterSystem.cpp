#include "JupiterSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../Dependencies/include/stb/stb_image.h" // 경로를 프로젝트에 맞게 수정해주세요!
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace {
    // 행렬 헬퍼 함수들
    glm::mat4 T(const glm::vec3& v) { return glm::translate(glm::mat4(1.0f), v); }
    glm::mat4 S(const glm::vec3& v) { return glm::scale(glm::mat4(1.0f), v); }
    glm::mat4 S(float s) { return S(glm::vec3(s)); }
    glm::mat4 R(float angleRadians, const glm::vec3& axis) { return glm::rotate(glm::mat4(1.0f), angleRadians, axis); }
    glm::mat4 R_x(float angleRadians) { return R(angleRadians, glm::vec3(1.0f, 0.0f, 0.0f)); }
    glm::mat4 R_y(float angleRadians) { return R(angleRadians, glm::vec3(0.0f, 1.0f, 0.0f)); }
    glm::mat4 R_z(float angleRadians) { return R(angleRadians, glm::vec3(0.0f, 0.0f, 1.0f)); }
    glm::mat4 makeAxialTiltX(float degree) { return R_z(glm::radians(-degree)); }

    glm::vec3 makeOrbitPosition(const Planet& planet, float time) {
        float orbitAngle = time * planet.orbitSpeed + planet.orbitPhase;
        glm::vec4 position = planet.orbitRotation * R_y(orbitAngle) * glm::vec4(planet.orbitRadius, 0.0f, 0.0f, 1.0f);
        return glm::vec3(position);
    }

    glm::mat4 makePlanetModel(const Planet& planet, float time) {
        float rotationAngle = time * planet.rotationSpeed + planet.rotationPhase;
        glm::vec3 orbitPosition = makeOrbitPosition(planet, time);
        return T(orbitPosition) * planet.orbitRotation * planet.axialTilt * R_y(rotationAngle) * S(planet.size);
    }

    glm::mat4 makeOrbitModel(const Planet& planet) {
        return planet.orbitRotation * S(planet.orbitRadius);
    }
}

JupiterSystem::JupiterSystem() {
    // 1. 공용 구체(Sphere) 및 궤도(Orbit) 버퍼 세팅
    // 모든 행성은 크기만 다를 뿐 똑같은 '구'이므로 하나만 만들어서 돌려 씁니다! (메모리 최적화)
    setupSphere(1.0f, 32, 32);
    setupOrbit(1.0f, 128);

    // 2. 텍스처 경로 설정
    std::string imageFolderPath = fs::current_path().string();
    std::string currentDir = imageFolderPath + "\\textures";

    // 3. 행성 데이터 초기화 및 텍스처 직접 로드
    planets = {
        { "Jupiter", 1.5f, makeAxialTiltX(3.0f), 0.8f, 0.0f, 0.0f, glm::mat4(1.0f), 0.0f, 0.0f, glm::vec4(1.0f, 0.7f, 0.35f, 1.0f),
          loadTexture((currentDir + "\\jupiter.jpg").c_str()), loadTexture((currentDir + "\\jupiter.jpg").c_str()) },
        { "Io", 0.25f, makeAxialTiltX(1.0f), 2.4f, 0.0f, 4.0f, R_x(glm::radians(0.05f)), 1.5f, 0.0f, glm::vec4(0.95f, 0.75f, 0.25f, 1.0f),
          loadTexture((currentDir + "\\io.jpg").c_str()), loadTexture((currentDir + "\\io.jpg").c_str()) },
        { "Europa", 0.22f, makeAxialTiltX(2.0f), 1.8f, 0.0f, 6.36f, R_x(glm::radians(0.47f)), 1.0f, 1.7f, glm::vec4(0.75f, 0.9f, 1.0f, 1.0f),
          loadTexture((currentDir + "\\europa.jpg").c_str()), loadTexture((currentDir + "\\europa.jpg").c_str()) },
        { "Ganymede", 0.35f, makeAxialTiltX(3.0f), 1.2f, 0.0f, 10.15f, R_z(glm::radians(0.20f)), 0.6f, 3.2f, glm::vec4(0.65f, 0.65f, 0.65f, 1.0f),
          loadTexture((currentDir + "\\ganymede.jpg").c_str()), loadTexture((currentDir + "\\ganymede.jpg").c_str()) },
        { "Callisto", 0.35f, makeAxialTiltX(3.0f), 1.2f, 0.0f, 17.86f, R_z(glm::radians(0.20f)), 0.6f, 3.2f, glm::vec4(0.65f, 0.65f, 0.65f, 1.0f),
          loadTexture((currentDir + "\\callisto.jpg").c_str()), loadTexture((currentDir + "\\callisto.jpg").c_str()) }
    };
}

JupiterSystem::~JupiterSystem() {
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);

    glDeleteVertexArrays(1, &orbitVAO);
    glDeleteBuffers(1, &orbitVBO);
    glDeleteBuffers(1, &orbitEBO);
}

void JupiterSystem::setupSphere(float radius, int thetaCount, int phiCount) {
    std::vector<JupiterVertex> vertices;
    std::vector<GLuint> indices;
    const float PI = 3.14159265359f;

    for (int i = 0; i <= phiCount; ++i) {
        float phi = (float)i / phiCount * PI;
        for (int j = 0; j <= thetaCount; ++j) {
            float theta = (float)j / thetaCount * 2.0f * PI;
            glm::vec3 pos(radius * sinf(phi) * cosf(theta), radius * cosf(phi), radius * sinf(phi) * sinf(theta));
            JupiterVertex vertex;
            vertex.position = pos;
            vertex.normal = glm::normalize(pos);
            vertex.texUV = glm::vec2((float)j / thetaCount, (float)i / phiCount);
            vertices.push_back(vertex);
        }
    }
    for (int i = 0; i < phiCount; ++i) {
        int k1 = i * (thetaCount + 1);
        int k2 = k1 + thetaCount + 1;
        for (int j = 0; j < thetaCount; ++j, ++k1, ++k2) {
            if (i != 0) { indices.push_back(k1); indices.push_back(k2); indices.push_back(k1 + 1); }
            if (i != phiCount - 1) { indices.push_back(k1 + 1); indices.push_back(k2); indices.push_back(k2 + 1); }
        }
    }

    sphereIndexCount = indices.size();

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(JupiterVertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // 위치 속성
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(JupiterVertex), (void*)0);
    glEnableVertexAttribArray(0);
    // 노말 속성
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(JupiterVertex), (void*)offsetof(JupiterVertex, normal));
    glEnableVertexAttribArray(1);
    // 텍스처 UV 속성
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(JupiterVertex), (void*)offsetof(JupiterVertex, texUV));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void JupiterSystem::setupOrbit(float radius, int segmentCount) {
    std::vector<JupiterVertex> vertices;
    std::vector<GLuint> indices;
    const float PI = 3.14159265359f;

    for (int i = 0; i < segmentCount; ++i) {
        float theta = (float)i / segmentCount * 2.0f * PI;
        JupiterVertex vertex{};
        vertex.position = glm::vec3(radius * cosf(theta), 0.0f, radius * sinf(theta));
        vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        vertex.texUV = glm::vec2(0.0f);
        vertices.push_back(vertex);
        indices.push_back(i);
    }
    // 궤도는 선으로 그릴 것이므로 선 루프(LINE_LOOP) 형태로 연결합니다.
    orbitIndexCount = indices.size();

    glGenVertexArrays(1, &orbitVAO);
    glGenBuffers(1, &orbitVBO);
    glGenBuffers(1, &orbitEBO);

    glBindVertexArray(orbitVAO);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(JupiterVertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, orbitEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(JupiterVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(JupiterVertex), (void*)offsetof(JupiterVertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(JupiterVertex), (void*)offsetof(JupiterVertex, texUV));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

unsigned int JupiterSystem::loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

void JupiterSystem::Draw(Shader& shader, Camera& camera, float time, glm::vec3 centerPos) {
    shader.use();
    glUniform1i(glGetUniformLocation(shader.ID, "diffuse0"), 0);
    GLint modelLoc = glGetUniformLocation(shader.ID, "model");

    for (int i = 0; i < planets.size(); i++) {
        Planet planet = planets[i];
        glm::mat4 systemOffset = glm::translate(glm::mat4(1.0f), centerPos);

        // 1. 궤도(Orbit) 그리기
        glm::mat4 OrbitModel = systemOffset * makeOrbitModel(planet);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(OrbitModel));

        glBindVertexArray(orbitVAO);
        glDrawElements(GL_LINE_LOOP, orbitIndexCount, GL_UNSIGNED_INT, 0);

        // 2. 행성(Planet) 구체 그리기
        glm::mat4 PlanetModel = systemOffset * makePlanetModel(planet, time);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(PlanetModel));

        // 해당 행성의 텍스처 바인딩
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, planet.diffuseTexID);

        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}