#version 330 core

out vec4 FragColor;

// Vertex Shader에서 넘어온 값
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// 텍스처
uniform sampler2D texture1;

// 카메라 위치
uniform vec3 viewPos;

// 전체 어두운 전시관 분위기
uniform float ambientStrength;

// 입구 조명
uniform vec3 entranceLightPos;
uniform vec3 entranceLightColor;

// 돔 내부 조명
uniform vec3 domeLightPos;
uniform vec3 domeLightColor;

// 안내판 간접등
uniform vec3 panelLightPositions[6];
uniform vec3 panelLightColor;
uniform float panelLightStrength;

// 포인트 조명 계산 함수
vec3 CalcPointLight(vec3 lightPos, vec3 lightColor, float strength, vec3 norm, vec3 fragPos)
{
    vec3 lightDir = normalize(lightPos - fragPos);

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);

    // 거리 감쇠
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (1.0 + 0.12 * distance + 0.035 * distance * distance);

    // 은은한 Specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 24.0);

    vec3 diffuse = diff * lightColor * strength;
    vec3 specular = spec * lightColor * 0.18 * strength;

    return (diffuse + specular) * attenuation;
}

void main()
{
    vec3 objectColor = texture(texture1, TexCoord).rgb;
    vec3 norm = normalize(Normal);

    // 전체 전시관은 어둡게 유지
    vec3 ambient = ambientStrength * objectColor;

    // 들어오는 길 쪽은 따뜻한 조명
    vec3 entranceLight = CalcPointLight(
        entranceLightPos,
        entranceLightColor,
        1.25,
        norm,
        FragPos
    );

    // 돔 내부는 차갑고 약한 우주빛
    vec3 domeLight = CalcPointLight(
        domeLightPos,
        domeLightColor,
        0.85,
        norm,
        FragPos
    );

    // 안내판 주변 간접등
    vec3 panelLight = vec3(0.0);

    for (int i = 0; i < 6; i++)
    {
        panelLight += CalcPointLight(
            panelLightPositions[i],
            panelLightColor,
            panelLightStrength,
            norm,
            FragPos
        );
    }

    vec3 lighting = ambient + entranceLight + domeLight + panelLight;

    vec3 result = lighting * objectColor;

    // 너무 과하게 타는 것 방지
    result = clamp(result, 0.0, 1.0);

    FragColor = vec4(result, 1.0);
}