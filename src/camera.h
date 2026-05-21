#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include "../Dependencies/include/glm/glm.hpp"
#include "../Dependencies/include/glm/gtc/matrix_transform.hpp"

// 카메라 이동 방향을 정의하는 열거형
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,     // 새로 추가
    DOWN    // 새로 추가
};

// 카메라 기본 설정값
const float YAW = -90.0f; // 초기 좌우 시선 (y축 기준 회전)
const float PITCH = 0.0f;  // 초기 상하 시선 (x축 기준 회전)
const float SPEED = 2.5f;  // 이동 속도
const float SENSITIVITY = 0.1f;  // 마우스 민감도

class Camera {
public:
    // 카메라 속성 벡터
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // 카메라 각도 (Euler Angles)
    float Yaw;
    float Pitch;

    // 카메라 옵션
    float MovementSpeed;
    float MouseSensitivity;

    // 생성자 (위치값만 받아 초기화)
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f));

    // 쉐이더에 전달할 View 행렬을 반환하는 함수 (핵심!)
    glm::mat4 GetViewMatrix();

    // 키보드 입력을 처리하여 위치(Position) 변경
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    // 마우스 입력을 처리하여 시선(Yaw, Pitch) 변경
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

private:
    // Yaw, Pitch 각도를 바탕으로 Front, Right, Up 벡터를 다시 계산하는 함수
    void updateCameraVectors();
};

#endif
