#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <STB_IMAGE/stb_image_write.h>

#include "SHADER.h"
#include "CAMERA.h"

#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <filesystem> // C++17 用于创建目录

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// 固定相机（俯视角度）
Camera camera(glm::vec3(0.0f, 8.0f, 8.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -35.0f);

const int GRID_SIZE = 200;
const float PLANE_SCALE = 10.0f;

// ---- 生成平面网格 ----
std::vector<float> generatePlaneVertices(int N, float scale) {
    std::vector<float> verts;
    float step = (2.0f * scale) / (N - 1);
    for (int i = 0; i < N; ++i) {
        float z = -scale + i * step;
        for (int j = 0; j < N; ++j) {
            float x = -scale + j * step;
            verts.push_back(x);
            verts.push_back(0.0f);
            verts.push_back(z);
        }
    }
    return verts;
}

std::vector<unsigned int> generatePlaneIndices(int N) {
    std::vector<unsigned int> inds;
    for (int i = 0; i < N - 1; ++i) {
        for (int j = 0; j < N; ++j) {
            inds.push_back(i * N + j);
            inds.push_back((i + 1) * N + j);
        }
        if (i < N - 2) {
            inds.push_back((i + 1) * N + (N - 1));
            inds.push_back((i + 1) * N + 0);
        }
    }
    return inds;
}

// ---- 截图 ----
void saveScreenshot(const std::string& filename, int w, int h) {
    std::vector<unsigned char> pixels(w * h * 3);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    stbi_flip_vertically_on_write(true);
    stbi_write_png(filename.c_str(), w, h, 3, pixels.data(), w * 3);
}

// ---- 测量 100 帧平均耗时（ms） ----
double measureAvgFrameTime(GLFWwindow* window, unsigned int VAO, int indexCount, int frames = 100) {
    glFinish(); // 清空管线
    double total = 0.0;
    for (int i = 0; i < frames; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
        glFinish(); // 等待 GPU 完成
        auto end = std::chrono::high_resolution_clock::now();
        total += std::chrono::duration<double, std::milli>(end - start).count();
    }
    return total / frames;
}

// ---- 实验参数 ----
struct ParamSet {
    int octaves;
    float lacunarity;
    float persistence;
    std::string name;
};

int main() {
    // 创建 output 目录
    std::filesystem::create_directory("output");

    // 初始化 GLFW
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
    glfwSwapInterval(0); // 关闭垂直同步

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // 着色器
    Shader waterShader("resources/shaders/water.vs", "resources/shaders/water.fs");

    // 网格
    auto vertices = generatePlaneVertices(GRID_SIZE, PLANE_SCALE);
    auto indices = generatePlaneIndices(GRID_SIZE);

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

    // 固定 uniform
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
    waterShader.setFloat("uAmplitude", 0.3f);   // 振幅缩放

    // 背景色
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

    // 实验组
    std::vector<ParamSet> experiments = {
        {1, 2.0f, 0.5f, "A_oct1"},
        {2, 2.0f, 0.5f, "A_oct2"},
        {4, 2.0f, 0.5f, "A_oct4"},
        {6, 2.0f, 0.5f, "A_oct6"},   // 基准
        {8, 2.0f, 0.5f, "A_oct8"},
        {6, 1.5f, 0.5f, "B_lac1.5"},
        {6, 2.0f, 0.5f, "B_lac2.0"},
        {6, 2.5f, 0.5f, "B_lac2.5"},
        {6, 3.0f, 0.5f, "B_lac3.0"},
        {6, 2.0f, 0.3f, "C_pers0.3"},
        {6, 2.0f, 0.4f, "C_pers0.4"},
        {6, 2.0f, 0.5f, "C_pers0.5"},
        {6, 2.0f, 0.6f, "C_pers0.6"},
        {6, 2.0f, 0.7f, "C_pers0.7"}
    };

    std::ofstream timeFile("output/timing_results.csv");
    timeFile << "name,time_ms\n";

    // 主循环（自动遍历实验组）
    for (size_t idx = 0; idx < experiments.size(); ++idx) {
        const auto& p = experiments[idx];
        std::cout << "当前实验: " << p.name << std::endl;

        // 更新窗口标题
        std::string title = "fBm Water - " + p.name;
        glfwSetWindowTitle(window, title.c_str());

        // 设置参数
        waterShader.use();
        waterShader.setInt("uOctaves", p.octaves);
        waterShader.setFloat("uLacunarity", p.lacunarity);
        waterShader.setFloat("uPersistence", p.persistence);

        // ---- 阶段1：流动展示 3 秒 ----
        double showStart = glfwGetTime();
        while (glfwGetTime() - showStart < 3.0) {
            // 事件处理，防止窗口无响应
            glfwPollEvents();
            if (glfwWindowShouldClose(window)) {
                glfwTerminate();
                timeFile.close();
                return 0;
            }
            float t = (float)glfwGetTime();
            waterShader.use();
            waterShader.setFloat("uTime", t * 2.0f); // 流动
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);
            glfwSwapBuffers(window);
        }

        // ---- 阶段2：冻结波浪并截图 ----
        // 固定时间戳，保证所有组截图条件一致（均取同一逻辑时刻）
        waterShader.use();
        waterShader.setFloat("uTime", 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents(); // 确保窗口刷新

        std::string screenshotName = "output/" + p.name + ".png";
        saveScreenshot(screenshotName, SCR_WIDTH, SCR_HEIGHT);
        std::cout << "截图已保存: " << screenshotName << std::endl;

        // ---- 阶段3：测量 100 帧平均耗时 ----
        double avgTime = measureAvgFrameTime(window, VAO, indices.size(), 100);
        std::cout << "平均耗时: " << avgTime << " ms" << std::endl;
        timeFile << p.name << "," << avgTime << "\n";

        // 最后一组不需要“切换到下一组”提示
        if (idx != experiments.size() - 1) {
            std::cout << "3 秒后自动切换到下一组..." << std::endl;
            // 再次流动过渡（可选），避免突兀
            double waitStart = glfwGetTime();
            while (glfwGetTime() - waitStart < 3.0) {
                glfwPollEvents();
                if (glfwWindowShouldClose(window)) {
                    glfwTerminate();
                    timeFile.close();
                    return 0;
                }
                float t = (float)glfwGetTime();
                waterShader.use();
                waterShader.setFloat("uTime", t * 0.2f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);
                glfwSwapBuffers(window);
            }
        }
    }

    timeFile.close();
    std::cout << "所有实验完成。截图和耗时数据已保存到 output/ 文件夹。" << std::endl;

    // 清理
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
    return 0;
}