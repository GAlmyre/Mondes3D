#include "plane.h"

Plane::Plane()
{
}

Plane::Plane(const PropertyList &propList)
{
    m_position = propList.getPoint("position",Point3f(0,0,0));
    m_normal = propList.getVector("direction",Point3f(0,0,1));
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

    if (t < 0) {  // behind the camera
      return false;
    }
    hit.setShape(this);
    hit.setT(t);  // we got the plane
    hit.setNormal(m_normal);
    return true;
}

REGISTER_CLASS(Plane, "plane")
