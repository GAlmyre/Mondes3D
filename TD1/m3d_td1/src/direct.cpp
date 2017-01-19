#include "integrator.h"
#include "scene.h"

class DirectIntegrator : public Integrator {
public:
    DirectIntegrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {

        Hit *h = new Hit();
        scene->intersect(ray, *h);
        Color3f *color = new Color3f();

        if (h->shape() == NULL) {
          return scene->backgroundColor();
        }
        else {
          // for every light, get the color
          for (int i = 0; i < scene->lightList().size(); i++) {
            float dist;
            float lightDir = scene->lightList()[i]->direction(ray.at(h->t()), dist);
            float cosTerm = std::max(lightDir.dot(h->normal()), 0);
            float ro = h->shape()->material()->brdf(-ray.direction(), lightDir, h->normal(), NULL);

            color += ro*cosTerm*scene->lightList()[i].intensity();
          }
        }
        delete h;
        return color;
    }

    std::string toString() const {
        return "DirectIntegrator[]";
    }
};

REGISTER_CLASS(DirectIntegrator, "direct")
