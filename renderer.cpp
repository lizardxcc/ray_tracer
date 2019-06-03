#include <stack>
#include <random>
#ifdef _OPENMP
#include <omp.h>
#endif
#include "renderer.h"
#include "pdf.h"


//Renderer::Renderer(const char *filename)
//{
//	Load(filename);
//}

void Renderer::Load(const char *objfilename, const char *matfilename)
{
	obj_loader.Load(objfilename);
	//for (const auto& s : obj_loader.mtl_file) {
	//	mtl_loader.Load(s.c_str());
	//}
	Material_loader.Load(matfilename);
	//for (int i = 0; i < obj_loader.objects.size(); i++) {
	//	std::cout << "name: " << obj_loader.objects[i]->Material_name << std::endl;
	//	//std::shared_ptr<Material> mat = Material_loader.Materials.at(obj_loader.objects[i]->Material_name);
	//	//Materials.push_back(mat);
	//}
	world = std::make_unique<objmodel>(obj_loader);
}


void Renderer::Clear(void)
{
	obj_loader.Clear();
	Material_loader.Clear();
	img_updated = false;
}


void Renderer::RenderImage(int nx, int ny, int ns)
{
	orig_img.reset(new double[nx*ny*4]);
	Material::lights.clear();
	for (size_t i = 0; i < world->models.size(); i++) {
		//world->models[i]->set_Material(std::shared_ptr<Material>(Materials[i]));
		//auto mat = Material_loader.Materials[obj_loader.objects[i]->Material_name];
		auto mat = Material_loader.Materials[Material_loader.obj_mat_names[i]];
		world->models[i]->set_Material(mat);
		if (mat->light_flag) {
			Material::lights.push_back(world->models[i]);
		}
	}


	//std::ofstream ofs;
	//ofs.open(filename);

	//ofs << "P3\n" << nx << " " << ny << "\n255\n";

	size_t count = 0;

int i, j, s;
#ifdef _OPENMP
#pragma omp parallel for private(j, s) schedule(dynamic)
#endif
	for (i = 0; i < nx; i++) {
		for (j = 0; j < ny; j++) {
			Spectrum radiance(0.0);
			for (s = 0; s < ns; s++) {
				double u = (i + drand48()) / nx;
				double v = (j + drand48()) / ny;
				ray r = cam.get_ray(u, v);

				double rand = drand48();
				const size_t num = N_SAMPLE; // 調整
				for (size_t k = 0; k < num; k++) {
					if (rand <= (static_cast<double>(k+1)/ num)) {
						double min_wl = 400.0 + 300.0/num*k;
						double max_wl = min_wl + 300.0/num - 0.00001;
						//double max_wl = min_wl + SAMPLE_SIZE - 0.00001;
						//double max_wl = 400 + 300.0/(double)num*(double)(k+1) - 0.00001;
						r.min_wl = min_wl;
						r.max_wl = max_wl;
						r.central_wl = (min_wl + max_wl) / 2.0;
						double rad = NEEVolPathTracing(r, true);
						if (!std::isnan(rad)) {
							radiance.add(rad/ns, min_wl, max_wl);
						}
						break;
					}
				}
			}
			vec3 rgb_col = rgb(radiance);
			for (size_t i = 0; i < 3; i++) {
				if (rgb_col[i] >= 0.0) {
					rgb_col.e[i] = pow(rgb_col[i], 1.0/2.2);
				}
			}
			size_t i_ = nx-i-1;
			size_t j_ = ny-j-1;
			orig_img[((ny-j_-1)*nx+i_)*4] = std::min(rgb_col[0], 1.0);
			orig_img[((ny-j_-1)*nx+i_)*4+1] = std::min(rgb_col[1], 1.0);
			orig_img[((ny-j_-1)*nx+i_)*4+2] = std::min(rgb_col[2], 1.0);
			orig_img[((ny-j_-1)*nx+i_)*4+3] = 1.0;

			count++;
			if (count % 5000 == 0) {
#ifdef _OPENMP
				std::cout << "thread: " << omp_get_thread_num() << "  ";
#endif
				std::cout << 100.0 * static_cast<double>(count) / (nx*ny) << "%" << std::endl;
				img_updated = true;
			}

		}
	}

	//BiliteralFilter filter(orig_img, nx, ny);
	//filter.FilterImage();
	//for (int i = 0; i < nx*ny*4; i++) {
	//	img[i] = static_cast<GLubyte>(std::max(0.0, std::min(255*orig_img[i], 255.0)));
	//}
	////img = filter.result;
	//std::cout << "filtered" << std::endl;
	////for (int j = ny-1; j >= 0; j--) {
	////	for (int i = 0; i < nx; i++) {
	//for (int j = 0; j < ny; j++) {
	//	for (int i = nx-1; i >= 0; i--) {

	//		size_t i_ = nx-i-1;
	//		size_t j_ = ny-j-1;
	//		int ir = img[((ny-j_-1)*nx+i_)*4];
	//		int ig = img[((ny-j_-1)*nx+i_)*4+1];
	//		int ib = img[((ny-j_-1)*nx+i_)*4+2];

	//		ofs << ir << " " << ig << " " << ib << "\n";
	//	}
	//}

	img_updated = true;
}


