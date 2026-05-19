#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SHADER.h"
#include "CAMERA.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <STB_IMAGE/stb_image_write.h>

#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <string>

// 窗口常量
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// 固定相机：俯视 45°，距离约 15 单位，可以看到整个平面
Camera camera(glm::vec3(0.0f, 8.0f, 8.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -35.0f);

// 网格分辨率
const int GRID_SIZE = 200;
const float PLANE_SCALE = 10.0f;

// 生成网格顶点（仅位置 xz，高度在着色器中计算）
std::vector<float> generatePlaneVertices(int N, float scale) {
    std::vector<float> vertices;
    float step = (2.0f * scale) / (N - 1);
    for (int i = 0; i < N; ++i) {
        float z = -scale + i * step;
        for (int j = 0; j < N; ++j) {
            float x = -scale + j * step;
            vertices.push_back(x);
            vertices.push_back(0.0f);  // y
            vertices.push_back(z);
        }
    }
    return vertices;
}

// 生成三角形条带索引
std::vector<unsigned int> generatePlaneIndices(int N) {
    std::vector<unsigned int> indices;
    for (int i = 0; i < N - 1; ++i) {
        for (int j = 0; j < N; ++j) {
            indices.push_back(i * N + j);
            indices.push_back((i + 1) * N + j);
        }
        if (i < N - 2) {
            indices.push_back((i + 1) * N + (N - 1));
            indices.push_back((i + 1) * N + 0);
        }
    }
    return indices;
}

// 实验参数结构
struct ParamSet {
    int octaves;
    float lacunarity;
    float persistence;
    std::string name;
};

int main() {
    // ====== 初始化窗口 ======
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "fBm Water Experiment", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // 关闭垂直同步，避免测量干扰

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // ====== 编译着色器 ======
    Shader waterShader("resources/shaders/water.vs", "resources/shaders/water.fs");

    // ====== 创建水面网格 ======
    std::vector<float> vertices = generatePlaneVertices(GRID_SIZE, PLANE_SCALE);
    std::vector<unsigned int> indices = generatePlaneIndices(GRID_SIZE);

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ====== 固定不变的 uniform ======
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 lightDir = glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f));
    glm::vec3 waterColor(0.1f, 0.3f, 0.6f);
    glm::vec3 ambient(0.25f);

    waterShader.use();
    waterShader.setMat4("projection", projection);
    waterShader.setMat4("view", view);
    waterShader.setMat4("model", model);
    waterShader.setVec3("uLightDir", lightDir);
    waterShader.setVec3("uColor", waterColor);
    waterShader.setVec3("uAmbient", ambient);
    waterShader.setFloat("uAmplitude", 0.3f);
    waterShader.setFloat("uTime", 0.0f); // 时间固定，冻结波浪

    // ====== 实验参数组 ======
    std::vector<ParamSet> experiments = {
        // 实验 A：octave 数
        {1, 2.0f, 0.5f, "A_oct1"},
        {2, 2.0f, 0.5f, "A_oct2"},
        {4, 2.0f, 0.5f, "A_oct4"},
        {6, 2.0f, 0.5f, "A_oct6 (基准)"},
        {8, 2.0f, 0.5f, "A_oct8"},

        // 实验 B：lacunarity
        {6, 1.5f, 0.5f, "B_lac1.5"},
        {6, 2.0f, 0.5f, "B_lac2.0 (基准)"},
        {6, 2.5f, 0.5f, "B_lac2.5"},
        {6, 3.0f, 0.5f, "B_lac3.0"},

        // 实验 C：persistence
        {6, 2.0f, 0.3f, "C_pers0.3"},
        {6, 2.0f, 0.4f, "C_pers0.4"},
        {6, 2.0f, 0.5f, "C_pers0.5 (基准)"},
        {6, 2.0f, 0.6f, "C_pers0.6"},
        {6, 2.0f, 0.7f, "C_pers0.7"}
    };

    int currentIndex = 0;
    bool keyPressed = false; // 防止一次按键连续触发多次
    bool keyTPressed = false;

    // 设置第一组参数
    waterShader.setInt("uOctaves", experiments[0].octaves);
    waterShader.setFloat("uLacunarity", experiments[0].lacunarity);
    waterShader.setFloat("uPersistence", experiments[0].persistence);
    std::string title = "fBm Water - " + experiments[0].name;
    glfwSetWindowTitle(window, title.c_str());
    std::cout << "当前组: " << experiments[0].name << std::endl;

    // ====== 主循环 ======
    while (!glfwWindowShouldClose(window)) {
        // 按键处理（空格下一组，ESC 退出）
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (!keyPressed) {
                keyPressed = true;
                currentIndex = (currentIndex + 1) % experiments.size();
                const auto& p = experiments[currentIndex];

                // 更新着色器参数
                waterShader.use();
                waterShader.setInt("uOctaves", p.octaves);
                waterShader.setFloat("uLacunarity", p.lacunarity);
                waterShader.setFloat("uPersistence", p.persistence);

                // 更新窗口标题
                std::string newTitle = "fBm Water - " + p.name;
                glfwSetWindowTitle(window, newTitle.c_str());

                std::cout << "切换到: " << p.name << std::endl;
            }
        }
        else {
            keyPressed = false;
        }

        // ---- T 键触发耗时测量 ----
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            if (!keyTPressed) {
                keyTPressed = true;
                std::cout << "正在测量当前参数组 100 帧平均耗时..." << std::endl;

                // 先丢弃几帧以稳定管线
                for (int i = 0; i < 5; ++i) {
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    glBindVertexArray(VAO);
                    glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);
                    glfwSwapBuffers(window);
                    glfwPollEvents();
                }

                // 正式测量 100 帧
                glFinish(); // 确保之前所有命令完成
                double totalTime = 0.0;
                for (int i = 0; i < 100; ++i) {
                    auto start = std::chrono::high_resolution_clock::now();
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    glBindVertexArray(VAO);
                    glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);
                    glFinish(); // 等待 GPU 完成
                    auto end = std::chrono::high_resolution_clock::now();
                    totalTime += std::chrono::duration<double, std::milli>(end - start).count();
                }
                double avgTime = totalTime / 100.0;
                std::cout << "【耗时结果】" << experiments[currentIndex].name
                    << " : " << avgTime << " ms" << std::endl;
            }
        }
        else {
            keyTPressed = false;
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        // 更新动态时间，让波浪流动
        float currentTime = (float)glfwGetTime();
        waterShader.use();
        waterShader.setFloat("uTime", currentTime * 2.0f);  // 乘以系数减慢速度

        // 渲染
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        waterShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
    return 0;
}