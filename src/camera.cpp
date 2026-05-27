#include "Camera.h"

// 생성자 구현
Camera::Camera(glm::vec3 position)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED),
    MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
    Position = position;
    WorldUp = glm::vec3(0.0f, 1.0f, 0.0f); // 보통 월드의 하늘은 항상 y축(0,1,0)
    Yaw = YAW;
    Pitch = PITCH;
    updateCameraVectors(); // 초기 벡터 계산
}

// View 행렬 반환
glm::mat4 Camera::GetViewMatrix() {
    // 현재 위치에서, 내 위치 + 바라보는 방향(Front)을 향해 렌즈를 맞춤
    return glm::lookAt(Position, Position + Front, Up);
}

// 키보드 이동 로직 (W, A, S, D)
// 키보드 이동 로직 (W, A, S, D, Q, E)
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;

    // --- [여기에 위아래 이동 추가] ---
    if (direction == UP)
        Position += Up * velocity;    // Up 벡터 방향으로 이동 (상승)
    if (direction == DOWN)
        Position -= Up * velocity;    // Up 벡터 반대 방향으로 이동 (하강)
    // ---------------------------------
}

// 마우스 시선 변경 로직
void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // 시선이 위아래로 90도를 넘어가지 않도록 제한 (목 꺾임 방지)
    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // 각도가 변했으니 벡터들을 다시 계산
    updateCameraVectors();
}

// 마우스 스크롤 처리 - 시야각 변경 (망원경 줌 효과)
void Camera::ProcessMouseScroll(float yoffset) {
    Zoom -= yoffset;
    if (Zoom < 1.0f)   Zoom = 1.0f;      // 너무 좁아지지 않게
    if (Zoom > 45.0f)  Zoom = 45.0f;     // 기본값보다 넓어지지 않게
}

// 삼각함수(cos, sin)를 이용해 3D 시선의 방향 벡터 계산
void Camera::updateCameraVectors() {
    glm::vec3 front;
    // 호도법(Radians)으로 변환 후 삼각함수 적용
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

    // 계산된 방향을 정규화(길이를 1로 만듦)하여 Front 벡터 갱신
    Front = glm::normalize(front);

    // Front와 WorldUp을 외적(Cross Product)하여 Right 벡터 도출
    Right = glm::normalize(glm::cross(Front, WorldUp));

    // Right와 Front를 외적하여 진정한 Up 벡터 도출
    Up = glm::normalize(glm::cross(Right, Front));
}