double Renderer::NaivePathTracing(const ray& r)
{
	HitRecord rec;
	ray _ray = r;
	double radiance = 0.0;
	double beta = 1.0;
	while (1) {
		bool hit = world->hit(_ray, 0.001, std::numeric_limits<double>::max(), rec);

		if (hit) {
			radiance += beta * rec.mat_ptr->Emitted(_ray, rec);
		} else
			break;

		double bxdf, pdf;
		onb uvw;
		uvw.build_from_w(rec.normal);
		vec3 generated_vi;
		double wli;
		bool respawn = rec.mat_ptr->Sample(rec, uvw, uvw.worldtolocal(-_ray.direction()), r.central_wl, generated_vi, wli, bxdf, pdf);
		if (respawn)
			beta *= (bxdf * abs(generated_vi.z()) / pdf);
		double prr = 0.5;
		double d = drand48();
		if (d < prr)
			break;
		beta /= (1-prr);

		ray scattered = ray(rec.p, uvw.localtoworld(generated_vi));
		_ray.A = scattered.A;
		_ray.B = scattered.B;
	}

	return radiance;

}



double Renderer::NEEPathTracing(const ray& r, bool enableNEE)
{
	HitRecord rec;
	ray _ray = r;
	double radiance = 0.0;
	double beta = 1.0;
	int bounce = 0;
	bool IsLastBounceSpecular = false;
	while (1) {
		bool hit = world->hit(_ray, 0.001, std::numeric_limits<double>::max(), rec);

		if (!hit)
			break;
			

		if (bounce == 0 || IsLastBounceSpecular)
			radiance += beta * rec.mat_ptr->Emitted(_ray, rec);

		// calculate direct lighting
		if (rec.mat_ptr->specular_flag == false) {
			if (!enableNEE) {
				vec3 generated_vi;
				double wli;
				onb uvw_;
				uvw_.build_from_w(rec.normal);
				UniformPdf pdf(rec.normal);
				vec3 generated_direction = pdf.Generate();
				//bool respawn = rec.mat_ptr->Sample(rec, uvw_, uvw_.worldtolocal(-_ray.direction()), r.central_wl, generated_vi, wli, bxdf, pdfval);
				generated_vi = uvw_.worldtolocal(generated_direction);
				bool respawn = true;
				if (respawn) {
					ray scattered = ray(rec.p, uvw_.localtoworld(generated_vi));
					scattered.central_wl = _ray.central_wl;
					scattered.min_wl = _ray.min_wl;
					scattered.max_wl = _ray.max_wl;
					wli = _ray.central_wl;

					HitRecord tmp_rec;
					bool hit = world->hit(scattered, 0.001, std::numeric_limits<double>::max(), tmp_rec);
					if (hit) {
						double bxdf, pdfval;
						pdfval = pdf.PdfVal(generated_direction);
						bxdf = rec.mat_ptr->BxDF(generated_vi, wli, uvw_.worldtolocal(-_ray.direction()), r.central_wl);
						radiance += bxdf * (beta * tmp_rec.mat_ptr->Emitted(scattered, tmp_rec) * abs(generated_vi.z()) / pdfval);
					}
				}
			} else {
				if (rec.mat_ptr->light_flag == false) {
					std::random_device rnd;
					int selectedLight = rnd() % Material::lights.size();
					onb uvw_;
					uvw_.build_from_w(rec.normal);
					HittablePdf pdf(Material::lights[selectedLight], rec.p);
					vec3 generated_direction = pdf.Generate();

					HitRecord light_rec;
					ray scattered = ray(rec.p, generated_direction);
					scattered.central_wl = _ray.central_wl;
					scattered.min_wl = _ray.min_wl;
					scattered.max_wl = _ray.max_wl;
					bool hit = world->hit(scattered, 0.001, std::numeric_limits<double>::max(), light_rec);
					if (hit) {
						double pdfval = pdf.PdfVal(generated_direction);
						vec3 vi = uvw_.worldtolocal(generated_direction);
						vec3 vo = uvw_.worldtolocal(-_ray.direction());
						double wlo = _ray.central_wl;
						double wli = wlo;
						double BxDF = rec.mat_ptr->BxDF(vi, wli, vo, wlo);
						radiance += Material::lights.size() * BxDF * (beta * light_rec.mat_ptr->Emitted(scattered, light_rec) * abs(vi.z())) / pdfval;
					}



				}
			}
		}


		if (rec.mat_ptr->light_flag)
			break;
		vec3 generated_vi;
		double wli;
		bool respawn;
		double bxdf, pdf;
		onb uvw;
		uvw.build_from_w(rec.normal);
		{
			respawn = rec.mat_ptr->Sample(rec, uvw, uvw.worldtolocal(-_ray.direction()), r.central_wl, generated_vi, wli, bxdf, pdf);
			if (respawn) {
				beta *= (bxdf * abs(generated_vi.z()) / pdf);
				//if (rec.mat_ptr->specular_flag)
				//	std::cout << bxdf *abs(generated_vi.z())/pdf<< std::endl;
			}
		}

		//double prr = std::max(0.8, 1.0-(0.5 + bxdf/2.0));
		//double prr = std::max(0.5, 1.0 + bxdf/2.0);
		double prr = 1.0 - std::min(0.7, 0.2+bxdf/2.0);
		if (bounce > 6)
			prr = 0.9;
		double d = drand48();
		if (d < prr)
			break;
		beta /= (1.0-prr);

		if (!respawn)
			break;

		ray scattered = ray(rec.p, uvw.localtoworld(generated_vi));
		_ray.A = scattered.A;
		_ray.B = scattered.B;

		bounce++;

		IsLastBounceSpecular = rec.mat_ptr->specular_flag;
	}

	return radiance;

}



