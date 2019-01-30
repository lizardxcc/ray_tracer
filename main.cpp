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

#ifdef _OPENMP
#include <omp.h>
#endif





vec3 color(const ray& r, hitable *world, int count)
{
	hit_record rec;
	if (world->hit(r, 0.001, MAXFLOAT, rec)) {
		ray scatterd;
		vec3 attenuation;
		//vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
		vec3 emitted = rec.mat_ptr->emitted(0, 0, rec.p);

		if (count < 20 && rec.mat_ptr->scatter(r, rec, attenuation, scatterd)) {
			return emitted + attenuation*color(scatterd, world, count+1);
		} else {
			return emitted;
		}
	} else {
		return vec3(0.0, 0.0, 0.0);
	}
}



int main(void)
{
	int nx = 800;
	int ny = 800;

	int ns = 100;

	vec3 array[nx][ny];

	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	std::vector<hitable*> list;
	float size = 1.0;
	float reflection = 0.90;
	//list.push_back(new plane(vec3(0, -size, 0), vec3(0, 1, 0), new lambertian(vec3(reflection, reflection, reflection))));
	//list.push_back(new plane(vec3(0, -size, 0), vec3(0, 1, 0), new lambertian(vec3(0.9, 0.5, 0.4))));
	list.push_back(new rectangle(vec3(0, -size+0.01, 0), vec3(0, 1, 0), vec3(-1, 0, 0), 2.0, 2.0, new lambertian(vec3(reflection, reflection, reflection))));
	list.push_back(new rectangle(vec3(0, size, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 2.0, 2.0, new lambertian(vec3(reflection, reflection, reflection))));
	//list.push_back(new plane(vec3(0, size, 0), vec3(0, -1, 0), new lambertian(vec3(reflection, reflection, reflection))));

	//list.push_back(new rectangle(vec3(size, 0, 0), vec3(-1, 0, 0), vec3(0, 1, 0), 2.0, 2.0, new lambertian(vec3(0, reflection, 0))));
	list.push_back(new rectangle(vec3(size, 0, 0), vec3(-1, 0, 0), vec3(0, 1, 0), 2.0, 2.0, new lambertian(vec3(0.6, 0.5, 0.3))));


	float window_size = 1.0;
	float a = (2.0-window_size);
	list.push_back(new rectangle(vec3(-size, window_size/2+a/4, 0), vec3(1, 0, 0), vec3(0, 1, 0), a/2, 2.0, new lambertian(vec3(reflection, reflection, reflection))));
	list.push_back(new rectangle(vec3(-size, -window_size/2-a/4, 0), vec3(1, 0, 0), vec3(0, 1, 0), a/2, 2.0, new lambertian(vec3(reflection, reflection, reflection))));
	list.push_back(new rectangle(vec3(-size, 0, -window_size/2-a/4), vec3(1, 0, 0), vec3(0, 1, 0), window_size, a/2, new lambertian(vec3(reflection, reflection, reflection))));
	list.push_back(new rectangle(vec3(-size, 0, window_size/2+a/4), vec3(1, 0, 0), vec3(0, 1, 0), window_size, a/2, new lambertian(vec3(reflection, reflection, reflection))));
	//list.push_back(new rectangle(vec3(-size, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0), 0.6, 0.6, new dielectric(2.0)));


	//list.push_back(new plane(vec3(-size, 0, 0), vec3(1, 0, 0), new dielectric(2.0)));
	list.push_back(new rectangle(vec3(0, 0, -size), vec3(0, 0, 1), vec3(-1, 0, 0), 2.0, 2.0, new lambertian(vec3(reflection, reflection, reflection))));



	//list.push_back(new rectangle(vec3(-size, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0), 2.0, 2.0, new lambertian(vec3(reflection, reflection, reflection))));
	//list.push_back(new plane(vec3(size, 0, 0), vec3(-1, 0, 0), new lambertian(vec3(0.0, reflection, 0.0))));
	////list.push_back(new plane(vec3(-size, 0, 0), vec3(1, 0, 0), new lambertian(vec3(reflection, 0.0, 0.0))));

	//list.push_back(new sphere(vec3(-7.0, 5.0, 5.3), 1.0, new diffuse_light(vec3(260.0, 250.0, 250.0))));
	//list.push_back(new plane(vec3(0, -size, 0), vec3(0, 1, 0), new lambertian(vec3(reflection, reflection, reflection))));

	//list.push_back(new plane(vec3(0, 0, -size), vec3(0, 0, 1), new lambertian(vec3(reflection, reflection, reflection))));
	//list.push_back(new plane(vec3(0, 0, size), vec3(0, 0, -1), new lambertian(vec3(1.0, 1.0, 1.0))));
	//list.push_back(new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.4, 0.4, new diffuse_light(vec3(20.0, 20.0, 20.0))));
	list.push_back(new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.4, 0.4, new diffuse_light(vec3(5.0, 5.0, 5.0))));
	list.push_back(new sphere(vec3(0.4, -0.6, 0.4), 0.3, new dielectric(1.42)));
	list.push_back(new sphere(vec3(-0.4, -0.6, -0.4), 0.3, new metal(vec3(0.8, 0.8, 0.8), 0.0)));
	//list.push_back(new rectangle(vec3(0, 0, -size+0.3), vec3(0, 0, 1), vec3(-1, 0, 0), 1.8, 1.8, new metal(vec3(0.99, 0.99, 0.99), 0.000)));

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
	hitable *world = new hitable_list(list);
	camera cam(vec3(0.0, 0.0, 2.5), vec3(0.0, 0.0, 0.0), vec3(0, 1, 0), 60.0, 1.0);


int i, j, s;
#ifdef _OPENMP
#pragma omp parallel for private(j, s)
#endif
	for (i = 0; i < nx; i++) {
		for (j = 0; j < ny; j++) {
			vec3 col(0, 0, 0);
			for (s = 0; s < ns; s++) {
				float u = float(i + drand48()) / float(nx);
				float v = float(j + drand48()) / float(ny);
				ray r = cam.get_ray(u, v);
				col += color(r, world, 0);
			}

			array[i][j] = col;

		}
	}

	for (int j = ny-1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {
			array[i][j] /= float(ns);
			array[i][j] = vec3(sqrt(array[i][j][0]), sqrt(array[i][j][1]), sqrt(array[i][j][2]));

			int ir = int(255.99*array[i][j][0]);
			int ig = int(255.99*array[i][j][1]);
			int ib = int(255.99*array[i][j][2]);

			std::cout << ir << " " << ig << " " << ib << "\n";
		}
	}
	return 0;
}
