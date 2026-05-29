#ifndef INFOPANEL_H
#define INFOPANEL_H

#include <glad/glad.h>
#include "../Dependencies/include/glm/glm.hpp"
#include "shader.h"

// ==========================================
// InfoPanel 클래스 (전시 안내판 - 키오스크형)
// ------------------------------------------
// Staircase와 같은 철학: 단위 큐브를 model 행렬로 변형해 그린다.
// 두 부분으로 구성:
//   - base  : 바닥에서 올라온 받침대 (똑바로 선 기둥)
//   - panel : 받침대 위에 비스듬히 기운 정보 패널 (텍스처 붙는 면)
//
// 새 개념은 panel의 '기울임 회전(rotate)' 하나뿐.
// (Room에서 벽 만들 때 쓴 glm::rotate와 같은 함수)
// ==========================================
class InfoPanel {
public:
    // 생성자:
    //   posX, posZ : 안내판을 놓을 가로/세로 위치
    //   floorY     : 안내판이 서는 바닥 높이 (안내방 평지면 2.0)
    InfoPanel(float posX, float posZ, float floorY,
        const char* infoTexturePath = "textures/florrguide.jpg",
        float facingYaw = 0.0f);
    ~InfoPanel();

    void Draw(Shader& shader);

private:
    // 받침대와 패널 각각의 변환 행렬 + 텍스처
    glm::mat4 baseModel;
    glm::mat4 panelModel;

    unsigned int VAO;
    unsigned int VBO;
    unsigned int baseTexture;   // 받침대 텍스처 (일단 wall)
    unsigned int panelTexture;  // 패널 텍스처 (일단 floor, 나중에 목성정보로 교체)

    void setupCube();
    unsigned int loadTexture(const char* path);

    glm::mat4 infoQuadModel;    // 패널 앞면에 붙는 정보판 quad
    unsigned int quadVAO;       // quad 전용 VAO
    unsigned int quadVBO;
    unsigned int infoTexture;   // 정보 이미지 (일단 floor)
    void setupQuad();           // quad VAO 생성
};

#endif
