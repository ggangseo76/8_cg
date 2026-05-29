#ifndef ROOM_H
#define ROOM_H

#include <glad/glad.h>
#include "../Dependencies/include/glm/glm.hpp"
#include "shader.h"

// ==========================================
// Room 클래스 (확장: 바닥 + 천장 + 벽 4개)
// ==========================================
class Room {
public:
    Room(float width, float depth, float height);
    ~Room();
    // (x,z)가 방 안인지 검사. margin만큼 벽에서 떨어진 안쪽 영역만 true.
    bool Contains(float x, float z, float margin) const;
    void Draw(Shader& shader);

private:
    float width;
    float depth;
    float height;

    // 한 면을 표현하는 데 필요한 GPU 리소스 + 변환 행렬
    struct Face {
        unsigned int VAO;
        unsigned int VBO;
        unsigned int texture;
        glm::mat4 model;        // 단위 사각형을 이 면 위치로 보내는 변환
    };

    // 6개 면
    Face floor;
    Face ceiling;
    Face wallFront;     // -Z쪽
    Face wallBack;      // +Z쪽
    Face wallLeft;      // -X쪽
    Face wallRight;     // +X쪽

    // 헬퍼 함수
    void   setupFace(Face& face);
    unsigned int loadTexture(const char* path);
    void   drawFace(const Face& face, Shader& shader);
};

#endif