double Renderer::NEEVolPathTracing(const ray& r, bool enableNEE)
{
	HitRecord rec;
	ray _ray = r;
	double radiance = 0.0;
	double beta = 1.0;
	int surface_bounce = 0;
	int volume_bounce = 0;
	bool IsLastBounceSpecular = false;

	std::stack<unsigned int> inside_object_stack;
	while (1) {
		bool hit = world->hit(_ray, 0.001, std::numeric_limits<double>::max(), rec);

		if (!hit)
			break;

		double SampleMedium = false;
		double medium_t = 0.0;
		MediumMaterial *mi = nullptr;
		if (!inside_object_stack.empty()) {
			std::shared_ptr<Material> mat = Material_loader.Materials[Material_loader.obj_mat_names[inside_object_stack.top()]];
			mi = mat->mi;
			if (mi != nullptr) {
				const double sigma_t = mi->sigma_t.get(_ray.central_wl);
				const double sigma_s = mi->albedo.get(_ray.central_wl);
				double t = - log(1.0-drand48())/sigma_t / _ray.direction().length();
				SampleMedium = t < rec.t;
				if (!SampleMedium)
					t = rec.t;
				double tr = exp(-sigma_t * t * _ray.direction().length());
				if (SampleMedium) {
					double pdf = sigma_t * tr;
					beta *= (sigma_s * tr)/pdf;
					medium_t = t;
				} else {
					//double pdf = tr;
					//beta *= tr / pdf;
					// don't need to calculate the term (tr / pdf)
					// because it's guaranteed that tr / pdf is 1.0
				}
			}
		} else {
			// if ray is going through world space
		}



		bool respawn = false;
		double scattering_coefficient = 0.0;
		vec3 scattered_point; // in world cooredinate
		vec3 scattered_direction; // in world cooredinate

		if (SampleMedium) {
			std::random_device rnd;
			int selectedLight = rnd() % Material::lights.size();
			HittablePdf pdf(Material::lights[selectedLight], _ray.point_at_parameter(medium_t));
			vec3 generated_direction = pdf.Generate();



			HitRecord tmp_rec;
			ray scattered = ray(_ray.point_at_parameter(medium_t), generated_direction);
			scattered.central_wl = _ray.central_wl;
			scattered.min_wl = _ray.min_wl;
			scattered.max_wl = _ray.max_wl;

			bool hit_anything = false;
			unsigned int medium_object_id = inside_object_stack.top();
			vec3 last_smoke_point;
			while (true) {
				bool hit = world->hit(scattered, 0.001, std::numeric_limits<double>::max(), tmp_rec);
				hit_anything = hit;
				if (!hit)
					break;
				if (tmp_rec.hit_object_id == medium_object_id) {
					last_smoke_point = scattered.point_at_parameter(tmp_rec.t);
					scattered = ray(scattered.point_at_parameter(tmp_rec.t), generated_direction);
					scattered.central_wl = _ray.central_wl;
					scattered.min_wl = _ray.min_wl;
					scattered.max_wl = _ray.max_wl;
				} else {
					break;
				}
			}
			if (hit_anything) {
				double distance = (last_smoke_point - _ray.point_at_parameter(medium_t)).length();
				const double sigma_t = mi->sigma_t.get(_ray.central_wl);
				double tr = exp(-sigma_t * distance);
				double pdfval = pdf.PdfVal(generated_direction);
				double wlo = _ray.central_wl;
				double wli = wlo;
				double p = mi->Phase(generated_direction, wli, -_ray.direction(), wlo);
				radiance += Material::lights.size() * p * tr * beta * tmp_rec.mat_ptr->Emitted(scattered, tmp_rec) / pdfval;
			}

			vec3 vi;
			double wli;
			double phase, pdfval;
			respawn = mi->Sample_p(-_ray.direction(), _ray.central_wl, vi, wli, phase, pdfval);
			if (respawn) {
				scattered_point = _ray.point_at_parameter(medium_t);
				scattered_direction = vi;
				//beta *= phase / pdfval;
				// don't need to calculate the term (phase / pdfval) because it's
				// guaranteed that scattered direction is Sampled with a pdf which
				// completely matches the phase function of the medium
				scattering_coefficient = phase;
			}



			// Sample new direction at medium
			IsLastBounceSpecular = false;

			volume_bounce++;

		} else { // surface interation
			if (surface_bounce == 0 || IsLastBounceSpecular)
				radiance += beta * rec.mat_ptr->Emitted(_ray, rec);

			// calculate direct lighting
			if (rec.mat_ptr->specular_flag == false) {
				if (!enableNEE) {
					vec3 generated_vi;
					double wli;
					onb uvw_;
					uvw_.build_from_w(rec.normal);
					UniformPdf pdf(rec.normal);
					vec3 generated_direction = pdf.Generate();
					//bool respawn = rec.mat_ptr->Sample(rec, uvw_, uvw_.worldtolocal(-_ray.direction()), r.central_wl, generated_vi, wli, bxdf, pdfval);
					generated_vi = uvw_.worldtolocal(generated_direction);
					bool respawn = true;
					if (respawn) {
						ray scattered = ray(rec.p, uvw_.localtoworld(generated_vi));
						scattered.central_wl = _ray.central_wl;
						scattered.min_wl = _ray.min_wl;
						scattered.max_wl = _ray.max_wl;
						wli = _ray.central_wl;
						HitRecord tmp_rec;

						bool hit = world->hit(scattered, 0.001, std::numeric_limits<double>::max(), tmp_rec);
						if (hit) {
							double bxdf, pdfval;
							pdfval = pdf.PdfVal(generated_direction);
							bxdf = rec.mat_ptr->BxDF(generated_vi, wli, uvw_.worldtolocal(-_ray.direction()), r.central_wl);
							radiance += bxdf * (beta * tmp_rec.mat_ptr->Emitted(scattered, tmp_rec) * abs(generated_vi.z()) / pdfval);
						}
					}
				} else {
					if (rec.mat_ptr->light_flag == false) {
						std::random_device rnd;
						int selectedLight = rnd() % Material::lights.size();
						onb uvw_;
						uvw_.build_from_w(rec.normal);
						HittablePdf pdf(Material::lights[selectedLight], rec.p);
						vec3 generated_direction = pdf.Generate();

						HitRecord light_rec;
						ray scattered = ray(rec.p, generated_direction);
						scattered.central_wl = _ray.central_wl;
						scattered.min_wl = _ray.min_wl;
						scattered.max_wl = _ray.max_wl;
						bool hit = world->hit(scattered, 0.001, std::numeric_limits<double>::max(), light_rec);
						if (hit) {
							double pdfval = pdf.PdfVal(generated_direction);
							vec3 vi = uvw_.worldtolocal(generated_direction);
							vec3 vo = uvw_.worldtolocal(-_ray.direction());
							double wlo = _ray.central_wl;
							double wli = wlo;
							double BxDF = rec.mat_ptr->BxDF(vi, wli, vo, wlo);
							radiance += Material::lights.size() * BxDF * (beta * light_rec.mat_ptr->Emitted(scattered, light_rec) * abs(vi.z())) / pdfval;
						}



					}
				}
			}



			if (rec.mat_ptr->light_flag)
				break;
			vec3 generated_vi;
			double wli;
			double bxdf, pdf;
			onb uvw;
			uvw.build_from_w(rec.normal);
			{
				respawn = rec.mat_ptr->Sample(rec, uvw, uvw.worldtolocal(-_ray.direction()), r.central_wl, generated_vi, wli, bxdf, pdf);
				if (respawn) {
					beta *= (bxdf * abs(generated_vi.z()) / pdf);
					scattered_point = rec.p;
					scattered_direction = uvw.localtoworld(generated_vi);
					scattering_coefficient = bxdf;

					// when light penetrates through a surface
					if ((uvw.worldtolocal(-_ray.direction()).z() * generated_vi.z()) < 0.0) {
						if (generated_vi.z() > 0.0) { // going out
							if (!inside_object_stack.empty()) {
								if (inside_object_stack.top() != rec.hit_object_id)
									std::cout << "ERROR" << std::endl;
								else
									inside_object_stack.pop();
							} else {
								//std::cout << "WARNING" << std::endl;
								// maybe one of the object in the scene is not manifold
							}
						} else { // going in
							inside_object_stack.push(rec.hit_object_id);
						}
					}
					//if (rec.mat_ptr->specular_flag)
					//	std::cout << bxdf *abs(generated_vi.z())/pdf<< std::endl;
				}
			}
			IsLastBounceSpecular = rec.mat_ptr->specular_flag;

			surface_bounce++;
		}

		//double prr = std::max(0.8, 1.0-(0.5 + bxdf/2.0));
		//double prr = std::max(0.5, 1.0 + bxdf/2.0);
		double prr = 1.0 - std::min(0.7, 0.2+scattering_coefficient/2.0);
		//double prr = 0.5;
		if (surface_bounce > 10)
			prr = 0.9;
		if (volume_bounce > 10)
			prr = 0.9;
		double d = drand48();
		if (d < prr)
			break;
		beta /= (1.0-prr);

		if (!respawn)
			break;

		ray scattered = ray(scattered_point, scattered_direction);
		_ray.A = scattered.A;
		_ray.B = scattered.B;

	}

	return radiance;

}


