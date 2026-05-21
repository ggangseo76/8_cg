#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Camera.h"
#include "shader.h"

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
bool isRightMousePressed = false; // 우클릭 상태 확인

// TODO: [임시 바닥 데이터] 카메라 테스트용입니다. 
// 나중에 인테리어(건물 내부) 파트 코드가 합쳐지면 삭제해도 됩니다.
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
// 창 크기 변경 콜백
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// 마우스 클릭 상태 감지 콜백 (우클릭 시점 변환용)
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            isRightMousePressed = true;
            firstMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // 마우스 숨김
        }
        else if (action == GLFW_RELEASE) {
            isRightMousePressed = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);   // 마우스 표시
        }
    }
}

// 마우스 움직임 콜백 (시점 변환)
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    // 우클릭 중이 아닐 때는 무시
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

// 키보드 입력 처리 (이동 및 종료)
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // W, A, S, D 이동
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // Q(하강), E(상승) 이동
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}


// ==========================================
// [3. 메인 함수 (프로그램 진입점)]
// ==========================================
int main() {
    // 1. GLFW 초기화 및 설정
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. 창 생성
    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Mini Gallery", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // 컨텍스트 및 콜백 설정
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // 3. GLAD 초기화
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 4. OpenGL 전역 설정 (깊이 테스트)
    glEnable(GL_DEPTH_TEST);

    // 5. 쉐이더 및 오브젝트 세팅
    Shader ourShader("shaders/basic.vert", "shaders/basic.frag");

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
        // 프레임 시간 계산
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 입력 처리
        processInput(window);

        // 화면 지우기
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 쉐이더 활성화 및 카메라/투영 행렬 설정
        ourShader.use();

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        ourShader.setMat4("model", model);

        // 바닥 그리기
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 버퍼 교체 및 이벤트 처리
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 7. 자원 정리 및 종료
    glfwTerminate();
    return 0;
}