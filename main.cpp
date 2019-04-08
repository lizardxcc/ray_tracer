#include <iostream>
#include <algorithm>
#include <random>
#include "vec3.h"
#include "onb.h"
#include "ray.h"
#include "camera.h"
#include "hitablelist.h"
#include "material.h"
#include "object.h"
#include "spectrum.h"

#ifdef _OPENMP
#include <omp.h>
#endif

camera *cam;

void set_camera(double lx, double ly, double lz, double rax, double ray, double raz, double rot_theta, double vfov, double aspect)
{
	cam = new camera(vec3(lx, ly, lz), vec3(rax, ray, raz), rot_theta, vfov, aspect);
}

double get_radiance(ray& r, const hitable *world, int count)
{
	hit_record rec;
	if (world->hit(r, 0.001, std::numeric_limits<double>::max(), rec)) {
		double bxdf, pdf;
		double radiance = rec.mat_ptr->emitted(r, rec);
		onb uvw;
		uvw.build_from_w(rec.normal);
		vec3 generated_vi;
		double wli;
		bool respawn = rec.mat_ptr->sample(rec, uvw, uvw.worldtolocal(-r.direction()), r.central_wl, generated_vi, wli, bxdf, pdf);
		double prr;
		if (respawn) {
			prr = std::min(1.0, 0.1+bxdf);
		} else {
			prr = 0.0;
		}

		if (drand48() < prr) {
			if (respawn) {
				ray scattered(rec.p, uvw.localtoworld(generated_vi));
				scattered.central_wl = wli;
				scattered.min_wl = r.min_wl;
				scattered.max_wl = r.max_wl;
				radiance += bxdf * get_radiance(scattered, world, count+1) *
					abs(generated_vi.z()) / pdf / prr;
			}
		}
		return radiance;
	} else {
		return 0.0;
	}
}



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


hitable *obj_room(void)
{
	std::vector<hitable *> list;
	list.push_back(new objmodel("test.obj"));
	//list.push_back(new sphere(vec3(-0.5, -0.0, -0.5), 0.3, new metal(RGBtoSpectrum(vec3(0.8, 0.5, 0.1)))));
	//double size = 5.0;
	//hitable *light;
	//lambertian mat(Spectrum(0));
	//Spectrum light_s(0.01);
	//light = new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.5, 0.5, new diffuse_light(light_s));
	////list.push_back(light);
	////mat.lights.push_back(light);


	return new hitable_list(list);
}

void execute(int nx, int ny, int ns, const char *filename)
{
	Spectrum **spectrum_array = new Spectrum*[nx];
	vec3 **rgb_array = new vec3*[nx];
	for (int i = 0; i < nx; i++) {
		spectrum_array[i] = new Spectrum[ny];
		rgb_array[i] = new vec3[ny];
	}


	std::ofstream ofs;
	ofs.open(filename);

	ofs << "P3\n" << nx << " " << ny << "\n255\n";
	//ofs << "P3\n" << nx << " " << ny << "\n65535\n";

	hitable *world = obj_room();
	//camera cam(vec3(-1.0, 2.0, 6.4), vec3(0.0, 4.2, 0.0), vec3(0, 1, 0), 90.0, 1.0);
	//camera cam(vec3(0.0, 3.0, 3.0), vec3(0.0, 1.0, 0.0), vec3(0, 1, 0), 60.0, 1.0);
	//pinhole_camera cam(vec3(0.0, 3.0, 12.0), vec3(0.0, 0.0, -10.0), vec3(0, 1, 0), 1.0, 1.0);
	//lens_camera cam(vec3(0.0, 3.0, 12.0), vec3(0.0, 0.0, -10.0), vec3(0, 1, 0), 1.0, 0.85, 0.8, 1.0);
	//camera cam(vec3(-2.0, 3.0, -3.0), vec3(0.0, 0.0, 0.0), vec3(0, 1, 0), 60.0, 1.0);
	//camera cam(vec3(0.0, 10.0, 10.0), vec3(0.0, 0.0, 0.0), vec3(0, 1, 0), 60.0, (double)nx/(double)ny);

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
				ray r = cam->get_ray(u, v);

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
						double rad = get_radiance(r, world, 0);
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
}

int main(void)
{
	set_camera(0, 0, 2, 1, 0, 0, 90, 60, 1.0);
	execute(500, 500, 10, "test.pnm");
}
