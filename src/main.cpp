#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

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

// 우클릭을 누르고 있을 때만 시점 회전 가능
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

// 마우스 스크롤 콜백: 줌 인/아웃
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// 마우스 이동 콜백: 우클릭 중일 때만 고개 회전
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

// 키보드 입력 처리
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 테스트용 질주 키
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.MovementSpeed = 7.5f;
    else
        camera.MovementSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    /*
    // 자유이동 봉인
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

    // 콜백 등록
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

    // 전시관 내부에서 벽과 돔을 보기 때문에 양면 렌더링
    glDisable(GL_CULL_FACE);

    // 5. 셰이더 및 오브젝트 세팅
    Shader ourShader("shaders/basic.vert", "shaders/basic.frag");          // 임시 바닥용
    Shader roomShader("shaders/room.vert", "shaders/room.frag");           // 전시관 공간용
    Shader jupiterShader("shaders/jupiter.vert", "shaders/jupiter.frag");  // 목성계 전용

    Room room(40.0f, 12.0f, 11.0f);
    Staircase stairs;

    // 원형 플라네타륨 방 생성
    // 중심 X=44.33, Z=0, 반지름=25, 천장 높이=20
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

    struct PanelObstacle {
        float x;
        float z;
        float radius;
    };

    std::vector<PanelObstacle> panelObstacles = {
        { -7.0f,   3.0f, 0.8f },   // 안내방 안내판
        { 24.0f,   0.0f, 0.8f },   // 목성 안내판
        { 59.9f,  15.6f, 0.8f },   // 이오 안내판
        { 28.8f,  15.6f, 0.8f },   // 유로파 안내판
        { 28.8f, -15.6f, 0.8f },   // 칼리스토 안내판
        { 59.9f, -15.6f, 0.8f },   // 가니메데 안내판
    };

    // 걸을 수 있는 영역 = 방 안 또는 돔 안
    auto isWalkable = [&](float x, float z) {
        const float MARGIN = 0.5f;

        // 방/돔 안인지 확인
        if (!(room.Contains(x, z, MARGIN) || dome.Contains(x, z, MARGIN)))
            return false;

        // 패널 반경 안이면 이동 제한
        for (const auto& p : panelObstacles) {
            float dx = x - p.x;
            float dz = z - p.z;

            if (dx * dx + dz * dz < p.radius * p.radius)
                return false;
        }

        return true;
        };

    // 목성계 생성
    JupiterSystem jupiterSys;

    // 6. 렌더 루프
    while (!glfwWindowShouldClose(window)) {
        // 프레임 시간 계산
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 입력 처리 전 위치 저장
        glm::vec3 oldPos = camera.Position;
        processInput(window);

        // 벽 충돌 처리: 축을 분리해서 검사
        // 막히는 축만 취소해서 벽을 따라 미끄러지는 느낌을 줌
        glm::vec3 p = camera.Position;

        if (!isWalkable(p.x, oldPos.z))
            p.x = oldPos.x;

        if (!isWalkable(p.x, p.z))
            p.z = oldPos.z;

        camera.Position.x = p.x;
        camera.Position.z = p.z;

        // 계단/바닥 높이에 맞게 카메라 높이 조정
        float floorH = stairs.GetFloorHeightAt(camera.Position.x, camera.Position.z);
        camera.StickToFloor(floorH);

        // 배경색: 어두운 우주 전시관 분위기
        glClearColor(0.025f, 0.025f, 0.055f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 공통 행렬 계산
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom),
            800.0f / 600.0f,
            0.1f,
            100.0f
        );

        glm::mat4 view = camera.GetViewMatrix();

        // ==========================================
        // [1] Room, 계단, 돔, 안내판 그리기
        // ==========================================

        roomShader.use();

        roomShader.setMat4("projection", projection);
        roomShader.setMat4("view", view);
        roomShader.setVec3("viewPos", camera.Position);

        // 전시관 전체 조도
        // 전체를 밝히기보다 어둡게 유지해서 우주 박물관 분위기를 만듦.
        roomShader.setFloat("ambientStrength", 0.055f);

        // 들어오는 길 조명
        // 입구와 이동 동선은 약간 따뜻한 색으로 밝혀서 관람 경로를 안내하는 느낌을 줌.
        roomShader.setVec3("entranceLightPos", glm::vec3(-8.0f, 5.0f, 0.0f));
        roomShader.setVec3("entranceLightColor", glm::vec3(0.95f, 0.72f, 0.45f));

        // 돔 내부 조명
        // 목성 전시 공간은 차갑고 약한 푸른빛으로 우주 분위기를 강조함.
        roomShader.setVec3("domeLightPos", glm::vec3(44.33f, 14.0f, 0.0f));
        roomShader.setVec3("domeLightColor", glm::vec3(0.18f, 0.24f, 0.48f));

        // 안내판 간접등
        // 주의: glm::vec3(x, y, z) 순서임.
        // InfoPanel 위치값을 기준으로 x/z 위치를 맞추고, y는 안내판을 비추는 높이로 설정함.
        roomShader.setVec3("panelLightPositions[0]", glm::vec3(-7.0f, 3.0f, 3.0f));      // 안내방 안내판
        roomShader.setVec3("panelLightPositions[1]", glm::vec3(24.0f, 2.4f, 0.0f));      // 목성 안내판
        roomShader.setVec3("panelLightPositions[2]", glm::vec3(59.9f, 2.4f, 15.6f));     // 이오 안내판
        roomShader.setVec3("panelLightPositions[3]", glm::vec3(28.8f, 2.4f, 15.6f));     // 유로파 안내판
        roomShader.setVec3("panelLightPositions[4]", glm::vec3(28.8f, 2.4f, -15.6f));    // 칼리스토 안내판
        roomShader.setVec3("panelLightPositions[5]", glm::vec3(59.9f, 2.4f, -15.6f));    // 가니메데 안내판

        // 따뜻한 간접등 색상
        roomShader.setVec3("panelLightColor", glm::vec3(1.0f, 0.82f, 0.52f));

        // 안내판 조명 강도
        roomShader.setFloat("panelLightStrength", 1.15f);

        room.Draw(roomShader);
        stairs.Draw(roomShader);
        dome.Draw(roomShader);

        infoPanel.Draw(roomShader);

        jupiterPanel.Draw(roomShader);
        ioPanel.Draw(roomShader);
        europaPanel.Draw(roomShader);
        callistoPanel.Draw(roomShader);
        ganymedePanel.Draw(roomShader);

        // ==========================================
        // [2] 목성계 그리기
        // ==========================================

        float time = static_cast<float>(glfwGetTime());

        jupiterShader.use();

        jupiterShader.setMat4("projection", projection);
        jupiterShader.setMat4("view", view);

        // 목성/위성 조명
        // 직접광은 약하게 두고, 자체 발광 느낌을 더해서 어두운 전시관 안에서 빛나는 전시물처럼 보이게 함.
        jupiterShader.setVec3("lightDir", glm::vec3(-0.2f, -0.7f, -0.25f));
        jupiterShader.setVec3("lightColor", glm::vec3(1.05f, 1.03f, 0.95f));

        // 주변광은 낮게 유지
        jupiterShader.setFloat("ambientStrength", 0.12f);

        // 자체 발광 느낌
        jupiterShader.setFloat("emissiveStrength", 0.35f);

        jupiterSys.Draw(
            jupiterShader,
            camera,
            time,
            glm::vec3(44.33f, 5.0f, 0.0f)
        );

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