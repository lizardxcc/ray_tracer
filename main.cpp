#include <iostream>
#include <random>
#include "vec3.h"
#include "ray.h"
#include "camera.h"
#include "sphere.h"
#include "hitablelist.h"
#include "material.h"





vec3 color(const ray& r, hitable *world, int count)
{
	hit_record rec;
	if (world->hit(r, 0.001, MAXFLOAT, rec)) {
		ray scatterd;
		vec3 attenuation;

		if (count < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scatterd)) {
			return attenuation*color(scatterd, world, count+1);
		} else {
			return vec3(0, 0, 0);
		}
	} else {
		vec3 unit_direction = unit_vector(r.direction());
		float t = 0.5 * (unit_direction.y() + 1.0);
		return (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
	}
}



int main(void)
{
	int nx = 800;
	int ny = 400;

	int ns = 100;

	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	hitable *list[3];
	list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.8, 0.3, 0.3)));
	list[1] = new sphere(vec3(0, -10000.5, -1), 10000, new lambertian(vec3(0.8, 0.8, 0.0)));
	list[2] = new sphere(vec3(1.2, 0, -1), 0.5, new metal(vec3(0.8, 0.8, 0.8)));
	hitable *world = new hitable_list(list, 3);
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
