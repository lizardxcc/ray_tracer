#include <iostream>
#include <algorithm>
#include <random>
#include "vec3.h"
#include "ray.h"
#include "camera.h"
#include "hitablelist.h"
#include "material.h"
#include "object.h"
#include "spectrum.h"

#ifdef _OPENMP
#include <omp.h>
#endif



double sample(ray& r, const hitable *world, int count)
{
	hit_record rec;
	//if (count > 20) {
	//	return 0.0;
	//}
	if (world->hit(r, 0.001, std::numeric_limits<double>::max(), rec)) {
		ray scattered;
		double bxdf, pdf;
		double value = rec.mat_ptr->emitted(0, 0, r, rec);
		double prr = 0.8;
		if (drand48() < prr) {
			if (rec.mat_ptr->sample(r, rec, scattered, bxdf, pdf)) {
				value += bxdf * sample(scattered, world, count+1) *
					abs(dot(rec.normal, unit_vector(scattered.direction()))) / pdf / prr;
			} else {
				value += 0.0;
			}
		}
		return value;
		//r.radiance *= (bxdf * abs(dot(rec.normal, unit_vector(scattered.direction()))) / pdf);
	} else {
		return 0.0;
	}
}


/*
vec3 color(const ray& r, hitable *world, int count)
{
	hit_record rec;
	if (world->hit(r, 0.001, std::numeric_limits<double>::max(), rec)) {
		ray scatterd;
		//vec3 attenuation;
		//vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
		double bxdf;
		double pdf;
		rec.mat_ptr->emitted(0, 0, r, rec);
		rec.mat_ptr->sample(r, rec, scattered, bxdf, pdf);

		//if (count < 20 && rec.mat_ptr->sample(r, rec, scatterd, bxdf, pdf)) {
		//	//return emitted + attenuation*color(scatterd, world, count+1);
		//	return emitted + bxdf*color(scatterd, world, count+1) * abs(dot(rec.normal, unit_vector(scatterd.direction())))/ pdf;
		//} else {
		//	return emitted;
		//}
	} else {
		return vec3(0.0, 0.0, 0.0);
	}
}
*/



hitable *room(void)
{
	std::vector<hitable *> list;
	Spectrum albedo(1.0);
	//albedo.data[5] = 1.0;
	quadrilateral *quad = new quadrilateral();
	quad->v[0] = vec3(-10.0, -1, 10.0);
	quad->v[1] = vec3(10.0, -1, 10.0);
	quad->v[2] = vec3(10.0, -1, -10.0);
	quad->v[3] = vec3(-10.0, -1, -10.0);
	quad->normal = vec3(0.0, 1.0, 0.0);
	quad->mat_ptr = new lambertian(albedo);
	list.push_back(quad);

	//list.push_back(new sphere(vec3(-0.5, -0.0, -0.5), 0.3, new dielectric(Spectrum(1), 1.72, 0.41342)));

	double size = 1.0;
	hitable *light;
	lambertian mat(Spectrum(0));
	Spectrum light_s;
	light_s.data[0] = 0.030;
	light_s.data[1] = 0.030;
	light_s.data[2] = 0.030;
	light_s.data[3] = 0.030;
	light_s.data[4] = 0.030;
	light_s.data[5] = 0.030;
	light_s.data[6] = 0.030;
	light_s.data[7] = 0.030;
	light_s.data[8] = 0.030;
	light_s.data[9] = 0.030;
	light = new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.5, 0.5, new diffuse_light(light_s));
	list.push_back(light);
	mat.lights.push_back(light);

	return new hitable_list(list);
}


/*
hitable *testroom(void)
{
	double size = 1.0;
	double reflection = 0.90;
	std::vector<hitable *> list;


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

	double size = 1.0;
	//double reflection = 0.90;
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
	double size = 1.0;
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

*/
hitable *obj_room(void)
{
	std::vector<hitable *> list;
	list.push_back(new objmodel("test.obj"));
	//double size = 5.0;
	//hitable *light;
	//lambertian mat(Spectrum(0));
	//Spectrum light_s(0.01);
	//light = new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.5, 0.5, new diffuse_light(light_s));
	////list.push_back(light);
	////mat.lights.push_back(light);


	return new hitable_list(list);
}

