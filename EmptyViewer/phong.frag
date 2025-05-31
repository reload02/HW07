#version 330 core
in vec3 fragPos;
in vec3 fragNormal;
out vec4 FragColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float p;
uniform float Ia;
uniform vec3 lightColor;
void main() {

    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    vec3 ambient = ka * Ia;
    vec3 diffuse = kd * max(dot(norm, lightDir), 0.0);
    vec3 specular = ks * pow(max(dot(viewDir, reflectDir), 0.0), p);

    vec3 result = (ambient + diffuse + specular) * lightColor;
    vec3 gammaCorrected = pow(clamp(result, 0.0, 1.0), vec3(1.0 / 2.2));
    FragColor = vec4(gammaCorrected, 1.0);
}
