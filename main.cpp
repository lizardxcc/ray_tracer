#include <iostream>
#include <algorithm>
#include <random>
#include "vec3.h"
#include "ray.h"
#include "camera.h"
#include "hitablelist.h"
#include "material.h"
#include "object.h"

#ifdef _OPENMP
#include <omp.h>
#endif





vec3 color(const ray& r, hitable *world, int count)
{
	hit_record rec;
	if (world->hit(r, 0.001, std::numeric_limits<float>::max(), rec)) {
		ray scatterd;
		vec3 attenuation;
		//vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
		vec3 emitted = rec.mat_ptr->emitted(0, 0, r, rec);

		float bxdf;
		float pdf;
		if (count < 20 && rec.mat_ptr->sample(r, rec, attenuation, scatterd, bxdf, pdf)) {
			//return emitted + attenuation*color(scatterd, world, count+1);
			return emitted + bxdf*attenuation*color(scatterd, world, count+1) * abs(dot(rec.normal, unit_vector(scatterd.direction())))/ pdf;
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
	list.push_back(new rectangle(vec3(size, 0, 0), vec3(-1, 0, 0), vec3(0, 1, 0), 2.0, 2.0, new lambertian(vec3(0.0, 1.0, 0.0))));
	list.push_back(new rectangle(vec3(-size, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0), 2.0, 2.0, new lambertian(vec3(1.0, 0.0, 0.0))));


	//float window_size = 1.0;
	//float a = (2.0-window_size);
	//list.push_back(new rectangle(vec3(-size, window_size/2+a/4, 0), vec3(1, 0, 0), vec3(0, 1, 0), a/2, 2.0, new lambertian(vec3(reflection, reflection, reflection))));
	//list.push_back(new rectangle(vec3(-size, -window_size/2-a/4, 0), vec3(1, 0, 0), vec3(0, 1, 0), a/2, 2.0, new lambertian(vec3(reflection, reflection, reflection))));
	//list.push_back(new rectangle(vec3(-size, 0, -window_size/2-a/4), vec3(1, 0, 0), vec3(0, 1, 0), window_size, a/2, new lambertian(vec3(reflection, reflection, reflection))));
	//list.push_back(new rectangle(vec3(-size, 0, window_size/2+a/4), vec3(1, 0, 0), vec3(0, 1, 0), window_size, a/2, new lambertian(vec3(reflection, reflection, reflection))));
	list.push_back(new rectangle(vec3(0, 0, -size), vec3(0, 0, 1), vec3(-1, 0, 0), 2.0, 2.0, new lambertian(vec3(reflection, reflection, reflection))));

	//list.push_back(new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.5, 0.5, new diffuse_light(vec3(30, 30, 30))));
	hitable *light = new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.5, 0.5, new diffuse_light(vec3(50, 50, 50)));
	list.push_back(light);
	lambertian mat(vec3(0, 0, 0));
	mat.lights.push_back(light);
	light = new rectangle(vec3(0, size-0.01, 0.7), vec3(0, -1, 0), vec3(-1, 0, 0), 0.5, 0.5, new diffuse_light(vec3(30, 30, 30)));
	list.push_back(light);
	mat.lights.push_back(light);
	//list.push_back(new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.4, 0.4, new diffuse_light(vec3(1.0*6, 0.576*6, 0.1607*6))));
	//list.push_back(new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.4, 0.4, new diffuse_light(vec3(1.0*56, 0.576*56, 0.1607*56))));
	//list.push_back(new sphere(vec3(0.4, -0.6, 0.4), 0.3, new dielectric(1.42)));

	//list.push_back(new sphere(vec3(-0.2, -0.5, -0.2), 0.5, new metal(vec3(0.95, 0.05, 0.85), 0.0)));
	list.push_back(new sphere(vec3(-0.5, -0.5, -0.2), 0.4, new lambertian(vec3(0.90, 0.90, 0.90))));
	list.push_back(new sphere(vec3(0.5, -0.5, -0.2), 0.4, new oren_nayar(vec3(0.90, 0.90, 0.90), 0.5)));
	//list.push_back(new sphere(vec3(0.4, -0.5, 0.4), 0.5, new dielectric(2.4)));

	//float box_size = 0.5;
	//list.push_back(new translate(new box(vec3(0, 0, 0), vec3(box_size, box_size*2, box_size), new dielectric(1.5)), vec3(0.4, -0.5, -0.8)));
	//list.push_back(new translate(new plymodel("human2.ply", new lambertian(vec3(1.0, 1.0, 1.0))), vec3(0.0, -0.2, -0.3)));

	//list.push_back(new translate(new plymodel("smooth_monkey.ply", new lambertian(vec3(1.0, 1.0, 1.0))), vec3(0.0, -0.2, 0.5)));
	//list.push_back(new translate(new plymodel("blender_monkey.ply", new dielectric(1.4)), vec3(0.0, -0.2, 0.5)));
	//hitable *obj = new sphere(vec3(0.0, -0.2, 0.5), 0.7, new dielectric(2.4));
	//mat.lights.push_back(obj);

	//list.push_back(new translate(new plymodel("blender_monkey.ply", new metal(vec3(1.0, 1.0, 1.0), 0.0)), vec3(0.0, -0.2, 0.5)));
	//list.push_back(new translate(new plymodel("human3.ply", new metal(vec3(1.0, 1.0, 1.0), 0.0)), vec3(-0.45, -0.2, 0.5)));
	//list.push_back(new translate(new plymodel("human3.ply", new lambertian(vec3(1.0, 1.0, 1.0))), vec3(0.45, -0.2, 0.5)));

	//list.push_back(new translate(new plymodel("nmonky3.ply", new metal(vec3(0.9, 0.9, 0.9), 0.0)), vec3(0.0, -0.3, -0.3)));

	//list.push_back(new xy_rect(0, 0, 0.5, 0.5, -0.5, new diffuse_light(vec3(1.0*5, 0.576*5, 0.1607*5))));

	return new hitable_list(list);
}


hitable *testroom(void)
{
	float size = 1.0;
	float reflection = 0.90;
	std::vector<hitable *> list;

	//list.push_back(new rectangle(vec3(0, 0, -size), vec3(0, 0, 1), vec3(-1, 0, 0), 2.0, 2.0, new lambertian(vec3(reflection, reflection, reflection))));

	quadrilateral *quad = new quadrilateral();
	quad->v[0] = vec3(-10.0, -1, 10.0);
	quad->v[1] = vec3(10.0, -1, 10.0);
	quad->v[2] = vec3(10.0, -1, -10.0);
	quad->v[3] = vec3(-10.0, -1, -10.0);
	quad->normal = vec3(0.0, 1.0, 0.0);
	quad->mat_ptr = new lambertian(vec3(0.2, 0.2, 0.2));
	list.push_back(quad);

	hitable *light;
	lambertian mat(vec3(0, 0, 0));
	//light = new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.5, 0.5, new diffuse_light(vec3(30, 30, 30)));
	//list.push_back(light);
	//mat.lights.push_back(light);

	//light = new rectangle(vec3(0, size-0.01, 0.7), vec3(0, -1, 0), vec3(-1, 0, 0), 0.5, 0.5, new diffuse_light(vec3(30, 30, 30)));
	//list.push_back(light);
	//mat.lights.push_back(light);

	light = new sphere(vec3(0.0, 10000.2, 0.0), 1000.05, new diffuse_light(vec3(50, 50, 50)));
	list.push_back(light);
	mat.lights.push_back(light);

	quad = new quadrilateral();
	quad->v[0] = vec3(-0.04, 0, 0.04);
	quad->v[1] = vec3(0.04, 0, 0.04);
	quad->v[2] = vec3(0.04, 0, -0.04);
	quad->v[3] = vec3(-0.04, 0, -0.04);
	quad->normal = vec3(0.0, -1.0, 0.0);
	quad->mat_ptr = new straight_light(vec3(300, 0, 0));
	light = new translate(quad, vec3(-0.6, 0.05, 0.2));
	list.push_back(light);
	mat.lights.push_back(light);

	quad = new quadrilateral();
	quad->v[0] = vec3(-0.07, 0, 0.07);
	quad->v[1] = vec3(0.07, 0, 0.07);
	quad->v[2] = vec3(0.07, 0, -0.07);
	quad->v[3] = vec3(-0.07, 0, -0.07);
	quad->normal = vec3(0.0, -1.0, 0.0);
	quad->mat_ptr = new straight_light(vec3(0, 300, 0));
	light = new translate(quad, vec3(0, 0.05, 0.2));
	list.push_back(light);
	mat.lights.push_back(light);

	quad = new quadrilateral();
	quad->v[0] = vec3(-0.1, 0, 0.1);
	quad->v[1] = vec3(0.1, 0, 0.1);
	quad->v[2] = vec3(0.1, 0, -0.1);
	quad->v[3] = vec3(-0.1, 0, -0.1);
	quad->normal = vec3(0.0, -1.0, 0.0);
	quad->mat_ptr = new straight_light(vec3(0, 0, 300));
	light = new translate(quad, vec3(0.6, 0.05, 0.2));
	list.push_back(light);
	mat.lights.push_back(light);
	

	//light = new sphere(vec3(-0.6, 0.05, 0.2), 0.02, new straight_light(vec3(500, 0, 0)));
	//list.push_back(light);
	//mat.lights.push_back(light);
	//light = new sphere(vec3(0.0, 0.05, 0.2), 0.07, new straight_light(vec3(0, 500, 0)));
	//list.push_back(light);
	//mat.lights.push_back(light);
	//light = new sphere(vec3(0.6, 0.05, 0.2), 0.15, new straight_light(vec3(0, 0, 500)));
	//list.push_back(light);
	//mat.lights.push_back(light);

	list.push_back(new translate(new plymodel("testroom.ply", new lambertian(vec3(1.0, 1.0, 1.0))), vec3(0.0, -0.8, 0.3)));


	return new hitable_list(list);
}

hitable *caustics_room(void)
{
	std::vector<hitable *> list;

	quadrilateral *quad = new quadrilateral();
	quad->v[0] = vec3(-10.0, -1, 10.0);
	quad->v[1] = vec3(10.0, -1, 10.0);
	quad->v[2] = vec3(10.0, -1, -10.0);
	quad->v[3] = vec3(-10.0, -1, -10.0);
	quad->normal = vec3(0.0, 1.0, 0.0);
	quad->mat_ptr = new lambertian(vec3(0.7, 0.7, 0.7));
	list.push_back(quad);

	lambertian mat(vec3(0, 0, 0));
	hitable *light;

	float size = 1.0;
	//float reflection = 0.90;
	//light = new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.5, 0.5, new diffuse_light(vec3(50, 50, 50)));
	//list.push_back(light);
	//mat.lights.push_back(light);

	light = new sphere(vec3(0.0, 10000.2, 0.0), 1000.05, new diffuse_light(vec3(50, 50, 50)));
	list.push_back(light);
	mat.lights.push_back(light);

	//list.push_back(new translate(new plymodel("blender_monkey.ply", new dielectric(1.4)), vec3(0.0, -0.2, 0.5)));
	list.push_back(new translate(new plymodel("blender_monkey.ply", new lambertian(vec3(1.0, 1.0, 1.0))), vec3(-0.7, -0.2, 0.0)));
	list.push_back(new translate(new plymodel("blender_monkey.ply", new oren_nayar(vec3(1.0, 1.0, 1.0), 1.0)), vec3(0.7, -0.2, 0.0)));
	hitable *obj = new sphere(vec3(0.0, -0.2, 0.5), 0.5, new dielectric(2.4));
	mat.lights.push_back(obj);

	return new hitable_list(list);
}

hitable *moon_room(void)
{
	std::vector<hitable *> list;

	quadrilateral *quad = new quadrilateral();
	float size = 1.0;
	quad->v[0] = vec3(-size, -1, size);
	quad->v[1] = vec3(size, -1, size);
	quad->v[2] = vec3(size, -1, -size);
	quad->v[3] = vec3(-size, -1, -size);
	quad->normal = vec3(0.0, 1.0, 0.0);
	quad->mat_ptr = new lambertian(vec3(0.7, 0.7, 0.7));
	//list.push_back(quad);

	lambertian mat(vec3(0, 0, 0));
	hitable *light;


	//light = new sphere(vec3(0.0, 0.0, 10000.2), 1000.05, new diffuse_light(vec3(50, 50, 50)));
	light = new sphere(vec3(0.0, 0000.0, 100.0), 50.05, new diffuse_light(vec3(3, 3, 3)));
	list.push_back(light);
	mat.lights.push_back(light);

	//list.push_back(new translate(new plymodel("blender_monkey.ply", new dielectric(1.4)), vec3(0.0, -0.2, 0.5)));
	//hitable *obj = new sphere(vec3(0.0, -0.2, 0.5), 0.5, new dielectric(2.4));
	//mat.lights.push_back(obj);
	list.push_back(new sphere(vec3(-0.7, -0.0, 0.5), 0.3, new lambertian(vec3(1, 1, 1))));
	list.push_back(new sphere(vec3(0.0, -0.0, 0.5), 0.3, new oren_nayar(vec3(1, 1, 1), 0.3)));
	list.push_back(new sphere(vec3(0.7, -0.0, 0.5), 0.3, new oren_nayar(vec3(1, 1, 1), 10000.0)));

	return new hitable_list(list);
}


int main(int argc, char **argv)
{

	if (argc < 2) {
		std::cout << "wrong number of arguments" << std::endl;
		exit(-1);
	}
	int nx = 800;
	int ny = 800;

	int ns = 300;

	vec3 **array = new vec3*[nx];
	for (int i = 0; i < nx; i++) {
		array[i] = new vec3[ny];
	}


	std::ofstream ofs;
	ofs.open(argv[1]);

	ofs << "P3\n" << nx << " " << ny << "\n255\n";
	//ofs << "P3\n" << nx << " " << ny << "\n65535\n";

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
	hitable *world = moon_room();
	camera cam(vec3(0.0, 0.0, 2.5), vec3(0.0, 0.0, 0.0), vec3(0, 1, 0), 60.0, 1.0);


	size_t count = 0;

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

			count++;
			array[i][j] = col;
			if (count % 5000 == 0) {
#ifdef _OPENMP
				std::cout << "thread: " << omp_get_thread_num() << "  ";
#endif
				std::cout << 100.0 * (float)count / (float)(nx*ny) << "%" << std::endl;
			}

		}
	}

	for (int j = ny-1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {
			array[i][j] /= float(ns);
			//array[i][j] = vec3(sqrt(array[i][j][0]), sqrt(array[i][j][1]), sqrt(array[i][j][2]));
			array[i][j] = vec3(pow(array[i][j][0], 1.0/2.2), pow(array[i][j][1], 1.0/2.2), pow(array[i][j][2], 1.0/2.2));

			int ir = std::min(std::max(int(255.99*array[i][j][0]), 0), 255);
			int ig = std::min(std::max(int(255.99*array[i][j][1]), 0), 255);
			int ib = std::min(std::max(int(255.99*array[i][j][2]), 0), 255);
			//int ir = std::min(std::max(int(65535.99*array[i][j][0]), 0), 65535);
			//int ig = std::min(std::max(int(65535.99*array[i][j][1]), 0), 65535);
			//int ib = std::min(std::max(int(65535.99*array[i][j][2]), 0), 65535);

			ofs << ir << " " << ig << " " << ib << "\n";
		}
	}
	return 0;
}
