#include "camera.h"

using namespace Eigen;

Camera::Camera()
{
  mViewMatrix.setIdentity();
  setPerspective(M_PI/2,0.1,10000);
  setViewport(0,0);
}

void Camera::lookAt(const Vector3f& position, const Vector3f& target, const Vector3f& up)
{
  mTarget = target;
  Vector3f z = (position-target).normalized();
  Vector3f x = (z.cross(up)).normalized();
  Vector3f y = (x.cross(z)).normalized();

  mViewMatrix.row(0) = Vector4f(x(0),x(1),x(2),-x.transpose()*position);
  mViewMatrix.row(1) = Vector4f(y(0),y(1),y(2),-y.transpose()*position);
  mViewMatrix.row(2) = Vector4f(z(0),z(1),z(2),-z.transpose()*position);
  mViewMatrix.row(3) = Vector4f(0,0,0,1);
}

void Camera::setPerspective(float fovY, float near, float far)
{
  m_fovY = fovY;
  m_near = near;
  m_far = far;
}

void Camera::setViewport(int width, int height)
{
  mVpWidth = width;
  mVpHeight = height;
}

void Camera::zoom(float x)
{
  Vector3f t = Affine3f(mViewMatrix) * mTarget;
  mViewMatrix = Affine3f(Translation3f(Vector3f(0,0,x*t.norm())).inverse()) * mViewMatrix;
}

void Camera::rotateAroundTarget(float angle, Vector3f axis)
{
  Vector3f t = Affine3f(mViewMatrix) * mTarget;
  mViewMatrix = Affine3f(Translation3f(t) * AngleAxisf(angle, axis) * Translation3f(-t)) * mViewMatrix;
}

Camera::~Camera()
{
}

const Matrix4f& Camera::viewMatrix() const
{
  return mViewMatrix;
}

Matrix4f Camera::projectionMatrix() const
{
  float aspect = float(mVpWidth)/float(mVpHeight);
  float theta = m_fovY*0.5;
  float range = m_far - m_near;
  float invtan = 1./std::tan(theta);

  Matrix4f projMat;
  projMat.setZero();
  projMat(0,0) = invtan / aspect;
  projMat(1,1) = invtan;
  projMat(2,2) = -(m_near + m_far) / range;
  projMat(2,3) = -2 * m_near * m_far / range;
  projMat(3,2) = -1;

  return projMat;
}
