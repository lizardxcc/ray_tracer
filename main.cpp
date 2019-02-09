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
		float pdf;

		if (count < 20 && rec.mat_ptr->scatter(r, rec, attenuation, scatterd, pdf)) {
			//return emitted + attenuation*color(scatterd, world, count+1);
			return emitted + attenuation*rec.mat_ptr->BxDF(r, rec, scatterd)*color(scatterd, world, count+1) * dot(rec.normal, unit_vector(scatterd.direction()))/ pdf;
		} else {
			return emitted;
		}
	} else {
		return vec3(0.0, 0.0, 0.0);
	}
}



hitable *room(void)
{
	std::vector<hitable *> list;

	float size = 1.0;
	float reflection = 0.90;
	list.push_back(new rectangle(vec3(0, -size+0.01, 0), vec3(0, 1, 0), vec3(-1, 0, 0), 2.0, 2.0, new lambertian(vec3(reflection, reflection, reflection))));
	list.push_back(new rectangle(vec3(0, size, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 2.0, 2.0, new lambertian(vec3(reflection, reflection, reflection))));
	list.push_back(new rectangle(vec3(size, 0, 0), vec3(-1, 0, 0), vec3(0, 1, 0), 2.0, 2.0, new lambertian(vec3(0.6, 0.5, 0.3))));


	float window_size = 1.0;
	float a = (2.0-window_size);
	list.push_back(new rectangle(vec3(-size, window_size/2+a/4, 0), vec3(1, 0, 0), vec3(0, 1, 0), a/2, 2.0, new lambertian(vec3(reflection, reflection, reflection))));
	list.push_back(new rectangle(vec3(-size, -window_size/2-a/4, 0), vec3(1, 0, 0), vec3(0, 1, 0), a/2, 2.0, new lambertian(vec3(reflection, reflection, reflection))));
	list.push_back(new rectangle(vec3(-size, 0, -window_size/2-a/4), vec3(1, 0, 0), vec3(0, 1, 0), window_size, a/2, new lambertian(vec3(reflection, reflection, reflection))));
	list.push_back(new rectangle(vec3(-size, 0, window_size/2+a/4), vec3(1, 0, 0), vec3(0, 1, 0), window_size, a/2, new lambertian(vec3(reflection, reflection, reflection))));
	list.push_back(new rectangle(vec3(0, 0, -size), vec3(0, 0, 1), vec3(-1, 0, 0), 2.0, 2.0, new lambertian(vec3(reflection, reflection, reflection))));

	//list.push_back(new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.4, 0.4, new diffuse_light(vec3(1.0*6, 0.576*6, 0.1607*6))));
	list.push_back(new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.4, 0.4, new diffuse_light(vec3(1.0*56, 0.576*56, 0.1607*56))));
	//list.push_back(new sphere(vec3(0.4, -0.6, 0.4), 0.3, new dielectric(1.42)));
	list.push_back(new sphere(vec3(-0.4, -0.6, -0.4), 0.3, new metal(vec3(0.8, 0.8, 0.8), 0.0)));

	float box_size = 0.5;
	list.push_back(new translate(new box(vec3(0, 0, 0), vec3(box_size, box_size*2, box_size), new dielectric(1.5)), vec3(0.4, -0.5, -0.8)));

	//list.push_back(new xy_rect(0, 0, 0.5, 0.5, -0.5, new diffuse_light(vec3(1.0*5, 0.576*5, 0.1607*5))));

	return new hitable_list(list);
}

int main(void)
{
	int nx = 400;
	int ny = 400;

	int ns = 300;

	vec3 array[nx][ny];

	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	//std::vector<hitable*> list;
	//float size = 1.0;
	//float reflection = 0.90;
	//list.push_back(new rectangle(vec3(0, -size+0.01, 0), vec3(0, 1, 0), vec3(-1, 0, 0), 2.0, 2.0, new lambertian(vec3(reflection, reflection, reflection))));
	//list.push_back(new rectangle(vec3(0, size, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 2.0, 2.0, new lambertian(vec3(reflection, reflection, reflection))));
	//list.push_back(new rectangle(vec3(size, 0, 0), vec3(-1, 0, 0), vec3(0, 1, 0), 2.0, 2.0, new lambertian(vec3(0.6, 0.5, 0.3))));


	//float window_size = 1.0;
	//float a = (2.0-window_size);
	//list.push_back(new rectangle(vec3(-size, window_size/2+a/4, 0), vec3(1, 0, 0), vec3(0, 1, 0), a/2, 2.0, new lambertian(vec3(reflection, reflection, reflection))));
	//list.push_back(new rectangle(vec3(-size, -window_size/2-a/4, 0), vec3(1, 0, 0), vec3(0, 1, 0), a/2, 2.0, new lambertian(vec3(reflection, reflection, reflection))));
	//list.push_back(new rectangle(vec3(-size, 0, -window_size/2-a/4), vec3(1, 0, 0), vec3(0, 1, 0), window_size, a/2, new lambertian(vec3(reflection, reflection, reflection))));
	//list.push_back(new rectangle(vec3(-size, 0, window_size/2+a/4), vec3(1, 0, 0), vec3(0, 1, 0), window_size, a/2, new lambertian(vec3(reflection, reflection, reflection))));
	//list.push_back(new rectangle(vec3(0, 0, -size), vec3(0, 0, 1), vec3(-1, 0, 0), 2.0, 2.0, new lambertian(vec3(reflection, reflection, reflection))));

	////list.push_back(new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.4, 0.4, new diffuse_light(vec3(1.0*6, 0.576*6, 0.1607*6))));
	//list.push_back(new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.4, 0.4, new diffuse_light(vec3(1.0*16, 0.576*16, 0.1607*16))));
	//list.push_back(new sphere(vec3(0.4, -0.6, 0.4), 0.3, new dielectric(1.42)));
	//list.push_back(new sphere(vec3(-0.4, -0.6, -0.4), 0.3, new metal(vec3(0.8, 0.8, 0.8), 0.0)));

	//float box_size = 0.4;
	//list.push_back(new translate(new box(vec3(0, 0, 0), vec3(box_size, box_size, box_size), new lambertian(vec3(1.0, 0.9, 0.8))), vec3(0.3, -0.5, -0.5)));

	////list.push_back(new xy_rect(0, 0, 0.5, 0.5, -0.5, new diffuse_light(vec3(1.0*5, 0.576*5, 0.1607*5))));

	//hitable *world = new hitable_list(list);
	hitable *world = room();
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
