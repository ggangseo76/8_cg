#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "shader.h"
#include "Camera.h"

// opengl.h 없이 독립적으로 사용할 정점 구조체
struct JupiterVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texUV;
};

// 행성 정보 구조체
struct Planet {
    std::string name;
    float size;
    glm::mat4 axialTilt;
    float rotationSpeed;
    float rotationPhase;
    float orbitRadius;
    glm::mat4 orbitRotation;
    float orbitSpeed;
    float orbitPhase;
    glm::vec4 orbitColor;
    unsigned int diffuseTexID;  // 파일 이름 대신 텍스처 ID를 직접 저장!
    unsigned int specularTexID;
};

class JupiterSystem {
public:
    JupiterSystem();
    ~JupiterSystem();

    // 돔의 중앙 좌표(centerPos)를 받아서 원하는 곳에 목성계를 띄워줍니다.
    void Draw(Shader& shader, Camera& camera, float time, glm::vec3 centerPos);

private:
    std::vector<Planet> planets;

    // 순수 OpenGL 버퍼 (Mesh 클래스 사용 안 함!)
    unsigned int sphereVAO, sphereVBO, sphereEBO;
    int sphereIndexCount;

    unsigned int orbitVAO, orbitVBO, orbitEBO;
    int orbitIndexCount;

    // 헬퍼 함수들
    void setupSphere(float radius, int thetaCount, int phiCount);
    void setupOrbit(float radius, int segmentCount);
    unsigned int loadTexture(const char* path);
};