#include "integrator.h"
#include "scene.h"

class DirectIntegrator : public Integrator {
public:
    DirectIntegrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {

        Hit *h = new Hit();
        Hit *shadowHit = new Hit();
        scene->intersect(ray, *h);
        Color3f color = Color3f();

        if (h->shape() == NULL) {
          return scene->backgroundColor();
        }
        else {
          // for every light, get the color
          for (int i = 0; i < scene->lightList().size(); i++) {

            Vector3f lightDir = scene->lightList()[i]->direction(ray.at(h->t()));
            Color3f cosTerm = std::fmaxf(h->normal().dot(lightDir), 0);
            Color3f ro = h->shape()->material()->brdf(-ray.direction,lightDir,h->normal(),NULL);

            float* dist;
            // ombre portée
            Ray shadowRay = Ray(ray.at(h->t())-Epsilon*h->normal(),scene->lightList()[i]->direction(ray.at(h->t()), dist));

            scene->intersect(shadowRay, *shadowHit);

            // if intersection => shadow
            if (!shadowHit->foundIntersection() && shadowHit->t()<) {
                color += ro*cosTerm*scene->lightList()[i]->intensity(ray.at(h->t()));
            }
          }
        }
        delete h;
        delete shadowHit;

        return color;
    }

    std::string toString() const {
        return "DirectIntegrator[]";
    }
};

REGISTER_CLASS(DirectIntegrator, "direct")
