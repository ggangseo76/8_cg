#version 330 core

out vec4 FragColor;

// Vertex Shader에서 넘어온 값
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// 텍스처
uniform sampler2D texture1;

// 목성계 조명
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform float emissiveStrength;

void main()
{
    vec3 objectColor = texture(texture1, TexCoord).rgb;
    vec3 norm = normalize(Normal);

    // 전체적으로 너무 어둡지 않게 하는 기본광
    vec3 ambient = ambientStrength * objectColor;

    // 방향성 조명
    vec3 dir = normalize(-lightDir);
    float diff = max(dot(norm, dir), 0.0);
    vec3 diffuse = diff * lightColor * objectColor;

    // 자체 발광 느낌
    // 실제 광원처럼 빛을 내는 건 아니지만, 어두운 배경 속에서 전시물이 살아 보이게 함
    vec3 emissive = emissiveStrength * objectColor;

    vec3 result = ambient + diffuse + emissive;

    // 살짝 밝게 보정
    result = result * 1.08;

    // 과노출 방지
    result = clamp(result, 0.0, 1.0);

    FragColor = vec4(result, 1.0);
}