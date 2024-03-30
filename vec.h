#ifndef VEC_H
#define VEC_H

// 定义一个三维向量类
template <typename T>

class Vec
{
public:
    // 成员变量
    T x, y, z;
    // 构造函数
    Vec() : x(T(0)), y(T(0)), z(T(0)) {}
    Vec(T xx) : x(xx), y(xx), z(xx) {}
    Vec(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}

    // 计算单位向量
    Vec &normalize()
    {
        T nor = length();
        if (nor > 0)
        {
            T invNor = 1 / nor;
            x *= invNor, y *= invNor, z *= invNor;
        }
        return *this;
    }

    // Vec中定义重载运算符
    Vec<T> operator*(const T &f) const { return Vec<T>(x * f, y * f, z * f); }

    Vec<T> operator-(const Vec<T> &v) const { return Vec<T>(x - v.x, y - v.y, z - v.z); }
    Vec<T> operator+(const Vec<T> &v) const { return Vec<T>(x + v.x, y + v.y, z + v.z); }
    Vec<T> operator*(const Vec<T> &v) const { return Vec<T>(x * v.x, y * v.y, z * v.z); }

    T dot(const Vec<T> &v) const { return x * v.x + y * v.y + z * v.z; }

    Vec<T> &operator+=(const Vec<T> &v)
    {
        x += v.x, y += v.y, z += v.z;
        return *this;
    }
    Vec<T> &operator*=(const Vec<T> &v)
    {
        x *= v.x, y *= v.y, z *= v.z;
        return *this;
    }
    Vec<T> operator-() const { return Vec<T>(-x, -y, -z); }

    // 计算坐标和原点距离
    T length() const { return sqrt(x * x + y * y + z * z); }

    // 重载流式输出
    friend std::ostream &operator<<(std::ostream &os, const Vec<T> &v)
    {
        os << "[" << v.x << " " << v.y << " " << v.z << "]";
        return os;
    }
};

typedef Vec<float> Vecf;

#endif