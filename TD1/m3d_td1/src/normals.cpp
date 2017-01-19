#include "integrator.h"
#include "scene.h"

class NormalsIntegrator : public Integrator {
public:
    NormalsIntegrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {

        Hit *h = new Hit();
        scene->intersect(ray, *h);

        if (h->shape() == NULL) {
          return scene->backgroundColor();
        }
        else {
          Normal3f normal = h->normal().cwiseAbs();
          Color3f color = Color3f(normal[0], normal[1], normal[2]);
          return color;
        }
        delete h;
    }

    std::string toString() const {
        return "NormalsIntegrator[]";
    }
};

REGISTER_CLASS(NormalsIntegrator, "normals")
