#version 330 core
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D diffuse0;
uniform vec3  lightDir;        // 빛이 "진행하는" 방향
uniform vec3  lightColor;
uniform float ambientStrength; // 어두운 면이 완전히 까매지지 않게

void main() {
    vec3 tex = texture(diffuse0, TexCoord).rgb;

    vec3 N = normalize(Normal);
    vec3 L = normalize(-lightDir);     // 빛을 향하는 방향
    float diff = max(dot(N, L), 0.0);

    vec3 ambient = ambientStrength * lightColor;
    vec3 diffuse = diff * lightColor;
    FragColor = vec4((ambient + diffuse) * tex, 1.0);
}