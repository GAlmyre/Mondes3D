
#include "mesh.h"
#include "shader.h"
#include <Eigen/Geometry>
#include <iostream>
#include <fstream>
#include <limits>

using namespace Eigen;

bool Mesh::load(const std::string& filename)
{
  std::string ext = filename.substr(filename.size()-3,3);
  if(ext=="off" || ext=="OFF")
    return loadOFF(filename);
  else if(ext=="obj" || ext=="OBJ")
    return loadOBJ(filename);

  std::cerr << "Mesh: extension \'" << ext << "\' not supported." << std::endl;
  return false;
}

void Mesh::computeNormals()
{
  //on met les normales de chaque sommet à 0
  for(std::vector<Vertex>::iterator it = mVertices.begin();it != mVertices.end();++it)
  {
      (*it).normal = Vector3f::Zero();
      (*it).tangent = Vector3f::Zero();
      (*it).bitangent = Vector3f::Zero();
  }
  //on parcourt les faces
  for(std::vector<Vector3i>::iterator it = mFaces.begin(); it != mFaces.end();++it)
  {
      Vector3i v = (*it);
      Vertex v0 = mVertices[v(0)];
      Vertex v1 = mVertices[v(1)];
      Vertex v2 = mVertices[v(2)];

      Vector3f q1 = v1.position-v0.position;
      Vector3f q2 = v2.position-v0.position;

      float s1 = v1.texcoord[0]-v0.texcoord[0];
      float s2 = v2.texcoord[0]-v0.texcoord[0];

      float t1 = v1.texcoord[1]-v0.texcoord[1];
      float t2 = v2.texcoord[1]-v0.texcoord[1];

      MatrixXf q1q2(3,2);
      q1q2 << q1(0),q2(0),
              q1(1),q2(1),
              q1(2),q2(2);

      Matrix2f m;
      m <<  s1,s2,
            t1,t2;

      MatrixXf TB(2,3);
      TB = q1q2*m.inverse();

      Vector3f T = TB.block<3,1>(0,0);
      Vector3f B = TB.block<3,1>(0,1);

      Vector3f n = q1.cross(q2);//.normalized();

      v0.normal += n;
      v1.normal += n;
      v2.normal += n;

      v0.tangent += T;
      v1.tangent += T;
      v2.tangent += T;

      v0.bitangent += B;
      v1.bitangent += B;
      v2.bitangent += B;

      mVertices[v(0)] = v0;
      mVertices[v(1)] = v1;
      mVertices[v(2)] = v2;
  }
  for(std::vector<Vertex>::iterator it = mVertices.begin();it != mVertices.end();++it)
  {
      (*it).tangent = (*it).tangent - ((*it).normal.dot((*it).tangent))*(*it).normal;
      (*it).bitangent = (*it).bitangent - ((*it).normal.dot( (*it).bitangent))*(*it).normal - ((*it).tangent.dot((*it).bitangent)) * (*it).tangent/(*it).tangent.norm();

      (*it).normal = (*it).normal.normalized();
      (*it).tangent = (*it).tangent.normalized();
      (*it).bitangent = (*it).bitangent.normalized();
  }
}

void Mesh::initVBA()
{
  // create the BufferObjects and copy the related data into them.

  // create a VBO identified by a unique index:
  glGenBuffers(1,&mVertexBufferId);
  // activate the VBO:
  glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId);
  // copy the data from host's RAM to GPU's video memory:
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*mVertices.size(), mVertices[0].position.data(), GL_STATIC_DRAW);

  glGenBuffers(1,&mIndexBufferId);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferId);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Vector3i)*mFaces.size(), mFaces[0].data(), GL_STATIC_DRAW);

  glGenVertexArrays(1,&mVertexArrayId);

  mIsInitialized = true;
}

Mesh::~Mesh()
{
  if(mIsInitialized)
  {
    glDeleteBuffers(1,&mVertexBufferId);
    glDeleteBuffers(1,&mIndexBufferId);
  }
}


