#include "viewer.h"

#include <filesystem/resolver.h>

void render(Scene* scene, ImageBlock* result, std::string outputName, bool* done)
{
    if(!scene)
        return;

    clock_t t = clock();

    const Camera *camera = scene->camera();
    const Integrator* integrator = scene->integrator();
    integrator->preprocess(scene);

    float tanfovy2 = tan(camera->fovY()*0.5);
    //Vector3f camX = camera->right() * 0.5 * camera->vpWidth();
    //Vector3f camY = camera->up() * 0.5 * camera->vpHeight();
    Vector3f camX = camera->right()*tanfovy2*camera->nearDist()*float(camera->vpWidth())/float(camera->vpHeight());
    Vector3f camY = -camera->up()*tanfovy2*camera->nearDist();
    Vector3f camF = camera->direction() * camera->nearDist();

	for (int i = 0; i < camera->vpHeight(); i++)
	{
		for (int j = 0; j < camera->vpWidth(); j++)
		{
			//Vector3f v = camF + ((-camX + (camX/(camera->vpWidth()/2))*i)+0.5) + ((camY - (camY/(camera->vpHeight()/2))*j)+0.5);
			Vector3f X = (2*(i+0.5)/camera->vpWidth()-1)*camX;
			Vector3f Y = (2*(i+0.5)/camera->vpHeight()-1)*camY;
			Vector3f v = camF+X+Y;
			v.normalize();
			Hit h = Hit();
      const Ray r =  Ray(camera->position(), v);
			scene->intersect(r, h);

      // stores the color
      result->put(Vector2f(i,j), integrator->Li(scene, r));
		}
	}


    /// TODO:
    ///  1. iterate over the image pixels
    ///  2. generate a primary ray
    ///  3. call the integartor to compute the color along this ray
    ///  4. write this color in the result image

    t = clock() - t;
    std::cout << "Raytracing time : " << float(t)/CLOCKS_PER_SEC << "s"<<std::endl;

    Bitmap* img = result->toBitmap();
    img->save(outputName);
    delete img;
    *done = true;
}

int main(int argc, char *argv[])
{
    getFileResolver()->prepend(DATA_DIR);

    try {
        nanogui::init();
        Viewer *screen = new Viewer();;

        if (argc == 2) {
            /* load file from the command line */
            filesystem::path path(argv[1]);

            if(path.extension() == "scn") { // load scene file
                screen->loadScene(argv[1]);
            }else if(path.extension() == "exr") { // load OpenEXR image
                screen->loadImage(argv[1]);
            }
        }

        /* Enter the application main loop */
        nanogui::mainloop();

        delete screen;
        nanogui::shutdown();
    } catch (const std::exception &e) {
        cerr << "Fatal error: " << e.what() << endl;
        return -1;
    }
    return 0;
}
