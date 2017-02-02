
#include "bvh.h"
#include "mesh.h"
#include <iostream>

void BVH::build(const Mesh* pMesh, int targetCellSize, int maxDepth)
{
    // store a pointer to the mesh
    m_pMesh = pMesh;
    // allocate the root node
    m_nodes.resize(1);

    if(m_pMesh->nbFaces() <= targetCellSize) { // only one node
        m_nodes[0].box = pMesh->AABB();
        m_nodes[0].first_face_id = 0;
        m_nodes[0].is_leaf = true;
        m_nodes[0].nb_faces = m_pMesh->nbFaces();
        m_faces.resize(m_pMesh->nbFaces());
        for(int i=0; i<m_pMesh->nbFaces(); ++i)
        {
            m_faces[i] = i;
        }
    }else{
        // reserve space for other nodes to avoid multiple memory reallocations
        m_nodes.reserve( std::min<int>(2<<maxDepth, std::log(m_pMesh->nbFaces()/targetCellSize) ) );

        // compute centroids and initialize the face list
        m_centroids.resize(m_pMesh->nbFaces());
        m_faces.resize(m_pMesh->nbFaces());
        for(int i=0; i<m_pMesh->nbFaces(); ++i)
        {
            m_centroids[i] = (m_pMesh->vertexOfFace(i, 0).position + m_pMesh->vertexOfFace(i, 1).position + m_pMesh->vertexOfFace(i, 2).position)/3.f;
            m_faces[i] = i;
        }
        // recursively build the BVH, starting from the root node and the entire list of faces
        buildNode(0, 0, m_pMesh->nbFaces(), 0, targetCellSize, maxDepth);
    }
}

bool BVH::intersect(const Ray& ray, Hit& hit) const
{
    // compute the intersection with the root node
    float tMin, tMax;
    Normal3f n;

    if (::intersect(ray, m_nodes[0].box, tMin, tMax, n) && tMax >0 && tMin < hit.t()) {
      return intersectNode(0, ray, hit);
    }
    return false;
}

bool BVH::intersectNode(int nodeId, const Ray& ray, Hit& hit) const
{
    Node node = m_nodes[nodeId];
    bool ret = false;

    if (node.is_leaf) {
      for (int i = node.first_face_id; i < node.first_face_id+node.nb_faces; i++) {
        if(m_pMesh->intersectFace(ray, hit, m_faces[i])) {
          return true;
        }
      }
    }
    else {
      float tMin, tMax;
      Normal3f n;

      int left = node.first_child_id;
      int right = left+1;
      if (::intersect(ray, m_nodes[left].box, tMin, tMax, n) && tMin < hit.t() && tMax>0) {
        if(intersectNode(left, ray, hit))
          ret = true;
      }
      if (::intersect(ray, m_nodes[right].box, tMin, tMax, n) && tMin < hit.t() && tMax>0) {
        if(intersectNode(right, ray, hit))
          ret = true;
      }
    }
    return ret;
}

/** Sorts the faces with respect to their centroid along the dimension \a dim and spliting value \a split_value.
  * \returns the middle index
  */
int BVH::split(int start, int end, int dim, float split_value)
{
    int l(start), r(end-1);
    while(l<r)
    {
        // find the first on the left
        while(l<end && m_centroids[l](dim) < split_value) ++l;
        while(r>=start && m_centroids[r](dim) >= split_value) --r;
        if(l>r) break;
        std::swap(m_centroids[l], m_centroids[r]);
        std::swap(m_faces[l], m_faces[r]);
        ++l;
        --r;
    }
    return m_centroids[l][dim]<split_value ? l+1 : l;
}

void BVH::buildNode(int nodeId, int start, int end, int level, int targetCellSize, int maxDepth)
{
    Node& node = m_nodes[nodeId];
    node.nb_faces = end-start;

    // étape 1 : calculer la boite englobante des faces indexées de mFaces[start] à mFaces[end]
    // (Utiliser la fonction extend de Eigen::AlignedBox3f et la fonction mpMesh->vertexOfFace(int) pour obtenir les coordonnées des sommets des faces)
    for (int i = start; i < end; i++) {
      for (int j = 0; j < 3; j++) {
        node.box.extend(m_pMesh->vertexOfFace(m_faces[i], j).position);
      }
    }

    // étape 2 : déterminer si il s'agit d'une feuille (appliquer les critères d'arrêts)
    if (level == maxDepth ||  node.nb_faces < targetCellSize) {
      // Si c'est une feuille, finaliser le noeud et quitter la fonction
      node.is_leaf = true;
      node.first_face_id = start;
    }
    else {
      // Si c'est un noeud interne :
      // étape 3 : calculer l'index de la dimension (x=0, y=1, ou z=2) et la valeur du plan de coupe
      // (on découpe au milieu de la boite selon la plus grande dimension)
      Vector3f size = node.box.sizes();
      int dim;
      float splitValue;

      if (size(0) > size(1) && size(0) > size(2)) { // size 0
        dim = 0;
      }
      else if (size(1) > size(0) && size(1) > size(2)) { // size 1
        dim = 1;
      }
      else { // size 2
        dim = 2;
      }
      splitValue = (size(dim)/2)+node.box.min()(dim);

      // étape 4 : appeler la fonction split pour trier (partiellement) les faces
      int mid = split(start, end, dim, splitValue);

      // étape 5 : allouer les fils, et les construire en appelant buildNode...
      Node left, right;
      int id = m_nodes.size();
      node.first_child_id = id;
      m_nodes.push_back(left);
      m_nodes.push_back(right);
      buildNode(id, start, mid, level+1, targetCellSize, maxDepth);
      buildNode(id+1, mid, end, level+1, targetCellSize, maxDepth);
    }
}
