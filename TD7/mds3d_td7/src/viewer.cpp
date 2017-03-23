#include "viewer.h"
#include "camera.h"
#include "SOIL2.h"

using namespace Eigen;

#define MAG_MAX 2
#define MIN_MAX 6

GLenum mag[2] = {GL_NEAREST, GL_LINEAR};
GLenum min[6] = {GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST,
              GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST,
              GL_LINEAR_MIPMAP_LINEAR};
int mag_it = 0;
int min_it = 0;

Viewer::Viewer()
  : _winWidth(0), _winHeight(0), _theta(0), _rotate(true)
{
}

Viewer::~Viewer()
{
}

////////////////////////////////////////////////////////////////////////////////
// GL stuff

// initialize OpenGL context
void Viewer::init(int w, int h){

    // Background color
    glClearColor(0.0, 0.0, 0.0, 0.0);

    loadShaders();
    if(!_mesh.load(DATA_DIR"/models/earth2.obj")) exit(1);
    _mesh.initVBA();
    _tex_2d = SOIL_load_OGL_texture
    (
        DATA_DIR"/textures/earth.jpg",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );
    _tex_2d_clouds = SOIL_load_OGL_texture
    (
        DATA_DIR"/textures/earth_clouds.jpg",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );
    _tex_2d_night = SOIL_load_OGL_texture
    (
        DATA_DIR"/textures/earth_night.jpg",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );
    _tex_2d_board = SOIL_load_OGL_texture
    (
        DATA_DIR"/textures/checkerboard.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );
    _normal_map = SOIL_load_OGL_texture
    (
        DATA_DIR"/textures/earth_normal.jpg",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );
    _tex_diffuse = SOIL_load_OGL_texture
    (
        DATA_DIR"/textures/diffuse.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );
    glGenSamplers(1, &_samplerId);


    int texW = 256;
    int texH = 256;
    unsigned char colors[6][3] = { {255,  0 , 0}, {0, 255,  0 }, { 0,  0, 255},
                                   {255, 255, 0}, {0, 255, 255}, {255, 0, 255} };
    unsigned char data[texW*texH*3];
    glGenTextures(1,&_texID1);
    glBindTexture(GL_TEXTURE_2D, _texID1);
    for(int l=0; ; ++l) {
      for(int i=0; i<texW*texH; ++i){
          data[3*i  ] = colors[l][0];
          data[3*i+1] = colors[l][1];
          data[3*i+2] = colors[l][2];
      }
    glTexImage2D(GL_TEXTURE_2D, l, (GLint)GL_RGB, texW, texH, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    if(texW==1 && texH==1)
      break;
    texW = std::max(1, texW/2);
    texH = std::max(1, texH/2);
    }

    reshape(w,h);
    _cam.setPerspective(M_PI/3,0.1f,20000.0f);
    _cam.lookAt(Vector3f(0,0,4), Vector3f(0,0,0), Vector3f(0,1,0));
    _trackball.setCamera(&_cam);

    glEnable(GL_DEPTH_TEST);
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
    // configure the rendering target size (viewport)
    glViewport(0, 0, _winWidth, _winHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glSamplerParameteri(_samplerId, GL_TEXTURE_MAG_FILTER, (GLint)_magFilter);
    glSamplerParameteri(_samplerId, GL_TEXTURE_MIN_FILTER, (GLint)_minFilter);

    _shader.activate();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,_tex_2d);
    glBindSampler(0, _samplerId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,_tex_2d_clouds);
    glBindSampler(1, _samplerId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D,_tex_2d_night);
    glBindSampler(2, _samplerId);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D,_normal_map);
    glBindSampler(3, _samplerId);

    glUniformMatrix4fv(_shader.getUniformLocation("view_mat"),1,GL_FALSE,_cam.viewMatrix().data());
    glUniformMatrix4fv(_shader.getUniformLocation("proj_mat"),1,GL_FALSE,_cam.projectionMatrix().data());
    glUniform1i(_shader.getUniformLocation("sampler"),0);
    glUniform1i(_shader.getUniformLocation("clouds_sampler"),1);
    glUniform1i(_shader.getUniformLocation("night_sampler"),2);
    glUniform1i(_shader.getUniformLocation("normal_map"),3);

    Affine3f M(AngleAxisf(_theta,Vector3f(0,1,0)));


    glUniformMatrix4fv(_shader.getUniformLocation("obj_mat"),1,GL_FALSE,M.matrix().data());

    Matrix4f matLocal2Cam = _cam.viewMatrix() * M.matrix();
    Matrix3f matN = matLocal2Cam.topLeftCorner<3,3>().inverse().transpose();
    glUniformMatrix3fv(_shader.getUniformLocation("normal_mat"),1,GL_FALSE,matN.data());

    Vector3f lightDir = Vector3f(1,0,1).normalized();
    lightDir = (_cam.viewMatrix().topLeftCorner<3,3>() * lightDir).normalized();
    //lightDir = Affine3f(AngleAxisf(-_theta,Vector3f(0,1,0)))*lightDir;
    glUniform3fv(_shader.getUniformLocation("lightDir"),1,lightDir.data());

    _mesh.draw(_shader);

    _shader.deactivate();
}


void Viewer::updateAndDrawScene()
{
  if(_rotate)
    _theta += 0.01*M_PI;
  drawScene();
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
  else if(key == GLFW_KEY_A && action == GLFW_PRESS)
  {
    _rotate = !_rotate;
  }

  if(action == GLFW_PRESS || action == GLFW_REPEAT )
  {
    if (key==GLFW_KEY_UP)
    {
      _magFilter = mag[++mag_it%MAG_MAX];
    }
    else if (key==GLFW_KEY_DOWN)
    {

    }
    else if (key==GLFW_KEY_LEFT)
    {

    }
    else if (key==GLFW_KEY_RIGHT)
    {
      _minFilter = min[++min_it%MIN_MAX];
    }
    else if (key==GLFW_KEY_PAGE_UP)
    {
    }
    else if (key==GLFW_KEY_PAGE_DOWN)
    {
    }
  }
}

void Viewer::charPressed(int /*key*/)
{
}
