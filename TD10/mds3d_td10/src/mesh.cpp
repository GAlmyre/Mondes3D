#include "mesh.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include "shader.h"
#include <Eigen/Geometry>
#include <limits>

using namespace std;
using namespace Eigen;
using namespace surface_mesh;

void Mesh::subdivide()
{

  // etape 3
  Surface_mesh newMesh;
  Surface_mesh::Vertex_property<Point> vertices = mHalfEdge.get_vertex_property<Point>("v:point");
  Surface_mesh::Vertex_property<Surface_mesh::Vertex> vertex_mapping = mHalfEdge.add_vertex_property<Surface_mesh::Vertex>("v:mapping");
  // reference : Connectivity queries
  Surface_mesh::Vertex_around_vertex_circulator vc, vc_end;
  Surface_mesh::Vertex_iterator vit;
  Surface_mesh::Edge_iterator edge;
  Surface_mesh::Face_iterator face;
  int di;

  for (vit = mHalfEdge.vertices_begin(); vit != mHalfEdge.vertices_end(); ++vit) {
    Vector3f c = Vector3f::Zero();
    float beta;
    // initialize circulators
    vc = mHalfEdge.vertices(*vit);
    vc_end = vc;
    // reset counter
    di = 0;

    do {
      di++;
      c += vertices[*vc];
    } while(++vc != vc_end);

    if (di == 3) {
      beta = 3./16;
    }
    else if (di > 3) {
      beta = (1./di)*(5./8-(3./8+1./4*cos((2*M_PI)/di))*(3./8+1./4*cos((2*M_PI)/di)));
    }
    Vector3f P = (1-beta*di)*vertices[*vit]+beta*c;
    vertex_mapping[*vit] = newMesh.add_vertex(P);
  }

  // etape 1
  Surface_mesh::Edge_property<Surface_mesh::Vertex> edge_mapping = mHalfEdge.add_edge_property<Surface_mesh::Vertex>("e:mapping");

  for (edge = mHalfEdge.edges_begin(); edge != mHalfEdge.edges_end(); ++edge) {
    Vector3f newPos = Vector3f::Zero();
    Surface_mesh::Halfedge half = mHalfEdge.halfedge(*edge, 0);
    Surface_mesh::Halfedge half2 = mHalfEdge.halfedge(*edge, 1);

    Surface_mesh::Vertex   v0 = mHalfEdge.to_vertex(half);
    Surface_mesh::Vertex   v2 = mHalfEdge.to_vertex(half2);

    Surface_mesh::Halfedge h0 = mHalfEdge.next_halfedge(half);
    Surface_mesh::Halfedge h1 = mHalfEdge.next_halfedge(half2);

    Surface_mesh::Vertex   v1 = mHalfEdge.to_vertex(h0);
    Surface_mesh::Vertex   v3 = mHalfEdge.to_vertex(h1);

    newPos = (3./8)*vertices[v0]+(1./8)*vertices[v1]+(3./8)*vertices[v2]+(1./8)*vertices[v3];
    edge_mapping[*edge] = newMesh.add_vertex(newPos);
  }

  for (face = mHalfEdge.faces_begin(); face != mHalfEdge.faces_end(); ++face) {

    Surface_mesh::Halfedge half = mHalfEdge.halfedge(*face);
    Surface_mesh::Vertex v0,v1,v2,u1,u3,u5;
    v0 = vertex_mapping[mHalfEdge.from_vertex(half)];
    u1 = edge_mapping[mHalfEdge.edge(half)];
    v2 = vertex_mapping[mHalfEdge.to_vertex(half)];
    half = mHalfEdge.next_halfedge(half);
    u3 = edge_mapping[mHalfEdge.edge(half)];
    v1 = vertex_mapping[mHalfEdge.to_vertex(half)];
    half = mHalfEdge.next_halfedge(half);
    u5 = edge_mapping[mHalfEdge.edge(half)];

    newMesh.add_triangle(u1,u5,v0);
    newMesh.add_triangle(u3,v1,u5);
    newMesh.add_triangle(u1,v2,u3);
    newMesh.add_triangle(u3,u5,u1);
  }

  mHalfEdge = newMesh;
  updateHalfedgeToMesh();
  updateVBO();
}

Mesh::~Mesh()
{
  if(mIsInitialized)
  {
    glDeleteBuffers(1,&mVertexBufferId);
    glDeleteBuffers(1,&mIndexBufferId);
    glDeleteVertexArrays(1,&mVertexArrayId);
  }
}


bool Mesh::load(const std::string& filename)
{
    std::cout << "Loading: " << filename << std::endl;

    if(!mHalfEdge.read(filename))
      return false;

    mHalfEdge.update_face_normals();
    mHalfEdge.update_vertex_normals();

    updateHalfedgeToMesh();

    return true;
}

