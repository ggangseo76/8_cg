#ifndef PLANETARIUM_DOME_H
#define PLANETARIUM_DOME_H

#include "shader.h"
#include <vector>

class PlanetariumDome {
public:
    // 중심 X, Z / 반지름(radius) / 높이(height)
    PlanetariumDome(float centerX, float centerZ, float radius, float height);
    ~PlanetariumDome();

    bool Contains(float x, float z, float margin) const;
    void Draw(Shader& shader);

private:
    unsigned int VAO, VBO;
    unsigned int floorTextureID;
    unsigned int spaceTextureID;
    int vertexCount; // 동적으로 생성된 정점 개수

    float centerX, centerZ, radius, height;
    unsigned int loadTexture(const char* path);
    void setupDome(float cx, float cz, float r, float h);
};

#endif