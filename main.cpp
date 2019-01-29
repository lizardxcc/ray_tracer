#include <iostream>
#include <random>
#include "vec3.h"
#include "ray.h"
#include "camera.h"
#include "sphere.h"
#include "plane.h"
#include "hitablelist.h"
#include "material.h"
#include "object.h"





vec3 color(const ray& r, hitable *world, int count)
{
	hit_record rec;
	if (world->hit(r, 0.001, MAXFLOAT, rec)) {
		ray scatterd;
		vec3 attenuation;
		//vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
		vec3 emitted = rec.mat_ptr->emitted(0, 0, rec.p);

		if (count < 10 && rec.mat_ptr->scatter(r, rec, attenuation, scatterd)) {
			return emitted + attenuation*color(scatterd, world, count+1);
		} else {
			return emitted;
		}
	} else {
		return vec3(0, 0, 0);
	}
}



int main(void)
{
	int nx = 300;
	int ny = 300;

	int ns = 5000;

	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	hitable *list[6];
	float size = 1.0;
	float reflection = 0.90;
	list[0] = new plane(vec3(0, -size, 0), vec3(0, 1, 0), new lambertian(vec3(reflection, reflection, reflection)));
	list[1] = new plane(vec3(0, size, 0), vec3(0, -1, 0), new lambertian(vec3(reflection, reflection, reflection)));
	list[2] = new plane(vec3(size, 0, 0), vec3(-1, 0, 0), new lambertian(vec3(0.0, reflection, 0.0)));
	list[3] = new plane(vec3(-size, 0, 0), vec3(1, 0, 0), new lambertian(vec3(reflection, 0.0, 0.0)));
	list[4] = new plane(vec3(0, 0, -size), vec3(0, 0, 1), new lambertian(vec3(reflection, reflection, reflection)));
	//list[5] = new plane(vec3(0, 0, size), vec3(0, 0, -1), new lambertian(vec3(1.0, 1.0, 1.0)));
	list[5] = new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.4, 0.4, new diffuse_light(vec3(20.0, 20.0, 20.0)));

	//list[6] = new plane(vec3(0, size-0.1, 0), vec3(0, -1, 0), new diffuse_light(vec3(0.0, size-0.1, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 1.0, 1.0));

	//list[0] = new sphere(vec3(0, 0, -3), 0.5, new lambertian(vec3(0.8, 0.3, 0.3)));
	////list[1] = new sphere(vec3(0, -1000.5, -1), 1000, new lambertian(vec3(0.8, 0.8, 0.0)));
	//list[1] = new plane(vec3(0, -0.5, 0), vec3(0, 1, 0), new lambertian(vec3(0.5, 0.8, 0.7)));
	////list[2] = new sphere(vec3(1.2, 0, -1), 0.5, new metal(vec3(0.8, 0.8, 0.8), 0.0));
	////list[2] = new sphere(vec3(0.8, 0, -1), 0.5, new dielectric(2.42));
	//list[2] = new sphere(vec3(2.8, 0, -3), 0.5, new metal(vec3(0.6, 0.6, 0.6), 0.03));
	//list[3] = new sphere(vec3(0.4, 0, -5), 0.5, new metal(vec3(0.6, 0.6, 0.6), 0.0));
	//list[4] = new sphere(vec3(-0.8, 0, -2), 0.5, new dielectric(2.42));
	//list[5] = new sphere(vec3(-0.8, 0, -4), 0.5, new metal(vec3(0.4, 0.2, 0.8), 0.0));
	//list[6] = new sphere(vec3(1.0, 1, -1), 0.20, new metal(vec3(0.8, 0.8, 0.8), 0.03));
	//list[7] = new sphere(vec3(-1.8, 0.0, -1.5), 0.5, new dielectric(1.2));
	hitable *world = new hitable_list(list, 6);
	camera cam;


	for (int j = ny-1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {
			vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++) {
				float u = float(i + drand48()) / float(nx);
				float v = float(j + drand48()) / float(ny);
				ray r = cam.get_ray(u, v);
				col += color(r, world, 0);
			}

			col /= float(ns);
			col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));

			int ir = int(255.99*col[0]);
			int ig = int(255.99*col[1]);
			int ib = int(255.99*col[2]);

			std::cout << ir << " " << ig << " " << ib << "\n";
		}
	}
	return 0;
}
