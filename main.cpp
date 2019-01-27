#include <iostream>
#include <random>
#include "vec3.h"
#include "ray.h"
#include "camera.h"
#include "sphere.h"
#include "hitablelist.h"




float hit_sphere(const vec3& center, float radius, const ray& r)
{
	return 0.0;
}

vec3 random_in_unit_sphere(void)
{
	//std::random_device rnd;
	vec3 p;
	do {
		p = 2.0*vec3(drand48(), drand48(), drand48()) - vec3(1, 1, 1);
	} while (p.squared_length() >= 1.0);

	//std::cout << p << std::endl;

	return p;
}

vec3 color(const ray& r, hitable *world, int count)
{
	if (count >= 20) {

		vec3 unit_direction = unit_vector(r.direction());
		float t = 0.5 * (unit_direction.y() + 1.0);
		return (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
	}
	hit_record rec;
	if (world->hit(r, 0.0, MAXFLOAT, rec)) {
		vec3 target = rec.p + rec.normal + random_in_unit_sphere();
		return 0.5 * color(ray(rec.p, target-rec.p), world, count+1);
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

	hitable *list[2];
	list[0] = new sphere(vec3(0, 0, -1), 0.5);
	list[1] = new sphere(vec3(0, -100.5, -1), 100);
	hitable *world = new hitable_list(list, 2);
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
			//vec3 col(float(i) / float(nx), float(j) / float(ny), 0.2);
			int ir = int(255.99*col[0]);
			int ig = int(255.99*col[1]);
			int ib = int(255.99*col[2]);

			std::cout << ir << " " << ig << " " << ib << "\n";
		}
	}
	return 0;
}