int main(int argc, char **argv)
{

	if (argc < 5) {
		std::cout << "wrong number of arguments" << std::endl;
		exit(-1);
	}
	int nx = atoi(argv[2]);
	int ny = atoi(argv[3]);

	int ns = atoi(argv[4]);

	Spectrum **spectrum_array = new Spectrum*[nx];
	vec3 **rgb_array = new vec3*[nx];
	for (int i = 0; i < nx; i++) {
		spectrum_array[i] = new Spectrum[ny];
		rgb_array[i] = new vec3[ny];
	}


	std::ofstream ofs;
	ofs.open(argv[1]);

	ofs << "P3\n" << nx << " " << ny << "\n255\n";
	//ofs << "P3\n" << nx << " " << ny << "\n65535\n";

	hitable *world = obj_room();
	//camera cam(vec3(-1.0, 2.0, 6.4), vec3(0.0, 4.2, 0.0), vec3(0, 1, 0), 90.0, 1.0);
	//camera cam(vec3(0.0, 3.0, 3.0), vec3(0.0, 1.0, 0.0), vec3(0, 1, 0), 60.0, 1.0);
	//pinhole_camera cam(vec3(0.0, 3.0, 12.0), vec3(0.0, 0.0, -10.0), vec3(0, 1, 0), 1.0, 1.0);
	//lens_camera cam(vec3(0.0, 3.0, 12.0), vec3(0.0, 0.0, -10.0), vec3(0, 1, 0), 1.0, 0.85, 0.8, 1.0);
	//camera cam(vec3(-2.0, 3.0, -3.0), vec3(0.0, 0.0, 0.0), vec3(0, 1, 0), 60.0, 1.0);
	camera cam(vec3(0.0, 0.0, 1.5), vec3(0.0, 0.0, 0.0), vec3(0, 1, 0), 60.0, 1.0);

	size_t count = 0;

int i, j, s;
#ifdef _OPENMP
#pragma omp parallel for private(j, s) schedule(dynamic)
#endif
	for (i = 0; i < nx; i++) {
		for (j = 0; j < ny; j++) {
			//vec3 col(0, 0, 0);
			Spectrum radiance(0);
			for (s = 0; s < ns; s++) {
				double u = double(i + drand48()) / double(nx);
				double v = double(j + drand48()) / double(ny);
				ray r = cam.get_ray(u, v);

				double rand = drand48();
				const size_t num = 10;
				for (size_t k = 0; k < num; k++) {
					if (rand <= ((double)(k+1)/ (double)num)) {
						double min_wl = 400 + 300.0/(double)num*(double)k;
						double max_wl = min_wl + 300.0/(double)num - 0.00001;
						//double max_wl = min_wl + SAMPLE_SIZE - 0.00001;
						//double max_wl = 400 + 300.0/(double)num*(double)(k+1) - 0.00001;
						r.min_wl = min_wl;
						r.max_wl = max_wl;
						r.central_wl = (min_wl + max_wl) / 2.0;
						double rad = sample(r, world, 0);
						//if (rad > std::numeric_limits<double>::max()) {
						//	std::cout << "ALARM" << std::endl;
						//}
						if (!std::isnan(rad)) {
							radiance.add((double)(rad/(double)ns), min_wl, max_wl);
						}
						break;
					}
				}
			}

			count++;
			spectrum_array[i][j] = radiance;
			if (count % 5000 == 0) {
#ifdef _OPENMP
				std::cout << "thread: " << omp_get_thread_num() << "  ";
#endif
				std::cout << 100.0 * (double)count / (double)(nx*ny) << "%" << std::endl;
			}

		}
	}

	for (int j = ny-1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {
	//for (int j = 0; j < ny; j++) {
	//	for (int i = nx-1; i >= 0; i--) {
			//spectrum_array[i][j] /= double(ns);
			//spectrum_array[i][j] *= 1.0;
			//array[i][j] = vec3(sqrt(array[i][j][0]), sqrt(array[i][j][1]), sqrt(array[i][j][2]));
			vec3 rgb_col = rgb(spectrum_array[i][j]);
			for (size_t i = 0; i < 3; i++) {
				if (rgb_col[i] >= 0.0) {
					rgb_col.e[i] = pow(rgb_col[i], 1.0/2.2);
					//rgb_col.e[i] = pow(rgb_col[i], 4.0);
				}
			}

			int ir = std::min(std::max(int(255.99*rgb_col[0]), 0), 255);
			int ig = std::min(std::max(int(255.99*rgb_col[1]), 0), 255);
			int ib = std::min(std::max(int(255.99*rgb_col[2]), 0), 255);
			//int ir = std::min(std::max(int(65535.99*array[i][j][0]), 0), 65535);
			//int ig = std::min(std::max(int(65535.99*array[i][j][1]), 0), 65535);
			//int ib = std::min(std::max(int(65535.99*array[i][j][2]), 0), 65535);

			ofs << ir << " " << ig << " " << ib << "\n";
		}
	}
	return 0;
}