void Mesh::updateHalfedgeToMesh()
{
    // vertex properties
    Surface_mesh::Vertex_property<Point> vertices = mHalfEdge.get_vertex_property<Point>("v:point");
    Surface_mesh::Vertex_property<Point> vnormals = mHalfEdge.get_vertex_property<Point>("v:normal");
    Surface_mesh::Vertex_property<Texture_coordinate> texcoords = mHalfEdge.get_vertex_property<Texture_coordinate>("v:texcoord");
    Surface_mesh::Vertex_property<Color> colors = mHalfEdge.get_vertex_property<Color>("v:color");

    // vertex iterator
    Surface_mesh::Vertex_iterator vit;

    Vector3f pos;
    Vector3f normal;
    Vector2f tex;
    Vector4f color;
    mVertices.clear();
    for(vit = mHalfEdge.vertices_begin(); vit != mHalfEdge.vertices_end(); ++vit)
    {
        pos = vertices[*vit];
        normal = Vector3f(0,0,0);
        if(texcoords)
            tex = texcoords[*vit];
        if(colors)
            color << colors[*vit], 1.0;
        else
            color = Vector4f(0.6,0.6,0.6,1.0);

        mVertices.push_back(Vertex(pos,normal,color,tex));
    }

    // face iterator
    Surface_mesh::Face_iterator fit, fend = mHalfEdge.faces_end();
    // vertex circulator
    Surface_mesh::Vertex_around_face_circulator fvit, fvend;
    Surface_mesh::Vertex v0, v1, v2;
    mFaces.clear();
    for (fit = mHalfEdge.faces_begin(); fit != fend; ++fit)
    {
        fvit = fvend = mHalfEdge.vertices(*fit);
        v0 = *fvit;
        ++fvit;
        v2 = *fvit;

        do{
            v1 = v2;
            ++fvit;
            v2 = *fvit;
            mFaces.push_back(Vector3i(v0.idx(),v1.idx(),v2.idx()));
        } while (++fvit != fvend);
    }

    updateNormals();
}

void Mesh::init()
{
    glGenVertexArrays(1,&mVertexArrayId);
    glGenBuffers(1,&mVertexBufferId);
    glGenBuffers(1,&mIndexBufferId);

    updateVBO();

    mIsInitialized = true;
}

void Mesh::updateNormals()
{
    // pass 1: set the normal to 0
    for(std::vector<Vertex>::iterator v_iter = mVertices.begin() ; v_iter!=mVertices.end() ; ++v_iter)
        v_iter->normal.setZero();

    // pass 2: compute face normals and accumulate
    for(std::size_t j=0; j<mFaces.size(); ++j)
    {
        Vector3f v0 = mVertices[mFaces[j][0]].position;
        Vector3f v1 = mVertices[mFaces[j][1]].position;
        Vector3f v2 = mVertices[mFaces[j][2]].position;

        Vector3f n = (v1-v0).cross(v2-v0).normalized();

        mVertices[mFaces[j][0]].normal += n;
        mVertices[mFaces[j][1]].normal += n;
        mVertices[mFaces[j][2]].normal += n;
    }

    // pass 3: normalize
    for(std::vector<Vertex>::iterator v_iter = mVertices.begin() ; v_iter!=mVertices.end() ; ++v_iter)
        v_iter->normal.normalize();
}

void Mesh::updateVBO()
{
  glBindVertexArray(mVertexArrayId);

  // activate the VBO:
  glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId);
  // copy the data from host's RAM to GPU's video memory:
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*mVertices.size(), mVertices[0].position.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferId);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Vector3i)*mFaces.size(), mFaces[0].data(), GL_STATIC_DRAW);


}


void Mesh::draw(const Shader& shd)
{
    if (!mIsInitialized)
      init();

      // Activate the VBO of the current mesh:
  glBindVertexArray(mVertexArrayId);
  glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferId);

  // Specify vertex data

  // 1 - get id of the attribute "vtx_position" as declared as "in vec3 vtx_position" in the vertex shader
  int vertex_loc = shd.getAttribLocation("vtx_position");
  if(vertex_loc>=0)
  {
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
  }

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

  int texcoord_loc = shd.getAttribLocation("vtx_texcoord");
  if(texcoord_loc>=0)
  {
    glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(Vector3f)+sizeof(Vector4f)));
    glEnableVertexAttribArray(texcoord_loc);
  }

  // send the geometry
  glDrawElements(GL_TRIANGLES, 3*mFaces.size(), GL_UNSIGNED_INT, 0);

  // at this point the mesh has been drawn and raserized into the framebuffer!
  if(vertex_loc>=0)   glDisableVertexAttribArray(vertex_loc);
  if(normal_loc>=0)   glDisableVertexAttribArray(normal_loc);
  if(color_loc>=0)    glDisableVertexAttribArray(color_loc);
  if(texcoord_loc>=0) glDisableVertexAttribArray(texcoord_loc);

  checkError();
}