double Renderer::GetRadiance(ray& r, int count)
{
	HitRecord rec;
	double radiance = 0.0;
	if (world->hit(r, 0.001, std::numeric_limits<double>::max(), rec)) {
		radiance += rec.mat_ptr->Emitted(r, rec);
		double bxdf, pdf;

		onb uvw;
		uvw.build_from_w(rec.normal);
		vec3 generated_vi;
		double wli;
		bool respawn = rec.mat_ptr->Sample(rec, uvw, uvw.worldtolocal(-r.direction()), r.central_wl, generated_vi, wli, bxdf, pdf);
		double prr;
		//if (respawn)
		//	beta *= (bxdf * abs(generated_vi.z()) / pdf);
		if (respawn) {
			if (count > 10) {
				prr = 0.01;
			} else {
				prr = std::min(0.8, 0.1+bxdf);
			}
		} else {
			prr = 0.0;
		}

		if (drand48() < prr) {
			if (respawn) {
				ray scattered(rec.p, uvw.localtoworld(generated_vi));
				scattered.central_wl = wli;
				scattered.min_wl = r.min_wl;
				scattered.max_wl = r.max_wl;
				radiance += bxdf * GetRadiance(scattered, count+1) *
					abs(generated_vi.z()) / pdf / prr;
			}
		}
		return radiance;
	} else {
		return 0.0;
	}
	return radiance;
}

