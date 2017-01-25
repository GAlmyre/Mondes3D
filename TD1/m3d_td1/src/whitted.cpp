#include "integrator.h"
#include "scene.h"

class WhittedIntegrator : public Integrator {
public:

    int m_maxRecursion;

    WhittedIntegrator(const PropertyList &props) {
        m_maxRecursion = props.getInteger("m_maxRecursion",4);
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {

        Hit *h = new Hit();
        scene->intersect(ray, *h);
        Color3f color = Color3f();

        if (!h->foundIntersection()) {
          return scene->backgroundColor();
        }
        const LightList &list = scene->lightList();

        // for every light, get the color
        for (LightList::const_iterator it = list.begin(); it != list.end(); it++) {
          Hit *shadowHit = new Hit();
          float dist;
          Vector3f lightDir = (*it)->direction(ray.at(h->t()), &dist);
          float cosTerm = std::max(lightDir.dot(h->normal()), 0.f);
          Color3f ro = h->shape()->material()->brdf(-ray.direction,lightDir,h->normal(),NULL);

          // shadow
          Ray shadowRay = Ray(ray.at(h->t())+h->normal()*Epsilon,lightDir);
          scene->intersect(shadowRay, *shadowHit);

          // if intersection => shadow
          if (!shadowHit->foundIntersection() || shadowHit->t() > dist) {
              color += ro*cosTerm*(*it)->intensity(ray.at(h->t()));
          }
          delete shadowHit;
        }

        // reflexion
        if (h->shape()->reflectivity() > 0 || ray.recursionLevel >= m_maxRecursion) {

          Ray reflectRay = Ray(ray.at(h->t()), );
        }

        delete h;

        return color;
    }

    std::string toString() const {
        return "WhittedIntegrator[]";
    }
};

REGISTER_CLASS(WhittedIntegrator, "whitted")