void Mesh::draw(const Shader &shd)
{
  // Activate the VBO of the current mesh:
  glBindVertexArray(mVertexArrayId);
  glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferId);

  // Specify vertex data

  // 1 - get id of the attribute "vtx_position" as declared as "in vec3 vtx_position" in the vertex shader
  int vertex_loc = shd.getAttribLocation("vtx_position");
  // 2 - tells OpenGL where to find the x, y, and z coefficients:
  glVertexAttribPointer(vertex_loc,     // id of the attribute
                        3,              // number of coefficients (here 3 for x, y, z)
                        GL_FLOAT,       // type of the coefficients (here float)
                        GL_FALSE,       // for fixed-point number types only
                        sizeof(Vertex), // number of bytes between the x coefficient of two vertices
                                        // (e.g. number of bytes between x_0 and x_1)
                        0);             // number of bytes to get x_0
  // 3 - activate this stream of vertex attribute
  glEnableVertexAttribArray(vertex_loc);

  int normal_loc = shd.getAttribLocation("vtx_normal");
  if(normal_loc>=0)
  {
    glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(Vector3f));
    glEnableVertexAttribArray(normal_loc);
  }

  int color_loc = shd.getAttribLocation("vtx_color");
  if(color_loc>=0)
  {
    glVertexAttribPointer(color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(Vector3f)));
    glEnableVertexAttribArray(color_loc);
  }
  int texture_loc = shd.getAttribLocation("vtx_texture");
  if(texture_loc>=0)
  {
    glVertexAttribPointer(texture_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)((2*sizeof(Vector3f)+(sizeof(Vector4f)))));
    glEnableVertexAttribArray(texture_loc);
  }
  int tangent_loc = shd.getAttribLocation("vtx_tangent");
  if(tangent_loc>=0)
  {
    glVertexAttribPointer(tangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(Vector3f)+sizeof(Vector4f)+sizeof(Vector2f)));
    glEnableVertexAttribArray(tangent_loc);
  }
  int bitangent_loc = shd.getAttribLocation("vtx_bitangent");
  if(bitangent_loc>=0)
  {
    glVertexAttribPointer(bitangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(Vector3f)+sizeof(Vector4f)+sizeof(Vector2f)));
    glEnableVertexAttribArray(bitangent_loc);
  }

  // send the geometry
  glDrawElements(GL_TRIANGLES, 3*mFaces.size(), GL_UNSIGNED_INT, 0);

  // at this point the mesh has been drawn and raserized into the framebuffer!
  glDisableVertexAttribArray(vertex_loc);

  if(normal_loc>=0) glDisableVertexAttribArray(normal_loc);
  if(color_loc>=0)  glDisableVertexAttribArray(color_loc);
  if(texture_loc>=0)  glDisableVertexAttribArray(texture_loc);
  if(tangent_loc>=0)  glDisableVertexAttribArray(tangent_loc);
  if(bitangent_loc>=0)  glDisableVertexAttribArray(bitangent_loc);
  checkError();
}




//********************************************************************************
// Loaders...
//********************************************************************************


bool Mesh::loadOFF(const std::string& filename)
{
  std::ifstream in(filename.c_str(),std::ios::in);
  if(!in)
  {
    std::cerr << "File not found " << filename << std::endl;
    return false;
  }

  std::string header;
  in >> header;

  bool hasColor = false;

  // check the header file
  if(header != "OFF")
  {
    if(header != "COFF")
    {
      std::cerr << "Wrong header = " << header << std::endl;
      return false;
    }
    hasColor = true;
  }

  int nofVertices, nofFaces, inull;
  int nb, id0, id1, id2;
  Vector3f v;

  in >> nofVertices >> nofFaces >> inull;

  for(int i=0 ; i<nofVertices ; ++i)
  {
    in >> v[0] >> v[1] >> v[2];
    mVertices.push_back(Vertex(v));

    if(hasColor) {
      Vector4f c;
      in >> c[0] >> c[1] >> c[2] >> c[3];
      mVertices[i].color = c/255.;
    }
  }

  for(int i=0 ; i<nofFaces ; ++i)
  {
    in >> nb >> id0 >> id1 >> id2;
    assert(nb==3);
    mFaces.push_back(Vector3i(id0, id1, id2));
  }

  in.close();

  computeNormals();

  return true;
}



#include <ObjFormat/ObjFormat.h>

bool Mesh::loadOBJ(const std::string& filename)
{
  ObjMesh* pRawObjMesh = ObjMesh::LoadFromFile(filename);

  if (!pRawObjMesh)
  {
      std::cerr << "Mesh::loadObj: error loading file " << filename << "." << std::endl;
      return false;
  }

  // Makes sure we have an indexed face set
  ObjMesh* pObjMesh = pRawObjMesh->createIndexedFaceSet(Obj::Options(Obj::AllAttribs|Obj::Triangulate));
  delete pRawObjMesh;
  pRawObjMesh = 0;

  // copy vertices
  mVertices.resize(pObjMesh->positions.size());

  for (std::size_t i=0 ; i<pObjMesh->positions.size() ; ++i)
  {
    mVertices[i] = Vertex(Vector3f(pObjMesh->positions[i].x, pObjMesh->positions[i].y, pObjMesh->positions[i].z));

    if(!pObjMesh->texcoords.empty())
      mVertices[i].texcoord = Vector2f(pObjMesh->texcoords[i]);

    if(!pObjMesh->normals.empty())
      mVertices[i].normal = Vector3f(pObjMesh->normals[i]);
  }

  // copy faces
  for (std::size_t smi=0 ; smi<pObjMesh->getNofSubMeshes() ; ++smi)
  {
    const ObjSubMesh* pSrcSubMesh = pObjMesh->getSubMesh(smi);

    mFaces.reserve(pSrcSubMesh->getNofFaces());

    for (std::size_t fid = 0 ; fid<pSrcSubMesh->getNofFaces() ; ++fid)
    {
      ObjConstFaceHandle srcFace = pSrcSubMesh->getFace(fid);
      mFaces.push_back(Vector3i(srcFace.vPositionId(0), srcFace.vPositionId(1), srcFace.vPositionId(2)));
    }
  }

  if(pObjMesh->normals.empty())
    computeNormals();

  return true;
}
