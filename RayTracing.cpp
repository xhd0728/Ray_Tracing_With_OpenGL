#include "glad/glad.h"

#include <GLFW/glfw3.h>
#include <sstream>
#include <vector>
#include <iostream>
#include <cmath>
#include <random>
#include <fstream>
#include <algorithm>

#include "vec.h"
#include "orb.h"
using namespace std;

// 最大递归深度
// 递归深度越大，计算越慢，效果越好
// 递归深度过大时，可能造成同一像素点颜色积累过多，图像偏黑
#define MAX_RAY_DEPTH 5

// 设置顶点着色器
GLuint VBO, VAO;

// 设置屏幕宽和高
const GLuint WIDTH = 1280;
const GLuint HEIGHT = 720;

// 设置窗口标题
const char *TITLE = "HEU_EASY_OPENGL";

// 计算光线按比例混合色度值
float mix(const float &a, const float &b, const float &mix)
{
    return b * mix + a * (1 - mix);
}

/**
 * 主跟踪函数
 * 测试该光线是否与场景中的任何几何体相交
 * 如果光线与一个物体相交，计算交点、在交点处的法线，并对该点进行着色
 * 着色取决于曲面特性（是否透明、反射、漫反射）
 * 光线不交于物体的话返回背景色
 * @param   rayorig   光线原点
 * @param   raydir    光线方向的单位向量
 * @param   orbs      球体集合
 * @param   depth     递归深度
 * @return  Vecf      颜色向量
 */
Vecf trace(const Vecf &rayorig,
           const Vecf &raydir,
           const std::vector<orb *> &orbs,
           const int &depth)
{
    float tnear = INFINITY; // 初始化距离为无穷远处
    const orb *orb = nullptr;

    // 在场景中找到此光线与球体最前面的交点
    // 对每一个球体进行相交判断
    for (auto i : orbs)
    {
        // 定义直线与球面的交点
        float t0 = INFINITY;
        float t1 = INFINITY;

        // 判断光线与球体相交
        if (i->intersect(rayorig, raydir, &t0, &t1))
        {
            // 如果光线在球的里面，就采用前面的交点
            if (t0 < 0)
                t0 = t1;

            // 判断 tnear 是否是最近的交点
            if (t0 < tnear)
            {
                tnear = t0; // 将最近的交点设置为t0
                orb = i;    // 设置球体
            }
        }
    }

    if (orb != NULL)
    {
        Vecf surfaceColor = 1.0;              // 球体表面的颜色
        Vecf phit = rayorig + raydir * tnear; // 计算交点坐标
        Vecf nhit = phit - orb->center;       // 计算交点法向量
        nhit.normalize();                     // 交点法向量标准化
        float bias = 1e-4;                    // 设置追踪偏差

        // 如果法线和视图方向不相反，反转法线方向
        if (raydir.dot(nhit) > 0)
        {
            nhit = -nhit;
        }

        // 如果表面能够反射，且未达到最大递归深度，则进行反射计算
        if ((orb->transparency > 0 || orb->reflection > 0) && depth < MAX_RAY_DEPTH)
        {
            // 光线方向规范化
            float IdotN = raydir.dot(nhit);

            // 如果-IdotN为负，说明在视点背面，不用显示，取0
            float facingratio = max(float(0), -IdotN);

            // 不同光波分量被折射和反射，遵循菲涅尔效应
            // 当视线垂直于表面时，反射较弱
            // 当视线非垂直表面时，夹角越小，反射越明显
            float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1);

            // 计算反射光线
            Vecf refldir = raydir - nhit * 2 * raydir.dot(nhit);

            // 反射光线向量规范化
            refldir.normalize();

            // 递归计算反射
            // 交点作为原点，进行光线追踪，增加递归深度，返回颜色
            Vecf reflection = trace(phit + nhit * bias, refldir, orbs, depth + 1);

            // 初始化折射率
            Vecf refraction = 0;

            // 如果透明度不为零，进行折射计算
            if (orb->transparency > 0)
            {
                // 折射系数
                float ior = 1.2;

                // 折射率
                float eta = 1 / ior;

                // 菲涅尔折射系数
                float k = 1 - eta * eta * (1 - IdotN * IdotN);

                // 方向向量乘上折射率
                // 加上菲涅尔效应的影响
                Vecf refrdir = raydir * eta - nhit * (eta * IdotN + sqrt(k));

                // 折射光线规范化
                refrdir.normalize();

                // 递归计算折射率
                // 交点作为原点，进行光线追踪，增加递归深度，返回颜色
                refraction = trace(phit - nhit * bias, refrdir, orbs, depth + 1);
            }
            // 结果是反射和折射的混合（如果球体是透明的）

            // 反射部分的颜色
            Vecf reflect_color = reflection * fresneleffect;

            // 折射部分的颜色
            Vecf refract_color = refraction * (1 - fresneleffect) * orb->transparency;

            // 计算混合后的表面颜色
            surfaceColor = (reflect_color + refract_color) * orb->surfaceColor;
        }
        else
        {
            // 这是一个折射率和反射率为0的物体，不需要再进行光线追踪
            double shadow = 1.0;

            // 遍历每个物体，依次计算是否相交，相交的话更新阴影
            for (unsigned i = 0; i < orbs.size(); ++i)
            {
                if (orbs[i]->emissionColor.x > 0)
                {
                    // 初始化转化率
                    Vecf transmission = 1.0;

                    // 球体法向量
                    Vecf lightDirection = orbs[i]->center - phit;

                    // 法向量规范化
                    lightDirection.normalize();

                    for (unsigned j = 0; j < orbs.size(); ++j)
                    {
                        if (i != j)
                        {
                            float t0, t1;
                            // 判断该点的光是否和源光线相交，如果相交，计算阴影
                            if (orbs[j]->intersect(phit + (nhit * bias), lightDirection, &t0, &t1))
                            {
                                // 相交的话更新折射率
                                shadow = std::max(0.0, shadow - (1.0 - orbs[j]->transparency));

                                // 计算转化率
                                transmission = transmission * shadow;
                            }
                        }
                    }

                    // 用冯氏模型计算每一条对这点像素造成影响的光线
                    // 加上最后传递的颜色
                    surfaceColor += orb->surfaceColor * transmission * orbs[i]->emissionColor;
                }
            }
        }
        // 返回最终的颜色
        return surfaceColor;
    }
    else
    {
        // 返回背景颜色
        return Vecf(1.0, 1.0, 1.0);
    }
}

