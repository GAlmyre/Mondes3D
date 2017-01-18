#include "integrator.h"
#include "scene.h"

class FlatIntegrator : public Integrator {
public:
    FlatIntegrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {
      
        Hit *h = new Hit();
        scene->intersect(ray, *h);

        if (h->shape() == NULL) {
          return scene->backgroundColor();
        }
        else {
          return h->shape()->material()->ambientColor();
        }

        delete h;
    }

    std::string toString() const {
        return "FlatIntegrator[]";
    }
};

REGISTER_CLASS(FlatIntegrator, "flat")
