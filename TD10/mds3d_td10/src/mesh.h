#ifndef MESH_H
#define MESH_H

#include <surface_mesh/surface_mesh.h>
#include <string>
#include <vector>

class Shader;

class Mesh
{
    typedef Eigen::Matrix<float,2,1,Eigen::DontAlign> Vector2f;
    typedef Eigen::Vector3f Vector3f;
    typedef Eigen::Matrix<float,4,1,Eigen::DontAlign> Vector4f;
    typedef Eigen::Vector3i Vector3i;

    struct Vertex
    {
      Vertex(const Vector3f& pos = Vector3f::Zero(),
             const Vector3f& n = Vector3f::Zero(),
             const Vector4f& c = Vector4f(0.6,0.6,0.6,1.0),
             const Vector2f& uv = Vector2f::Zero()
            )
        : position(pos), normal(n), color(c), texcoord(uv)
      {}

      Vector3f position;
      Vector3f normal;
      Vector4f color;
      Vector2f texcoord;
    };

public:
    Mesh() {}
    ~Mesh();

    /** load a triangular mesh from the file \a filename (.off or .obj) */
    bool load(const std::string& filename);

    void subdivide();

    /** initialize OpenGL's Vertex Buffer Array (must be called once before calling draw()) */
    void init();

    /** Send the mesh to OpenGL for drawing using shader \a shd */
    void draw(const Shader& shd);

    /// returns a reference to the surface_mesh data structure
    surface_mesh::Surface_mesh& halfEdgeMesh() { return mHalfEdge; }

    /// returns a const reference to the surface_mesh data structure
    const surface_mesh::Surface_mesh& halfEdgeMesh() const { return mHalfEdge; }

    /// Re-compute vertex normals (needs to be called after editing vertex positions)
    void updateNormals();

    /// Copy vertex attributes from the CPU to GPU memory (needs to be called after editing any vertex attributes: positions, normals, texcoords, masks, etc.)
    void updateVBO();

private:

    void updateHalfedgeToMesh();

    /** The list of vertices */
    std::vector<Vertex> mVertices;

    /** The list of face indices */
    std::vector<Vector3i> mFaces;

    unsigned int mVertexArrayId;
    unsigned int mVertexBufferId; ///< the id of the BufferObject storing the vertex attributes
    unsigned int mIndexBufferId;  ///< the id of the BufferObject storing the faces indices
    bool mIsInitialized;
    surface_mesh::Surface_mesh mHalfEdge;
};


#endif // MESH_H
