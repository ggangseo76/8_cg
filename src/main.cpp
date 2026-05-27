#define GLM_ENABLE_EXPERIMENTAL
#include <filesystem>
namespace fs = std::filesystem;
#include "opengl.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Camera.h"
#include "shader.h"

const unsigned int width = 1600;
const unsigned int height = 800;



typedef struct Planet
{
    const char* name;

    // 행성 자체 정보
    float size;                 // 행성 크기
    glm::mat4 axialTilt;        // 자전축
    float rotationSpeed;        // 자전 속도
    float rotationPhase;        // 자전 초기 각도

    // 공전 정보
    float orbitRadius;          // 공전 궤도 반지름
    glm::mat4 orbitRotation;    // 공전 궤도 회전/기울기
    float orbitSpeed;           // 공전 속도
    float orbitPhase;           // 공전 초기 각도

    // 선택 정보
    glm::vec4 orbitColor;       // 궤도 색상

    // 텍스처 매핑 정보 (추가됨)
    const char* diffuseMap;     // 디퓨즈(컬러) 맵 파일명
    const char* specularMap;    // 스펙큘러(반사) 맵 파일명
} Planet;

//행성 계산 헬퍼 함수들

glm::mat4 T(const glm::vec3& v) { return glm::translate(glm::mat4(1.0f), v); }
glm::mat4 S(const glm::vec3& v) { return glm::scale(glm::mat4(1.0f), v); }
glm::mat4 S(float s) { return S(glm::vec3(s)); }
glm::mat4 R(float angleRadians, const glm::vec3& axis) { return glm::rotate(glm::mat4(1.0f), angleRadians, axis); }
glm::mat4 R_x(float angleRadians) { return R(angleRadians, glm::vec3(1.0f, 0.0f, 0.0f)); }
glm::mat4 R_y(float angleRadians) { return R(angleRadians, glm::vec3(0.0f, 1.0f, 0.0f)); }
glm::mat4 R_z(float angleRadians) { return R(angleRadians, glm::vec3(0.0f, 0.0f, 1.0f)); }

glm::mat4 makeAxialTiltX(float degree) { return R_z(glm::radians(-degree)); }


glm::vec3 makeOrbitPosition(const Planet& planet, float time)
{
    float orbitAngle = time * planet.orbitSpeed + planet.orbitPhase;
    glm::vec4 position = planet.orbitRotation * R_y(orbitAngle) * glm::vec4(planet.orbitRadius, 0.0f, 0.0f, 1.0f);
    return glm::vec3(position);
}

glm::mat4 makePlanetModel(const Planet& planet, float time)
{
    float rotationAngle = time * planet.rotationSpeed + planet.rotationPhase;
    glm::vec3 orbitPosition = makeOrbitPosition(planet, time);

    return T(orbitPosition) * planet.orbitRotation * planet.axialTilt * R_y(rotationAngle) * S(planet.size);
}

glm::mat4 makeOrbitModel(const Planet& planet)
{
    return planet.orbitRotation * S(planet.orbitRadius);
}

