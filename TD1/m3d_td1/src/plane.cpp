#include "plane.h"

Plane::Plane()
{
}

Plane::Plane(const PropertyList &propList)
{
    m_position = propList.getPoint("position",Point3f(0,0,0));
    m_normal = propList.getVector("normal",Point3f(0,0,1));
}

Plane::~Plane()
{
}

bool Plane::intersect(const Ray& ray, Hit& hit) const
{

    float num = (m_position - ray.origin).dot(m_normal);
    float den = ray.direction.dot(m_normal);

    if (den < Epsilon && den > -Epsilon) { // t infinite
      return false;
    }
    if (den == 0) { // t undefined
      return false;
    }

    float t = num/den;

    if (t > 0 && t < hit.t()) {
      hit.setShape(this);
      hit.setT(t);  // we got the plane
      hit.setNormal(m_normal);
      return true;
    }
    return false;
}

REGISTER_CLASS(Plane, "plane")