/**
 * 为图像的每个像素计算一条光线，跟踪它并返回一个颜色，
 * 如果光线击中球体，则返回相交点处球体的颜色，否则返回背景色
 * @param orbs      球体集合
 * @param window    GLFW窗口
 */
void render(const std::vector<orb *> &orbs, GLFWwindow *window)
{
    // 初始化图像
    Vecf *image = new Vecf[WIDTH * HEIGHT];
    Vecf *pixel = image;

    // 计算屏占比
    float invWidth = 1 / float(WIDTH), invHeight = 1 / float(HEIGHT);

    // 设定视场角
    float fov = 40;

    // 设定宽高比
    float aspectratio = WIDTH / float(HEIGHT);

    // 把视场角转化为普通的角度
    float angle = tan(M_PI * 0.5 * fov / 180.0);

    // 逐像素点进行光线追踪
    for (unsigned y = 0; y < HEIGHT; ++y)
    {
        for (unsigned x = 0; x < WIDTH; ++x, ++pixel)
        {
            // 进行坐标系的转换
            float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;

            // z的值决定
            Vecf raydir(xx, yy, -1);

            // 光线向量标准化
            raydir.normalize();

            // 调用trace追踪函数
            *pixel = trace(Vecf(0), raydir, orbs, 0);
        }
    }

    // 顶点集合
    vector<float> vertices;
    for (unsigned int i = 0; i < HEIGHT; i++)
    {
        for (unsigned int j = 0; j < WIDTH; j++)
        {
            // 坐标转换
            auto a = -2 * (float(j) / WIDTH - 0.5);
            auto b = -2 * (float(i) / HEIGHT - 0.5);

            // 添加顶点
            vertices.push_back(a); // x
            vertices.push_back(b); // y
            vertices.push_back(0); // z

            // 设置颜色值
            vertices.push_back(min(image[i * WIDTH + j].x, float(1))); // R
            vertices.push_back(min(image[i * WIDTH + j].y, float(1))); // G
            vertices.push_back(min(image[i * WIDTH + j].z, float(1))); // B
        }
    }

    // 设置着色器
    // 顶点着色器硬编码
    const char *vert = "#version 330 core\n"
                       "layout (location = 0) in vec3 aPos;\n"
                       "layout (location = 1) in vec3 aColor;\n"
                       "out vec3 ourColor;\n"
                       "void main()\n"
                       "{\n"
                       "gl_Position = vec4(aPos, 1.0);\n"
                       "ourColor = aColor;\n"
                       "}";

    // 片段着色器硬编码
    const char *frag = "#version 330 core\n"
                       "out vec4 FragColor;\n"
                       "in vec3 ourColor;\n"
                       "void main()\n"
                       "{\n"
                       "FragColor = vec4(ourColor, 1.0f);\n"
                       "}";

    // 创建顶点着色器对象
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // 创建片段着色器对象
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // 将顶点着色器源码附加到顶点着色器对象上并编译
    glShaderSource(vertexShader, 1, &vert, NULL);
    glCompileShader(vertexShader);

    // 将片段着色器源码附加到片段着色器对象上并编译
    glShaderSource(fragmentShader, 1, &frag, NULL);
    glCompileShader(fragmentShader);

    // 创建 shader program 对象
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    // 将顶点着色器和片段着色器附加到program对象上
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    // 将二者链接
    glLinkProgram(shaderProgram);

    // 将 shader program 对象设置为当前使用的 shader 程序
    glUseProgram(shaderProgram);

    // 删除顶点和片段着色器对象
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 创建一个 VAO（顶点数组对象）
    glGenVertexArrays(1, &VAO);

    // 绑定VAO
    glBindVertexArray(VAO);

    // 创建一个 VBO（缓冲对象）
    glGenBuffers(1, &VBO);

    // 绑定 VBO，指定其类型为 GL_ARRAY_BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 将数据存入 VBO
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * 4, &vertices[0], GL_STATIC_DRAW);

    // 为顶点属性指针提供数据，并启用顶点属性
    // 参数说明：位置、每个顶点属性的组成数量（vec3 则为 3，vec4 则为 4），数据类型，是否归一化，步长，偏移量
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);

    // 启用顶点属性
    glEnableVertexAttribArray(0);

    // 同上，为法向量属性指针提供数据，并启用法向量属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    while (!glfwWindowShouldClose(window))
    {
        // 响应事件
        // 设置清屏颜色
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // 设置清空深度缓冲区时的深度值
        glClearDepth(1.0);

        // 清空颜色缓冲区和深度缓冲区
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 绘制顶点数组中的点
        glDrawArrays(GL_POINTS, 0, vertices.size() / 6);

        // 启用深度测试，避免重叠的物体被遮挡
        glEnable(GL_DEPTH_TEST);

        // 交换缓冲区以显示图像
        glfwSwapBuffers(window);

        // 处理窗口事件
        glfwPollEvents();
    }

    // 删除 VAO
    glDeleteVertexArrays(1, &VAO);

    // 删除 VBO
    glDeleteBuffers(1, &VBO);

    // 终止 GLFW
    glfwTerminate();
}

