#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

// 창 크기가 변경될 때마다 호출되는 콜백 함수
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // 렌더링될 뷰포트의 크기를 조정
    glViewport(0, 0, width, height);
}

// 키보드 입력을 처리하는 함수 (ESC 누르면 종료)
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    // 1. GLFW 초기화 및 설정
    glfwInit();
    // 사용할 OpenGL 버전을 3.3으로 명시
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Core Profile 사용 (구버전 함수 사용 불가)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. 창 생성 (너비 800, 높이 600)
    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Mini Gallery", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 현재 스레드의 주 컨텍스트로 설정
    glfwMakeContextCurrent(window);
    // 창 크기 변경 콜백 함수 등록
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 3. GLAD 초기화 (OpenGL 함수 포인터 로드)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 4. 3D 그래픽스를 위한 깊이 테스트 활성화 (중요!)
    glEnable(GL_DEPTH_TEST);

    // 5. 렌더 루프 (Render Loop)
    // 창이 닫히라는 명령이 있기 전까지 계속 반복
    while (!glfwWindowShouldClose(window)) {
        // 입력 처리
        processInput(window);

        // 렌더링 명령 (배경색 지정: 짙은 청록색)
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // 컬러 버퍼와 깊이 버퍼(3D)를 동시에 지움
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // (이곳에 전시관 바닥, 3D 모델, 카메라 등의 렌더링 코드가 들어갈 예정입니다)

        // 이벤트를 확인하고 버퍼를 교체 (Double Buffering)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 6. 종료 전 할당된 자원 정리
    glfwTerminate();
    return 0;
}