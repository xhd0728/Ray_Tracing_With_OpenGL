#ifndef ORB_H
#define ORB_H

#include <cmath>
#include "vec.h"

// 定义一个球类用于绘制，与光线求交
class orb
{
public:
    // 成员变量
    Vecf center;        // 球心坐标
    float radius;       // 球体半径
    float radius2;      // 球体半径平方
    Vecf surfaceColor;  // 球体表面颜色
    Vecf emissionColor; // 反射光线颜色
    float transparency; // 球体表面反射率
    float reflection;   // 球体透明度

    // 构造函数
    orb() {}
    orb(const Vecf &c,           // 球体中心点向量
        const float &r,          // 球体半径
        const Vecf &sc,          // 球体表面颜色
        const float &refl = 0,   // 球体折射率
        const float &transp = 0, // 球体表面反射率

        // 类内构造函数
        const Vecf &ec = 0) : center(c),
                              radius(r),
                              radius2(r * r),
                              surfaceColor(sc),
                              emissionColor(ec),
                              transparency(transp),
                              reflection(refl)
    {
    }

    // 计算光线和球体的交点
    bool intersect(const Vecf &rayorig, // 光线原点
                   const Vecf &raydir,  // 光线方向
                   float *t0,           // 第一个交点
                   float *t1            // 第二个交点
    ) const
    {
        // 光线原点到球心向量_l
        Vecf _l = center - rayorig;

        // 入射方向和光线到球心向量的夹角余弦
        // cos = _l * cos(theta)
        float cos = _l.dot(raydir);

        // 如果夹角大于90度，光线不可能射中球体
        if (cos < 0)
            return false;

        // d^2 = _l^2 * sin^2 = l^2 - l^2 * cos(theta)^2
        float d2 = _l.dot(_l) - (cos * cos);

        // 距离大于半径，光线和球无交点
        if (d2 > radius2)
            return false;

        // 与球心到光线距离垂直平分线的距离
        // 已知 d^2 <= r^2，可以避免abs函数的使用
        float thc = sqrt(radius2 - d2);

        if (t0 != nullptr && t1 != nullptr)
        {
            *t0 = cos - thc; // 到前一个交点的距离
            *t1 = cos + thc; // 到后一个交点的距离
        }

        return true;
    }
};

#endif