#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Camera.h"
#include "shader.h"
#include "Room.h"
#include "Staircase.h"
#include "InfoPanel.h"
#include "PlanetariumDome.h"
#include "JupiterSystem.h"

// ==========================================
// [1. 전역 변수 및 설정]
// ==========================================
// 카메라 객체 초기 위치
Camera camera(glm::vec3(-12.0f, 3.5f, 0.0f));

// 프레임 시간 계산용 변수
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 마우스 상태 추적 변수
bool firstMouse = true;
float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;
bool isRightMousePressed = false;

// TODO: [임시 바닥 데이터] 카메라 테스트용입니다.
// 인테리어(Room) 파트가 안정화되면 삭제 예정.
float floorVertices[] = {
     50.0f, -0.5f,  50.0f,
    -50.0f, -0.5f,  50.0f,
    -50.0f, -0.5f, -50.0f,

     50.0f, -0.5f,  50.0f,
    -50.0f, -0.5f, -50.0f,
     50.0f, -0.5f, -50.0f
};


// ==========================================
// [2. 콜백 및 입력 처리 함수]
// ==========================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            isRightMousePressed = true;
            firstMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else if (action == GLFW_RELEASE) {
            isRightMousePressed = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

// 마우스 스크롤 콜백 (줌)
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!isRightMousePressed) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    /* 자유이동 봉인
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    */
}


// ==========================================
// [3. 메인 함수]
// ==========================================
int main() {
    // 1. GLFW 초기화
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. 창 생성
    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Mini Gallery - Jupiter", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 3. GLAD 초기화
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 4. OpenGL 전역 설정
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);   // ⭐ 추가: 안에서 보는 박스라서 양면 그려야 함

    // 5. 셰이더 및 오브젝트 세팅
    Shader ourShader("shaders/basic.vert", "shaders/basic.frag");   // 임시 바닥용
    Shader roomShader("shaders/room.vert", "shaders/room.frag");    // ⭐ 추가: Room 전용
    Room room(40.0f, 12.0f, 11.0f);    // width=30(가로 길게), depth=12, height=8
    Staircase stairs;

    // 원형 플라네타륨 방 생성 (중심 X=65, Z=0, 반지름 25, 천장 20)
    PlanetariumDome dome(44.33f, 0.0f, 25.0f, 20.0f);

    InfoPanel infoPanel(-7.0f, 3.0f, 2.0f);

    InfoPanel jupiterPanel(24.0f, 0.0f, 0.0f, "textures/jupiterInfo.jpg", 90.0f);
    InfoPanel ioPanel(59.9f, 15.6f, 0.0f, "textures/ioInfo.jpg", -135.0f);
    InfoPanel europaPanel(28.8f, 15.6f, 0.0f, "textures/europaInfo.jpg", 135.0f);
    InfoPanel callistoPanel(28.8f, -15.6f, 0.0f, "textures/CallistoInfo.jpg", 45.0f);
    InfoPanel ganymedePanel(59.9f, -15.6f, 0.0f, "textures/ganymedeInfo.jpg", -45.0f);



    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);

    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 걸을 수 있는 영역 = 방 안 또는 돔 안
    auto isWalkable = [&](float x, float z) {
        const float MARGIN = 0.5f;
        return room.Contains(x, z, MARGIN) || dome.Contains(x, z, MARGIN);
        };

    // 목성 생성
    JupiterSystem jupiterSys;

    // 6. 렌더 루프
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glm::vec3 oldPos = camera.Position;
        processInput(window);

        // ⭐ 벽 충돌: 축을 분리해서 검사 (벽 따라 미끄러짐)
        glm::vec3 p = camera.Position;
        if (!isWalkable(p.x, oldPos.z)) p.x = oldPos.x;   // x 막히면 x만 취소
        if (!isWalkable(p.x, p.z))      p.z = oldPos.z;   // z 막히면 z만 취소
        camera.Position.x = p.x;
        camera.Position.z = p.z;

        float floorH = stairs.GetFloorHeightAt(camera.Position.x, camera.Position.z);
        camera.StickToFloor(floorH);

        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);   // 어두운 우주 배경
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 공통 행렬 계산
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // ===== 1. Room과 돔(공간) 그리기 =====
        roomShader.use();
        roomShader.setMat4("projection", projection);
        roomShader.setMat4("view", view);
        roomShader.setVec3("lightPos", glm::vec3(65.0f, 15.0f, 0.0f));      // 천장 중앙 광원
        roomShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));    // 흰빛
        roomShader.setVec3("viewPos", camera.Position);
        
        room.Draw(roomShader);
        stairs.Draw(roomShader);
        dome.Draw(roomShader);

        infoPanel.Draw(roomShader);

        jupiterPanel.Draw(roomShader);
        ioPanel.Draw(roomShader);
        europaPanel.Draw(roomShader);
        callistoPanel.Draw(roomShader);
        ganymedePanel.Draw(roomShader);

        // ===== 2. 목성계 그리기 =====
        float time = static_cast<float>(glfwGetTime());
        
        // ⭐ 목성 전용 셰이더(ourShader) 활성화 및 카메라 정보 전달 (순서가 매우 중요!)
        ourShader.use(); 
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        // (조명 모델 행렬 등은 JupiterSystem.cpp 안에서 알아서 처리됩니다)

        // 좌표 수정
        jupiterSys.Draw(ourShader, camera, time, glm::vec3(44.33f, 5.0f, 0.0f));

        // 버퍼 교체 및 이벤트 처리
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 7. 종료
    glDeleteVertexArrays(1, &floorVAO);
    glDeleteBuffers(1, &floorVBO);
    glfwTerminate();
    return 0;
}