void makeSphere(float radius, int thetaCount, int phiCount, std::vector<Vertex>& vertices, std::vector<GLuint>& indices)
{
    const float PI = 3.14159265359f;
    vertices.clear();
    indices.clear();

    for (int i = 0; i <= phiCount; ++i) {
        float phi = (float)i / phiCount * PI;
        for (int j = 0; j <= thetaCount; ++j) {
            float theta = (float)j / thetaCount * 2.0f * PI;

            glm::vec3 pos(radius * sinf(phi) * cosf(theta), radius * cosf(phi), radius * sinf(phi) * sinf(theta));
            glm::vec3 normal = glm::normalize(pos);

            Vertex vertex;
            vertex.position = pos;
            vertex.normal = normal;
            vertex.color = glm::vec3(1.0f);
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
}

void makeOrbit(float radius, int segmentCount, std::vector<Vertex>& vertices, std::vector<GLuint>& indices)
{
    const float PI = 3.14159265359f;
    vertices.clear();
    indices.clear();

    for (int i = 0; i < segmentCount; ++i) {
        float theta = (float)i / segmentCount * 2.0f * PI;
        Vertex vertex{};
        vertex.position = glm::vec3(radius * cosf(theta), 0.0f, radius * sinf(theta));
        vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        vertex.color = glm::vec3(1.0f);
        vertex.texUV = glm::vec2(0.0f);
        vertices.push_back(vertex);
        indices.push_back(i);
    }
}

void drawWithWireframe(Mesh& mesh, Shader& shader, Camera& camera, const glm::mat4& model)
{
    shader.use(); // 셰이더 활성화 필수

    glUniform1i(glGetUniformLocation(shader.ID, "diffuse0"), 0);
    GLint modelLoc = glGetUniformLocation(shader.ID, "model");
    GLint lightColorLoc = glGetUniformLocation(shader.ID, "lightColor");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Fill
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUniform4f(lightColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
    mesh.Draw(shader, camera);

    // Wireframe
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-1.0f, -1.0f);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1.5f);
    glUniform4f(lightColorLoc, 0.0f, 0.0f, 0.0f, 1.0f);
    mesh.Draw(shader, camera);

    glDisable(GL_POLYGON_OFFSET_LINE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

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
    // 행성 데이터 정의
    const Planet planets[] =
    {
        {
            "Jupiter",
            1.5f, makeAxialTiltX(3.0f), 0.8f, 0.0f,
            0.0f, glm::mat4(1.0f), 0.0f, 0.0f,
            glm::vec4(1.0f, 0.7f, 0.35f, 1.0f),
            "jupiter.jpg", "jupiter.jpg"
        },
        {
            "Io",
            0.25f, makeAxialTiltX(1.0f), 2.4f, 0.0f,
            4.0f, R_x(glm::radians(0.05f)), 1.5f, 0.0f,
            glm::vec4(0.95f, 0.75f, 0.25f, 1.0f),
            "io.jpg", "io.jpg"
        },
        {
            "Europa",
            0.22f, makeAxialTiltX(2.0f), 1.8f, 0.0f,
            6.36f, R_x(glm::radians(0.47f)), 1.0f, 1.7f,
            glm::vec4(0.75f, 0.9f, 1.0f, 1.0f),
            "europa.jpg", "europa.jpg"
        },
        {
            "Ganymede",
            0.35f, makeAxialTiltX(3.0f), 1.2f, 0.0f,
            10.15f, R_z(glm::radians(0.20f)), 0.6f, 3.2f,
            glm::vec4(0.65f, 0.65f, 0.65f, 1.0f),
            "ganymede.jpg", "ganymede.jpg"
        },
        {
            "Callisto",
            0.35f, makeAxialTiltX(3.0f), 1.2f, 0.0f,
            17.86f, R_z(glm::radians(0.20f)), 0.6f, 3.2f,
            glm::vec4(0.65f, 0.65f, 0.65f, 1.0f),
            "callisto.jpg", "callisto.jpg"
        }
    };

    //정점 데이터
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    makeSphere(1.0f, 32, 32, vertices, indices); 

    // ── 행성별 텍스처 로드 및 메쉬 생성 ─────────────────────────────────
    std::string imageFolderPath = fs::current_path().string();
    std::string currentDir = imageFolderPath + "\\opengl_images";

    std::vector<Mesh> planetMeshes;
    int numPlanets = sizeof(planets) / sizeof(planets[0]);
    
    // 1. 텍스처를 담아둘 보관함
    std::vector<std::vector<Texture>> planetTextureStorage(numPlanets);

    for (int i = 0; i < numPlanets; i++)
    {
        // 2. 보관함(planetTextureStorage[i])에 직접 텍스처를 push_back 합니다.
        planetTextureStorage[i].push_back(Texture((currentDir + "\\" + planets[i].diffuseMap).c_str(), "diffuse", 0, GL_RGB, GL_UNSIGNED_BYTE));
        planetTextureStorage[i].push_back(Texture((currentDir + "\\" + planets[i].specularMap).c_str(), "specular", 1, GL_RGB, GL_UNSIGNED_BYTE));

        // 3. 메쉬 생성 시 보관함의 참조를 전달합니다.
        planetMeshes.push_back(Mesh(vertices, indices, planetTextureStorage[i]));
    }
    // ── 궤도 선 메쉬 생성 ────────────────────────────────────────────────
    std::vector<Texture> noTextures;
    std::vector<Vertex> lineVertices;
    std::vector<GLuint> lineIndices;
    makeOrbit(1.0f, 128, lineVertices, lineIndices);
    Mesh orbitMesh(lineVertices, lineIndices, noTextures);


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
        //행성과 궤도 그리기
        float time = static_cast<float>(glfwGetTime());
        
        for (int i = 0; i < numPlanets; i++)
        {
            Planet planet = planets[i];

            //행성계를 y축으로 2.0만큼 들어올리는 행렬
            glm::mat4 systemOffset = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));

            glm::mat4 OrbitModel = systemOffset * makeOrbitModel(planet);
            glm::mat4 PlanetModel = systemOffset * makePlanetModel(planet, time);

            // 궤도 그리기
            drawWithWireframe(orbitMesh, ourShader, camera, OrbitModel);

            // 해당 행성 메쉬 그리기 (각자 다른 텍스처가 바인딩됨)
            drawWithWireframe(planetMeshes[i], ourShader, camera, PlanetModel);
        }

        // 버퍼 교체 및 이벤트 처리
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 7. 자원 정리 및 종료
    glfwTerminate();
    return 0;
}