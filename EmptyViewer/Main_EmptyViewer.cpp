// main_Phong_Shader.cpp
#include <Windows.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>

#define M_PI 3.14159265358979323846

using namespace glm;


// --- Geometry ---
struct Vertex {
    vec3 position;
    vec3 normal;
};

std::vector<Vertex> vertices;
std::vector<unsigned int> indices;

GLuint shaderProgram, VAO, VBO, EBO;

GLuint compile_shader(const char* src, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, NULL, log);
        std::cerr << "Shader Compile Error: " << log << std::endl;
    }
    return shader;
}

std::string read_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
        exit(EXIT_FAILURE);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


void init_shader() {
    std::string vsCode = read_file("phong.vert");
    std::string fsCode = read_file("phong.frag");

    GLuint vs = compile_shader(vsCode.c_str(), GL_VERTEX_SHADER);
    GLuint fs = compile_shader(fsCode.c_str(), GL_FRAGMENT_SHADER);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);
}


// --- Sphere Generator ---
void create_sphere(unsigned int width = 32, unsigned int height = 16) {
    for (unsigned int j = 1; j < height - 1; ++j) {
        for (unsigned int i = 0; i < width; ++i) {
            float theta = float(j) / (height - 1) * M_PI;
            float phi = float(i) / width * 2.0f * M_PI;
            float x = sinf(theta) * cosf(phi);
            float y = cosf(theta);
            float z = -sinf(theta) * sinf(phi);
            vec3 pos = vec3(x, y, z);
            vertices.push_back({ pos, normalize(pos) });
        }
    }
    vertices.push_back({ vec3(0, 1, 0), vec3(0, 1, 0) });
    vertices.push_back({ vec3(0, -1, 0), vec3(0, -1, 0) });

    for (unsigned int j = 0; j < height - 3; ++j) {
        for (unsigned int i = 0; i < width; ++i) {
            unsigned int idx0 = j * width + i;
            unsigned int idx1 = (j + 1) * width + i;
            unsigned int idx2 = j * width + (i + 1) % width;
            unsigned int idx3 = (j + 1) * width + (i + 1) % width;
            indices.insert(indices.end(), { idx0, idx3, idx2, idx0, idx1, idx3 });
        }
    }
    unsigned int top = static_cast<unsigned int>(vertices.size()) - 2;
    for (unsigned int i = 0; i < width; ++i)
        indices.insert(indices.end(), { top, i, (i + 1) % width });

    unsigned int bottom = static_cast<unsigned int>(vertices.size()) - 1;
    unsigned int base = (height - 3) * width;
    for (unsigned int i = 0; i < width; ++i)
        indices.insert(indices.end(), { bottom, base + (i + 1) % width, base + i });
}

void init_buffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    mat4 modeling = translate(mat4(1.0f), vec3(0, 0, -7)) * scale(mat4(1.0f), vec3(2.0f));
    mat4 view = lookAt(vec3(0, 0, 0), vec3(0, 0, -1), vec3(0, 1, 0));
    float l = -0.1f, r = 0.1f, b = -0.1f, t = 0.1f, n = 0.1f, f = 1000.0f;
    mat4 projection(0.0f);
    projection[0][0] = 2 * n / (r - l);
    projection[1][1] = 2 * n / (t - b);
    projection[2][0] = (r + l) / (r - l);
    projection[2][1] = (t + b) / (t - b);
    projection[2][2] = -(f + n) / (f - n);
    projection[2][3] = -1.0f;
    projection[3][2] = -2 * f * n / (f - n);

    mat4 modeling_inv_tr = transpose(inverse(modeling));

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modeling"), 1, GL_FALSE, value_ptr(modeling));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "camera"), 1, GL_FALSE, value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modeling_inv_tr"), 1, GL_FALSE, value_ptr(modeling_inv_tr));

    glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), -4.0f, 4.0f, -3.0f);
    glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), 0.0f, 0.0f, 0.0f);
    glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutCreateWindow("Phong Shader Sphere");
    glewInit();

    glEnable(GL_DEPTH_TEST);
    create_sphere();
    init_shader();
    init_buffers();

    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
