#include "sphere.h"
#include <iostream>

Sphere::Sphere(float radius)
    : m_radius(radius)
{
}

Sphere::Sphere(const PropertyList &propList)
{
    m_radius = propList.getFloat("radius",1.f);
    m_center = propList.getPoint("center",Point3f(0,0,0));
}

Sphere::~Sphere()
{
}

bool Sphere::intersect(const Ray& ray, Hit& hit) const
{
    bool ret = false;
    // a = ||d||²
    float a = ray.direction.squaredNorm();
    // b = 2*d.(o-c)
    float b = (2*ray.direction).dot(ray.origin-m_center);
    // c = ||o-c||²-r²
    float c = (ray.origin-m_center).squaredNorm()-m_radius*m_radius;
    // Delta = b²-4ac
    float delta = b*b-4*a*c;
    float t;
    // Solutions
    if (delta < 0) { // no solution
      return false;
    }
    else if (delta == 0) { // 1 solution
      if(float x = -b/(2*a) >= 0) {
        t = x;
        ret = true;
      }
      return false;
    }
    else {  // 2 solutions
      float x1 = (-b-sqrt(delta))/(2*a);
      float x2 = (-b+sqrt(delta))/(2*a);

      if (x1 < x2) {
        if (x1 >= 0) {
          t = x1;
          ret = true;
        }
        else if (x2 >= 0) {
          t = x2;
          ret = true;
        }
      }
      else
        if (x2 >= 0) {
          t = x2;
          ret = true;
        }
        else if (x1 >= 0) {
          t = x1;
          ret = true;
        }
    }
    if (ret && t < hit.t()) {
      hit.setT(t);
      hit.setNormal(Point3f(ray.at(hit.t())-m_center).normalized());
      hit.setShape(this);
      return true;
    }

    return false;
}

REGISTER_CLASS(Sphere, "sphere")