/**
 * 回调函数，
 * 检测并响应键盘事件
 * @param window GLFW窗口
 * @param key 键盘按键输入
 * @param scancode 系统扫描码
 * @param action 响应事件
 * @param mode 键盘修饰符的状态
 */
void key_call_back(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

/**
 * 生成介于L和R之间的随机数
 * @param L     随机数下限
 * @param R     随机数上限
 * @return double 随机数
 */
double random_double(double L, double R)
{
    random_device rd;                            // 随机设备
    mt19937 gen(rd());                           // 随机数引擎
    uniform_real_distribution<double> dis(L, R); // 均匀分布生成器
    return dis(gen);
}

void show_balls(GLFWwindow *window);

/**
 * 主函数
 * @return code 程序状态码
 */
int main(int argc, char **argv)
{
    glfwInit();                                    // 初始化GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL主版本号 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // OpenGL副版本号 3
    glfwWindowHint(GLFW_OPENGL_PROFILE,            // OpenGL模式
                   GLFW_OPENGL_CORE_PROFILE        // OpenGL核心模式
    );

    GLFWwindow *window = glfwCreateWindow(WIDTH,   // 窗口宽度
                                          HEIGHT,  // 窗口高度
                                          TITLE,   // 窗口标题
                                          nullptr, // 显示器
                                          nullptr  // 窗口
    );

    // 窗口创建失败
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // 令该程序所有绘画操作在window上
    glfwMakeContextCurrent(window);

    // 设置键盘回调函数
    glfwSetKeyCallback(window, key_call_back);

    // 设置光标可见性
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    show_balls(window);
    return 0;
}

/**
 * 空间内容展示
 * 展示若干个球体
 * @param window GLFW窗口
 */
void show_balls(GLFWwindow *window)
{
    // 球体集合
    vector<orb *> orbs;

    // 底面的大球
    orbs.push_back(new orb(Vecf(0, -10004, -20), 10001, Vecf(1.0, 1.0, 1.0), 1.0, 0.0));

    // 生成球体的数量
    const int BALL_COUNT = 15;

    // 记录球体参数信息
    ofstream out("/home/xhd0728/BallTracing/balls/test.txt");
    if (!out.is_open())
    {
        cout << "cannot open file"
             << "\n";
    }

    // 随机生成球体参数
    for (int i = 0; i < BALL_COUNT; ++i)
    {
        // 随机坐标
        float x = random_double(-10, 10);  // x
        float y = random_double(-2, 2);    // y
        float z = random_double(-20, -20); // z
        // 随机半径
        float r = random_double(0.3, 1.2); // r
        // 随机颜色
        float R = random_double(0.5, 0.9); // R
        float G = random_double(0.5, 0.9); // G
        float B = random_double(0.5, 0.9); // B
        // 随机折射率和反射率
        float u = random_double(0.5, 0.2); // reflection
        float v = random_double(0.9, 0.9); // refraction

        out << "(" << x << "," << y << "," << z << ")\t" << r << "\t";
        out << "(" << R << "," << G << "," << B << ")\t";
        out << u << "\t" << v << "\n";

        // 加入球体集合
        orbs.push_back(new orb(Vecf(x, y, z), r, Vecf(R, G, B), u, v));
    }
    out.close();

    // 开始光线追踪
    render(orbs, window);
}
