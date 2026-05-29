#version 330 core
out vec4 FragColor;

// Vertex Shader에서 넘어온 값
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// CPU에서 받는 값
uniform sampler2D texture1;   // 텍스처
uniform vec3 lightPos;        // 광원 위치
uniform vec3 lightColor;      // 광원 색
uniform vec3 viewPos;         // 카메라 위치 (specular 추가할 때 씀)

void main() {
    // 1. 텍스처 색
    vec3 objectColor = texture(texture1, TexCoord).rgb;
    
    // 2. Ambient (전역광) - 어디든 기본 30% 밝기
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
    
    // 3. Diffuse (면이 빛 받는 정도)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // 4. 최종 색 = (밝기 비율) × 텍스처 색
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}

