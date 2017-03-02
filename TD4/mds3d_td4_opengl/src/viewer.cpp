#include "viewer.h"
#include "camera.h"

using namespace Eigen;

Viewer::Viewer()
  : _winWidth(0), _winHeight(0), zoom(1), lines(-1), mvtVect(0,0)
{
  _transformMatrix.setIdentity();
}

Viewer::~Viewer()
{
}

////////////////////////////////////////////////////////////////////////////////
// GL stuff

// initialize OpenGL context
void Viewer::init(int w, int h){

    loadShaders();
    glClearColor(0.3, 0.3, 0.3, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    if(!_mesh.load(DATA_DIR"/models/monkey2.obj")) exit(1);
    _mesh.initVBA();

    reshape(w,h);
    _trackball.setCamera(&_cam);

    // earth init
    Affine3f affine;
    affine = Translation3f(Vector3f(2.5,0,0))*AngleAxisf(23.44*M_PI/180, Vector3f(0,0,1))*Scaling(Vector3f(0.5,0.5,0.5));
    _earthTransformMatrix.setIdentity();
    _earthTransformMatrix = _earthTransformMatrix*affine.matrix();
    _earthAxis = Affine3f(AngleAxisf(23.44*M_PI/180., Vector3f(0,0,1)))*Vector3f(0,1,0);
    // moon init
    affine = Translation3f(Vector3f(1.8,0,0))*AngleAxisf(6.68*M_PI/180, Vector3f(0,0,1))*Scaling(Vector3f(0.1,0.1,0.1));
    _moonTransformMatrix.setIdentity();
    _moonTransformMatrix = _moonTransformMatrix*affine.matrix();
    _moonAxis = Affine3f(AngleAxisf(6.68*M_PI/180., Vector3f(0,0,1)))*Vector3f(0,1,0);

    _cam.lookAt(Vector3f(0,0,5),Vector3f(0,0,0),Vector3f(0,1,0));
}

void Viewer::reshape(int w, int h){
    _winWidth = w;
    _winHeight = h;
    _cam.setViewport(w,h);
}

/*!
   callback to draw graphic primitives
 */
void Viewer::drawScene()
{

  glViewport(0, 0, _winWidth, _winHeight);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  Matrix4f projectionMatrix = _cam.projectionMatrix();
  int color = 1;
  _shader.activate();

  // earth rotation
  Affine3f earthAffine;
  Vector3f t = Affine3f(_earthTransformMatrix) * Vector3f(0,0,0);
  earthAffine = AngleAxisf(0.02, Vector3f(0,1,0))
                *Translation3f(t)
                *AngleAxisf(0.2,_earthAxis)
                *AngleAxisf(-0.02, Vector3f(0,1,0))
                *Translation3f(-t);
  _earthTransformMatrix = earthAffine.matrix()*_earthTransformMatrix;

  // moon rotation
  Affine3f moonAffine;
  Vector3f tMoon = Affine3f(_moonTransformMatrix) * Vector3f(0,0,0);
  //rotation sur elle meme
  _moonTransformMatrix = Affine3f(Translation3f(tMoon)*AngleAxisf(0.1, _moonAxis)*Translation3f(-tMoon))*_moonTransformMatrix;
  // correction axe lune
  _moonAxis = Affine3f(AngleAxisf(0.03, Vector3f(0,1,0)))*_moonAxis;
  // suivi de la terre
  _moonTransformMatrix = Affine3f(AngleAxisf(0.02, Vector3f(0,1,0)))*_moonTransformMatrix;
  //rotation autour de la terre
  _moonAxis = Affine3f(AngleAxisf(0.03, Vector3f(0,1,0)))*_moonAxis;
  _moonTransformMatrix = Affine3f(Translation3f(t)*AngleAxisf(0.03, Vector3f(0,1,0))*Translation3f(-t))*_moonTransformMatrix;

  // wireframe display
  color = 2;
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

  glUniformMatrix4fv(_shader.getUniformLocation("viewMatrix"), 1, GL_FALSE, _cam.viewMatrix().data());
  glUniformMatrix4fv(_shader.getUniformLocation("projectionMatrix"), 1, GL_FALSE, projectionMatrix.data());
  glUniform3f(_shader.getUniformLocation("spec_color"),1,1,1);
  glUniform3f(_shader.getUniformLocation("light_dir"),1,1,1);
  glUniform1f(_shader.getUniformLocation("specular"), 15);

  // SUN

  Matrix3f normMatrix = (_cam.viewMatrix()*_transformMatrix).block(0,0,3,3);
  normMatrix.inverse();
  normMatrix.transpose();

  glUniformMatrix3fv(_shader.getUniformLocation("normal_mat"), 1, GL_FALSE, normMatrix.data());
  glUniformMatrix4fv(_shader.getUniformLocation("transformMatrix"), 1, GL_FALSE, _transformMatrix.data());
  _mesh.draw(_shader);

  //EARTH
  normMatrix = ((_cam.viewMatrix()*_earthTransformMatrix).block(0,0,3,3)).inverse().transpose();

  glUniformMatrix3fv(_shader.getUniformLocation("normal_mat"), 1, GL_FALSE, normMatrix.data());
  glUniformMatrix4fv(_shader.getUniformLocation("transformMatrix"), 1, GL_FALSE, _earthTransformMatrix.data());
  _mesh.draw(_shader);

  //MOON
  normMatrix = ((_cam.viewMatrix()*_moonTransformMatrix).block(0,0,3,3)).inverse().transpose();

  glUniformMatrix3fv(_shader.getUniformLocation("normal_mat"), 1, GL_FALSE, normMatrix.data());
  glUniformMatrix4fv(_shader.getUniformLocation("transformMatrix"), 1, GL_FALSE, _moonTransformMatrix.data());
  _mesh.draw(_shader);

  _shader.deactivate();

}

void Viewer::drawScene2D()
{
    glViewport(0, 0, _winWidth, _winHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _shader.activate();

    Affine3f A;
    A = Translation3f(Vector3f(-0.6,-1,0));
    Matrix4f M;
    M <<  0.5, 0, 0, 0,
          0, 0.5, 0, 0,
          0, 0, 0.5, 0,
          0, 0, 0, 1;

    M = A*M;

    Affine3f A2;
    A2 = Translation3f(Vector3f(0.6,-1,0));
    Matrix4f M2;
    M2 <<  -0.5, 0, 0, 0,
          0, 0.5, 0, 0,
          0, 0, 0.5, 0,
          0, 0, 0, 1;

    M2 = A2*M2;

    glUniformMatrix4fv(_shader.getUniformLocation("transformMatrix"), 1, GL_FALSE, M.data());
    _mesh.draw(_shader);
    glUniformMatrix4fv(_shader.getUniformLocation("transformMatrix"), 1, GL_FALSE, M2.data());
    _mesh.draw(_shader);
    glUniformMatrix4fv(_shader.getUniformLocation("transformMatrix"), 1, GL_FALSE, _transformMatrix.data());
    _mesh.draw(_shader);
    _shader.deactivate();
}

void Viewer::updateAndDrawScene()
{
    drawScene();
    //drawScene2D();
}

void Viewer::loadShaders()
{
    // Here we can load as many shaders as we want, currently we have only one:
    _shader.loadFromFiles(DATA_DIR"/shaders/simple.vert", DATA_DIR"/shaders/simple.frag");
    checkError();
}

////////////////////////////////////////////////////////////////////////////////
// Events

/*!
   callback to manage keyboard interactions
   You can change in this function the way the user
   interact with the application.
 */
void Viewer::keyPressed(int key, int action, int /*mods*/)
{
  if(key == GLFW_KEY_R && action == GLFW_PRESS)
  {
    loadShaders();
  }

  if(action == GLFW_PRESS || action == GLFW_REPEAT )
  {
    Affine3f A;
    if (key==GLFW_KEY_UP)
    {
      // A = Translation3f(Vector3f(0,0.1,0));
      // _transformMatrix=A.matrix()*_transformMatrix;
    }
    else if (key==GLFW_KEY_DOWN)
    {
      // A = Translation3f(Vector3f(0,-0.1,0));
      // _transformMatrix=A.matrix()*_transformMatrix;
    }
    else if (key==GLFW_KEY_LEFT)
    {
      // A = Translation3f(Vector3f(0,0.5,0))*AngleAxisf(0.1,Vector3f(0,0,1))*Translation3f(Vector3f(0,-0.5,0));
      // A = Translation3f(Vector3f(-0.1,0,0));
      A = AngleAxisf(0.1,Vector3f(0,-1,0));
      _transformMatrix=A.matrix()*_transformMatrix;
    }
    else if (key==GLFW_KEY_RIGHT)
    {
      // A = Translation3f(Vector3f(0,0.5,0))*AngleAxisf(0.1,Vector3f(0,0,-1))*Translation3f(Vector3f(0,-0.5,0));
      // A = Translation3f(Vector3f(0.1,0,0));
      A = AngleAxisf(0.1,Vector3f(0,1,0));
      _transformMatrix=A.matrix()*_transformMatrix;
    }
    else if (key==GLFW_KEY_PAGE_UP)
    {
      // Matrix4f M;
      // M <<  1.1, 0, 0, 0,
      // 0, 1.1, 0, 0,
      // 0, 0, 1.1, 0,
      // 0, 0, 0, 1;
      // _transformMatrix=_transformMatrix*M;
    }
    else if (key==GLFW_KEY_PAGE_DOWN)
    {
      // Matrix4f M;
      // M <<  0.9, 0, 0, 0,
      // 0, 0.9, 0, 0,
      // 0, 0, 0.9, 0,
      // 0, 0, 0, 1;
      //
      // _transformMatrix=_transformMatrix*M;
    }
    if (key==GLFW_KEY_L)
    {
      lines = -lines;
    }

  }
}

/*!
   callback to manage mouse : called when user press or release mouse button
   You can change in this function the way the user
   interact with the application.
 */
void Viewer::mousePressed(GLFWwindow */*window*/, int /*button*/, int action)
{
  if(action == GLFW_PRESS)
  {
      _trackingMode = TM_ROTATE_AROUND;
      _trackball.start();
      _trackball.track(_lastMousePos);
  }
  else if(action == GLFW_RELEASE)
  {
      _trackingMode = TM_NO_TRACK;
  }
}


/*!
   callback to manage mouse : called when user move mouse with button pressed
   You can change in this function the way the user
   interact with the application.
 */
void Viewer::mouseMoved(int x, int y)
{
    if(_trackingMode == TM_ROTATE_AROUND)
    {
        _trackball.track(Vector2i(x,y));
    }

    _lastMousePos = Vector2i(x,y);
}

void Viewer::mouseScroll(double /*x*/, double y)
{
  _cam.zoom(-0.1*y);
}

void Viewer::charPressed(int /*key*/)
{
}
