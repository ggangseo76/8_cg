#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Camera.h"
#include "shader.h"
#include "Room.h"
#include "Staircase.h"
#include "InfoPanel.h"

// ==========================================
// [1. 전역 변수 및 설정]
// ==========================================
// 카메라 객체 (초기 위치 z = 3.0f)
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

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
    InfoPanel infoPanel(-7.0f, 3.0f, 2.0f);   // x=-7, z=0, 안내방 평지(y=2.0) 위

    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);

    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 6. 렌더 루프
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        // ⭐ 카메라가 선 자리의 바닥 높이를 구해서 y를 거기 맞춤
        float floorH = stairs.GetFloorHeightAt(camera.Position.x, camera.Position.z);
        camera.StickToFloor(floorH);

        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);   // 어두운 우주 배경
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 공통 행렬 계산
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // ===== Room 그리기 =====
        roomShader.use();
        roomShader.setMat4("projection", projection);
        roomShader.setMat4("view", view);
        roomShader.setVec3("lightPos", glm::vec3(0.0f, 9.0f, 0.0f));      // 천장 중앙 광원
        roomShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));    // 흰빛
        roomShader.setVec3("viewPos", camera.Position);
        room.Draw(roomShader);
        stairs.Draw(roomShader);   // ⭐ 계단도 같은 셰이더로 그림
        infoPanel.Draw(roomShader);

        // ===== 임시 바닥 (격자) 그리기 =====
        // 통합 시 제거 예정
        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        glm::mat4 model = glm::mat4(1.0f);
        ourShader.setMat4("model", model);

        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 7. 종료
    glDeleteVertexArrays(1, &floorVAO);
    glDeleteBuffers(1, &floorVBO);
    glfwTerminate();
    return 0;
}