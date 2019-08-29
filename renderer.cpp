#include <iostream>
#include <iomanip>
#include <stack>
#ifdef _OPENMP
#include <omp.h>
#endif
#include "renderer.h"
#include "pdf.h"
#include "scene.h"


Renderer::Renderer(void)
{
	std::random_device rd;
	mt.seed(rd());
}

void Renderer::Load(const char *objfilename)
{
	obj_loader.Load(objfilename);
	world = std::make_unique<ObjModel>(obj_loader);
}

void Renderer::LoadMaterials(const std::vector<std::shared_ptr<NodeMaterial>>& materials)
{
	light_objects.clear();
	for (size_t i = 0; i < world->models.size(); i++) {
		world->models[i]->SetMaterial(materials[i].get());
		if (materials[i]->light_flag) {
			for (const auto& pol : world->polygon_models[i]) {
				light_objects.push_back(pol);
			}
		}
	}
}


void Renderer::Clear(void)
{
	obj_loader.Clear();
	img_updated = false;
}


void Renderer::RenderImage(int nx, int ny, int ns, int spectral_samples, bool enable_openmp)
{
	orig_img.resize(nx*ny*4);
	spectrum_img.resize(nx*ny);
	std::fill(spectrum_img.begin(), spectrum_img.end(), 0.0);

	size_t count = 0;
	rendering_runnnig = true;

int s;

	for (s = 0; s < ns; s++) {
		if (stop_rendering)
			break;
		int i, j;
#ifdef _OPENMP
		//#pragma omp parallel for private(j, s) schedule(dynamic) if (enable_openmp)
#pragma omp parallel for private(j) schedule(dynamic) if (enable_openmp)
#endif
		for (i = 0; i < nx; i++) {
			for (j = 0; j < ny; j++) {

				double u = (i + drand48()) / nx;
				double v = (j + drand48()) / ny;
				ray r = cam.get_ray(u, v);

				int k = mt()%spectral_samples;
				double min_wl = 400.0 + 300.0/spectral_samples*k;
				double max_wl = min_wl + 300.0/spectral_samples - 0.00001;
				//double max_wl = min_wl + SAMPLE_SIZE - 0.00001;
				//double max_wl = 400 + 300.0/(double)num*(double)(k+1) - 0.00001;
				r.min_wl = min_wl;
				r.max_wl = max_wl;
				r.central_wl = (min_wl + max_wl) / 2.0;
				double rad;
				switch (algorithm_type) {
					case Naive:
						rad = NaivePathTracing(r);
						break;
					case NEE:
						rad = NEEPathTracingWithoutSpecular(r);
						break;
					case MIS:
						rad = NEEMISPathTracing(r);
				}
				if (!std::isnan(rad)) {
					spectrum_img[i*ny+j].add(rad/ns, min_wl, max_wl);
				}
			}
		}
#ifdef _OPENMP
#pragma omp parallel for private(j) if (enable_openmp)
#endif
		for (i = 0; i < nx; i++) {
			for (j = 0; j < ny; j++) {
				vec3 rgb_col = rgb(spectrum_img[i*ny+j]/(s+1)*ns);
				for (size_t c = 0; c < 3; c++) {
					if (rgb_col[c] >= 0.0) {
						rgb_col.e[c] = pow(rgb_col[c], 1.0/2.2);
					}
				}
				size_t i_ = nx-i-1;
				size_t j_ = ny-j-1;
				orig_img[((ny-j_-1)*nx+i_)*4] = std::min(rgb_col[0], 1.0);
				orig_img[((ny-j_-1)*nx+i_)*4+1] = std::min(rgb_col[1], 1.0);
				orig_img[((ny-j_-1)*nx+i_)*4+2] = std::min(rgb_col[2], 1.0);
				orig_img[((ny-j_-1)*nx+i_)*4+3] = 1.0;
				img_updated = true;
			}
		}
		std::cout << 100.0 * static_cast<double>(s) / (ns) << "%" << std::endl;
	}

	img_updated = true;
	std::cout << "Completed rendering" << std::endl;
	if (stop_rendering)
		stop_rendering = false;
	rendering_runnnig = false;
}


double Renderer::NaivePathTracing(const ray& r)
{
	HitRecord rec;
	ray _ray = r;
	double radiance = 0.0;
	double beta = 1.0;
	Sphere sphere;
	sphere.center = vec3(0.0, 0.0, 0.0);
	sphere.radius = 100.0;
	while (1) {
		bool hit = world->Hit(_ray, 0.001, std::numeric_limits<double>::max(), rec);

		if (!hit) {
			if (env_mapping_texture == nullptr)
				break;
			bool hit = sphere.Hit(_ray, 0.001, std::numeric_limits<double>::max(), rec);
			if (hit) {
				vec3 p = unit_vector(_ray.point_at_parameter(rec.t));
				double theta = acos(p.y());
				double phi = atan2(p.x()/sin(theta), p.z()/sin(theta));
				phi += M_PI;
				int x = env_mapping_width - (int)(phi/ (2.0*M_PI) * env_mapping_width);
				int y = (int)(theta/ M_PI * env_mapping_height);
				vec3 rgb;
				rgb[0] = static_cast<double>(env_mapping_texture[env_mapping_bpp*(x+y*env_mapping_width)])/255.0;
				rgb[1] = static_cast<double>(env_mapping_texture[env_mapping_bpp*(x+y*env_mapping_width)+1])/255.0;
				rgb[2] = static_cast<double>(env_mapping_texture[env_mapping_bpp*(x+y*env_mapping_width)+2])/255.0;
				Spectrum spectrum = RGBtoSpectrum(rgb * env_brightness);
				radiance += beta * spectrum.get(_ray.central_wl);
			}
			break;
		}

		rec.mat_ptr->PreProcess(rec);
		radiance += beta * rec.mat_ptr->Emitted(_ray, rec, rec.vt);

		double bxdf_divided_by_pdf;
		double bxdf, pdf;
		ONB uvw;
		uvw.BuildFromW(rec.normal);
		vec3 generated_vi;
		double wli;
		bool respawn = rec.mat_ptr->Sample(rec, uvw, uvw.WorldToLocal(-_ray.direction()), r.central_wl, generated_vi, wli, bxdf_divided_by_pdf, bxdf, pdf);
		if (respawn)
			beta *= (bxdf_divided_by_pdf * abs(generated_vi.z()));
		double prr = 0.5;
		double d = drand48();
		if (d < prr)
			break;
		beta /= (1-prr);

		ray scattered = ray(rec.p, uvw.LocalToWorld(generated_vi));
		_ray.A = scattered.A;
		_ray.B = scattered.B;
	}

	return radiance;

}


double Renderer::NEEPathTracingWithoutSpecular(const ray& r)
{
	ray _ray = r;
	HitRecord rec;
	int bounce = 0;
	double radiance = 0.0;
	double beta = 1.0;
	while (1) {
		bool hit = world->Hit(_ray, 0.0001, std::numeric_limits<double>::max(), rec);
		if (!hit) {
			break;
		}

		bool preprocessed = false;

		if (bounce == 0) {
			if (rec.mat_ptr->light_flag) {
				rec.mat_ptr->PreProcess(rec);
				preprocessed = true;
				const double light = rec.mat_ptr->Emitted(_ray, rec, rec.vt);
				radiance += light;
			}
		}
		// sample light
		{
			int selected_light = mt() % light_objects.size();
			vec3 p;
			double area;
			light_objects[selected_light]->GetRandomPointOnPolygon(p, area);
			ray shadow_ray = ray(rec.p, unit_vector(p - rec.p));
			shadow_ray.central_wl = r.central_wl;
			shadow_ray.min_wl = r.min_wl;
			shadow_ray.max_wl = r.max_wl;

			if (!world->Occluded(shadow_ray, 0.0001, (p-rec.p).length()-0.0001)) {
				if (!preprocessed) {
					rec.mat_ptr->PreProcess(rec);
					preprocessed = true;
				}
				ONB uvw_;
				uvw_.BuildFromW(rec.normal);
				const vec3 vi = uvw_.WorldToLocal(unit_vector(p - rec.p));
				const vec3 vo = uvw_.WorldToLocal(-_ray.direction());
				const double wlo = _ray.central_wl;
				const double wli = wlo;
				const double BxDF = rec.mat_ptr->BxDF(vi, wli, vo, wlo, rec.vt);
				const double G = abs(dot(shadow_ray.direction(), light_objects[selected_light]->face_normal) * dot(shadow_ray.direction(), rec.normal) / (p-rec.p).squared_length());
				const double pdfarea = 1.0 / area / light_objects.size();
				HitRecord light_rec;
				if (light_objects[selected_light]->Hit(shadow_ray, 0.0001, (p-rec.p).length()+0.0001, light_rec)) {
					light_rec.mat_ptr->PreProcess(light_rec);
					const double light = light_rec.mat_ptr->Emitted(shadow_ray, light_rec, light_rec.vt);
					const double add = BxDF * beta * light * G / pdfarea;
					radiance += add;
				} else {
					// bug?
				}
			}
		}


		vec3 generated_vi;
		double wli;
		double bxdf_divided_by_pdf;
		double bxdf, pdf;
		ONB uvw;
		uvw.BuildFromW(rec.normal);
		if (!preprocessed)
			rec.mat_ptr->PreProcess(rec);
		bool respawn = rec.mat_ptr->Sample(rec, uvw, uvw.WorldToLocal(-_ray.direction()), _ray.central_wl, generated_vi, wli, bxdf_divided_by_pdf, bxdf, pdf);
		if (respawn)
			beta *= bxdf_divided_by_pdf * abs(generated_vi.z());


		//double prr = 1.0 - std::min(0.7, 0.2+bxdf/2.0);
		double prr = 0.5;
		if (bounce > 6)
			prr = 0.9;
		double d = drand48();
		if (d < prr)
			break;
		beta /= (1.0-prr);

		if (!respawn)
			break;

		_ray.A = rec.p;
		_ray.B = unit_vector(uvw.LocalToWorld(generated_vi));
		_ray.central_wl = r.central_wl;
		_ray.min_wl = r.min_wl;
		_ray.max_wl = r.max_wl;

		bounce++;

	}
	return radiance;
}



double Renderer::NEEMISPathTracing(const ray& r)
{
	ray _ray = r;
	HitRecord rec;
	int bounce = 0;
	double radiance = 0.0;
	double beta = 1.0;
	while (1) {
		bool hit = world->Hit(_ray, 0.001, std::numeric_limits<double>::max(), rec);
		if (!hit) {
			break;
		}

		bool preprocessed = false;

		if (bounce == 0) {
			if (rec.mat_ptr->light_flag) {
				rec.mat_ptr->PreProcess(rec);
				preprocessed = true;
				const double light = rec.mat_ptr->Emitted(_ray, rec, rec.vt);
				radiance += light;
			}
		}
		int selected_light = mt() % light_objects.size();
		// sample light
		{
			vec3 p;
			double area;
			light_objects[selected_light]->GetRandomPointOnPolygon(p, area);
			ray shadow_ray = ray(rec.p, unit_vector(p - rec.p));
			shadow_ray.central_wl = r.central_wl;
			shadow_ray.min_wl = r.min_wl;
			shadow_ray.max_wl = r.max_wl;

			bool notoccluded = false;
			HitRecord light_rec;
			bool light_hit = world->Hit(shadow_ray, 0.001, (p-rec.p).length()+0.001, light_rec);
			if (light_hit) {
				if (light_rec.hit_object == light_objects[selected_light])
					notoccluded = true;
			}

			if (notoccluded) {
				if (!preprocessed) {
					rec.mat_ptr->PreProcess(rec);
					preprocessed = true;
				}
				ONB uvw_;
				uvw_.BuildFromW(rec.normal);
				const vec3 vi = uvw_.WorldToLocal(unit_vector(p - rec.p));
				const vec3 vo = uvw_.WorldToLocal(-_ray.direction());
				const double wlo = _ray.central_wl;
				const double wli = wlo;
				const double BxDF = rec.mat_ptr->BxDF(vi, wli, vo, wlo, rec.vt);
				const double G = abs(dot(shadow_ray.direction(), light_rec.normal) * dot(shadow_ray.direction(), rec.normal) / (light_rec.t*light_rec.t));
				const double lightpdfarea = 1.0 / area / light_objects.size();
				//const double lightpdf = lightpdfarea * abs(dot(shadow_ray.direction(), light_rec.normal)) / (light_rec.t*light_rec.t);
				const double lightpdf_solidangle = light_rec.t*light_rec.t / abs(dot(shadow_ray.direction(), light_rec.normal)) * lightpdfarea;
				light_rec.mat_ptr->PreProcess(light_rec);
				const double light = light_rec.mat_ptr->Emitted(shadow_ray, light_rec, light_rec.vt);

				const double scatteringpdf = rec.mat_ptr->PDF(vi, wli, vo, wlo, rec.vt);
				double mis_weight;
				if (isinf(scatteringpdf))
					mis_weight = 0.0;
				else
					mis_weight = lightpdf_solidangle*lightpdf_solidangle/(lightpdf_solidangle*lightpdf_solidangle + scatteringpdf * scatteringpdf);
				//const double add = BxDF * beta * light * abs(vi.z()) * G / pdfarea;
				const double add = BxDF * beta * light * abs(vi.z()) * mis_weight / lightpdf_solidangle;
				radiance += add;
			}
		}

		// sample bsdf
		{
			double bxdf_divided_by_pdf;
			double bxdf, scattering_pdf;
			ONB uvw;
			uvw.BuildFromW(rec.normal);
			vec3 generated_vi;
			double wli;
			bool respawn = rec.mat_ptr->Sample(rec, uvw, uvw.WorldToLocal(-_ray.direction()), r.central_wl, generated_vi, wli, bxdf_divided_by_pdf, bxdf, scattering_pdf);
			if (respawn) {
				ray shadow_ray = ray(rec.p, uvw.LocalToWorld(generated_vi));
				shadow_ray.central_wl = r.central_wl;
				shadow_ray.min_wl = r.min_wl;
				shadow_ray.max_wl = r.max_wl;
				HitRecord light_rec;
				bool light_hit = world->Hit(shadow_ray, 0.001, std::numeric_limits<double>::max(), light_rec);
				if (light_hit) {
					if (light_rec.hit_object == light_objects[selected_light]) {
						const double light = light_rec.mat_ptr->Emitted(shadow_ray, light_rec, light_rec.vt);
						const double lightpdfarea = 1.0 / light_objects[selected_light]->polygon_area / light_objects.size();
						const double light_pdf_solid_angle = light_rec.t*light_rec.t / abs(dot(shadow_ray.direction(), light_rec.normal)) * lightpdfarea;
						//const double scatteringpdf = rec.mat_ptr->PDF(vi, wli, vo, wlo);
						double mis_weight;
						if (isinf(light_pdf_solid_angle))
							mis_weight = 0.0;
						else if (isinf(scattering_pdf))
							mis_weight = 1.0;
						else
							mis_weight = scattering_pdf*scattering_pdf/(scattering_pdf*scattering_pdf + light_pdf_solid_angle*light_pdf_solid_angle);
						const double add = bxdf_divided_by_pdf * beta * light * abs(generated_vi.z()) * mis_weight;
						radiance += add;
					}
				}
			}
		}


		vec3 generated_vi;
		double wli;
		double bxdf_divided_by_pdf;
		double bxdf, pdf;
		ONB uvw;
		uvw.BuildFromW(rec.normal);
		if (!preprocessed)
			rec.mat_ptr->PreProcess(rec);
		bool respawn = rec.mat_ptr->Sample(rec, uvw, uvw.WorldToLocal(-_ray.direction()), _ray.central_wl, generated_vi, wli, bxdf_divided_by_pdf, bxdf, pdf);
		if (respawn)
			beta *= bxdf_divided_by_pdf * abs(generated_vi.z());
			//beta *= bxdf * abs(generated_vi.z()) / pdf;


		//double prr = 1.0 - std::min(0.7, 0.2+bxdf/2.0);
		double prr = 0.5;
		if (bounce > 6)
			prr = 0.9;
		double d = drand48();
		if (d < prr)
			break;
		beta /= (1.0-prr);

		if (!respawn)
			break;

		_ray.A = rec.p;
		_ray.B = unit_vector(uvw.LocalToWorld(generated_vi));
		_ray.central_wl = r.central_wl;
		_ray.min_wl = r.min_wl;
		_ray.max_wl = r.max_wl;

		bounce++;

	}
	return radiance;
}

//double Renderer::NEEPathTracing(const ray& r)
//{
//	HitRecord rec;
//	ray _ray = r;
//	double radiance = 0.0;
//	double beta = 1.0;
//	int bounce = 0;
//	bool IsLastBounceSpecular = false;
//	while (1) {
//		bool hit = world->Hit(_ray, 0.001, std::numeric_limits<double>::max(), rec);
//
//		if (!hit)
//			break;
//			
//
//		if (bounce == 0 || IsLastBounceSpecular)
//			radiance += beta * rec.mat_ptr->Emitted(_ray, rec);
//
//		// calculate direct lighting
//		if (rec.mat_ptr->specular_flag == false) {
//			/*
//			if (!enableNEE) {
//				vec3 generated_vi;
//				double wli;
//				ONB uvw_;
//				uvw_.BuildFromW(rec.normal);
//				UniformPdf pdf(rec.normal);
//				vec3 generated_direction = pdf.Generate();
//				//bool respawn = rec.mat_ptr->Sample(rec, uvw_, uvw_.WorldToLocal(-_ray.direction()), r.central_wl, generated_vi, wli, bxdf, pdfval);
//				generated_vi = uvw_.WorldToLocal(generated_direction);
//				bool respawn = true;
//				if (respawn) {
//					ray scattered = ray(rec.p, uvw_.LocalToWorld(generated_vi));
//					scattered.central_wl = _ray.central_wl;
//					scattered.min_wl = _ray.min_wl;
//					scattered.max_wl = _ray.max_wl;
//					wli = _ray.central_wl;
//
//					HitRecord tmp_rec;
//					bool hit = world->Hit(scattered, 0.001, std::numeric_limits<double>::max(), tmp_rec);
//					if (hit) {
//						double bxdf, pdfval;
//						pdfval = pdf.PdfVal(generated_direction);
//						bxdf = rec.mat_ptr->BxDF(generated_vi, wli, uvw_.WorldToLocal(-_ray.direction()), r.central_wl);
//						radiance += bxdf * (beta * tmp_rec.mat_ptr->Emitted(scattered, tmp_rec) * abs(generated_vi.z()) / pdfval);
//					}
//				}
//			} else {
//			*/
//			if (rec.mat_ptr->light_flag == false) {
//				std::random_device rnd;
//				int selectedLight = rnd() % Material::lights.size();
//				ONB uvw_;
//				uvw_.BuildFromW(rec.normal);
//				HittablePdf pdf(Material::lights[selectedLight], rec.p);
//				vec3 generated_direction = pdf.Generate();
//
//				HitRecord light_rec;
//				ray scattered = ray(rec.p, generated_direction);
//				scattered.central_wl = _ray.central_wl;
//				scattered.min_wl = _ray.min_wl;
//				scattered.max_wl = _ray.max_wl;
//				bool hit = world->Hit(scattered, 0.001, std::numeric_limits<double>::max(), light_rec);
//				if (hit) {
//					double pdfval = pdf.PdfVal(generated_direction);
//					vec3 vi = uvw_.WorldToLocal(generated_direction);
//					vec3 vo = uvw_.WorldToLocal(-_ray.direction());
//					double wlo = _ray.central_wl;
//					double wli = wlo;
//					double BxDF = rec.mat_ptr->BxDF(vi, wli, vo, wlo);
//					radiance += Material::lights.size() * BxDF * (beta * light_rec.mat_ptr->Emitted(scattered, light_rec) * abs(vi.z())) / pdfval;
//				}
//			}
//		}
//
//
//		if (rec.mat_ptr->light_flag)
//			break;
//		vec3 generated_vi;
//		double wli;
//		bool respawn;
//		double bxdf, pdf;
//		ONB uvw;
//		uvw.BuildFromW(rec.normal);
//		{
//			respawn = rec.mat_ptr->Sample(rec, uvw, uvw.WorldToLocal(-_ray.direction()), r.central_wl, generated_vi, wli, bxdf, pdf);
//			if (respawn) {
//				beta *= (bxdf * abs(generated_vi.z()) / pdf);
//				//if (rec.mat_ptr->specular_flag)
//				//	std::cout << bxdf *abs(generated_vi.z())/pdf<< std::endl;
//			}
//		}
//
//		//double prr = std::max(0.8, 1.0-(0.5 + bxdf/2.0));
//		//double prr = std::max(0.5, 1.0 + bxdf/2.0);
//		double prr = 1.0 - std::min(0.7, 0.2+bxdf/2.0);
//		if (bounce > 6)
//			prr = 0.9;
//		double d = drand48();
//		if (d < prr)
//			break;
//		beta /= (1.0-prr);
//
//		if (!respawn)
//			break;
//
//		ray scattered = ray(rec.p, uvw.LocalToWorld(generated_vi));
//		_ray.A = scattered.A;
//		_ray.B = scattered.B;
//
//		bounce++;
//
//		IsLastBounceSpecular = rec.mat_ptr->specular_flag;
//	}
//
//	return radiance;
//
//}
//
//
//
//double Renderer::NEEVolPathTracing(const ray& r, bool enableNEE)
//{
//	HitRecord rec;
//	ray _ray = r;
//	double radiance = 0.0;
//	double beta = 1.0;
//	int surface_bounce = 0;
//	int volume_bounce = 0;
//	bool IsLastBounceSpecular = false;
//
//	std::stack<unsigned int> inside_object_stack;
//	while (1) {
//		bool hit = world->Hit(_ray, 0.001, std::numeric_limits<double>::max(), rec);
//
//		if (!hit)
//			break;
//
//		double SampleMedium = false;
//		double medium_t = 0.0;
//		MediumMaterial *mi = nullptr;
//		if (!inside_object_stack.empty()) {
//			std::shared_ptr<Material> mat = Material_loader.Materials[Material_loader.obj_mat_names[inside_object_stack.top()]];
//			mi = mat->mi;
//			if (mi != nullptr) {
//				const double sigma_t = mi->sigma_t.get(_ray.central_wl);
//				const double sigma_s = mi->albedo.get(_ray.central_wl);
//				double t = - log(1.0-drand48())/sigma_t / _ray.direction().length();
//				SampleMedium = t < rec.t;
//				if (!SampleMedium)
//					t = rec.t;
//				double tr = exp(-sigma_t * t * _ray.direction().length());
//				if (SampleMedium) {
//					double pdf = sigma_t * tr;
//					beta *= (sigma_s * tr)/pdf;
//					medium_t = t;
//				} else {
//					//double pdf = tr;
//					//beta *= tr / pdf;
//					// don't need to calculate the term (tr / pdf)
//					// because it's guaranteed that tr / pdf is 1.0
//				}
//			}
//		} else {
//			// if ray is going through world space
//		}
//
//
//
//		bool respawn = false;
//		double scattering_coefficient = 0.0;
//		vec3 scattered_point; // in world cooredinate
//		vec3 scattered_direction; // in world cooredinate
//
//		if (SampleMedium) {
//			std::random_device rnd;
//			int selectedLight = rnd() % Material::lights.size();
//			HittablePdf pdf(Material::lights[selectedLight], _ray.point_at_parameter(medium_t));
//			vec3 generated_direction = pdf.Generate();
//
//
//
//			HitRecord tmp_rec;
//			ray scattered = ray(_ray.point_at_parameter(medium_t), generated_direction);
//			scattered.central_wl = _ray.central_wl;
//			scattered.min_wl = _ray.min_wl;
//			scattered.max_wl = _ray.max_wl;
//
//			bool hit_anything = false;
//			unsigned int medium_object_id = inside_object_stack.top();
//			vec3 last_smoke_point;
//			while (true) {
//				bool hit = world->Hit(scattered, 0.001, std::numeric_limits<double>::max(), tmp_rec);
//				hit_anything = hit;
//				if (!hit)
//					break;
//				if (tmp_rec.hit_object_id == medium_object_id) {
//					last_smoke_point = scattered.point_at_parameter(tmp_rec.t);
//					scattered = ray(scattered.point_at_parameter(tmp_rec.t), generated_direction);
//					scattered.central_wl = _ray.central_wl;
//					scattered.min_wl = _ray.min_wl;
//					scattered.max_wl = _ray.max_wl;
//				} else {
//					break;
//				}
//			}
//			if (hit_anything) {
//				double distance = (last_smoke_point - _ray.point_at_parameter(medium_t)).length();
//				const double sigma_t = mi->sigma_t.get(_ray.central_wl);
//				double tr = exp(-sigma_t * distance);
//				double pdfval = pdf.PdfVal(generated_direction);
//				double wlo = _ray.central_wl;
//				double wli = wlo;
//				double p = mi->Phase(generated_direction, wli, -_ray.direction(), wlo);
//				radiance += Material::lights.size() * p * tr * beta * tmp_rec.mat_ptr->Emitted(scattered, tmp_rec) / pdfval;
//			}
//
//			vec3 vi;
//			double wli;
//			double phase, pdfval;
//			respawn = mi->Sample_p(-_ray.direction(), _ray.central_wl, vi, wli, phase, pdfval);
//			if (respawn) {
//				scattered_point = _ray.point_at_parameter(medium_t);
//				scattered_direction = vi;
//				//beta *= phase / pdfval;
//				// don't need to calculate the term (phase / pdfval) because it's
//				// guaranteed that scattered direction is Sampled with a pdf which
//				// completely matches the phase function of the medium
//				scattering_coefficient = phase;
//			}
//
//
//
//			// Sample new direction at medium
//			IsLastBounceSpecular = false;
//
//			volume_bounce++;
//
//		} else { // surface interation
//			if (surface_bounce == 0 || IsLastBounceSpecular)
//				radiance += beta * rec.mat_ptr->Emitted(_ray, rec);
//
//			// calculate direct lighting
//			if (rec.mat_ptr->specular_flag == false) {
//				if (!enableNEE) {
//					vec3 generated_vi;
//					double wli;
//					ONB uvw_;
//					uvw_.BuildFromW(rec.normal);
//					UniformPdf pdf(rec.normal);
//					vec3 generated_direction = pdf.Generate();
//					//bool respawn = rec.mat_ptr->Sample(rec, uvw_, uvw_.WorldToLocal(-_ray.direction()), r.central_wl, generated_vi, wli, bxdf, pdfval);
//					generated_vi = uvw_.WorldToLocal(generated_direction);
//					bool respawn = true;
//					if (respawn) {
//						ray scattered = ray(rec.p, uvw_.LocalToWorld(generated_vi));
//						scattered.central_wl = _ray.central_wl;
//						scattered.min_wl = _ray.min_wl;
//						scattered.max_wl = _ray.max_wl;
//						wli = _ray.central_wl;
//						HitRecord tmp_rec;
//
//						bool hit = world->Hit(scattered, 0.001, std::numeric_limits<double>::max(), tmp_rec);
//						if (hit) {
//							double bxdf, pdfval;
//							pdfval = pdf.PdfVal(generated_direction);
//							bxdf = rec.mat_ptr->BxDF(generated_vi, wli, uvw_.WorldToLocal(-_ray.direction()), r.central_wl);
//							radiance += bxdf * (beta * tmp_rec.mat_ptr->Emitted(scattered, tmp_rec) * abs(generated_vi.z()) / pdfval);
//						}
//					}
//				} else {
//					if (rec.mat_ptr->light_flag == false) {
//						std::random_device rnd;
//						int selectedLight = rnd() % Material::lights.size();
//						ONB uvw_;
//						uvw_.BuildFromW(rec.normal);
//						HittablePdf pdf(Material::lights[selectedLight], rec.p);
//						vec3 generated_direction = pdf.Generate();
//
//						HitRecord light_rec;
//						ray scattered = ray(rec.p, generated_direction);
//						scattered.central_wl = _ray.central_wl;
//						scattered.min_wl = _ray.min_wl;
//						scattered.max_wl = _ray.max_wl;
//						bool hit = world->Hit(scattered, 0.001, std::numeric_limits<double>::max(), light_rec);
//						if (hit) {
//							double pdfval = pdf.PdfVal(generated_direction);
//							vec3 vi = uvw_.WorldToLocal(generated_direction);
//							vec3 vo = uvw_.WorldToLocal(-_ray.direction());
//							double wlo = _ray.central_wl;
//							double wli = wlo;
//							double BxDF = rec.mat_ptr->BxDF(vi, wli, vo, wlo);
//							radiance += Material::lights.size() * BxDF * (beta * light_rec.mat_ptr->Emitted(scattered, light_rec) * abs(vi.z())) / pdfval;
//						}
//
//
//
//					}
//				}
//			}
//
//
//
//			if (rec.mat_ptr->light_flag)
//				break;
//			vec3 generated_vi;
//			double wli;
//			double bxdf, pdf;
//			ONB uvw;
//			uvw.BuildFromW(rec.normal);
//			{
//				respawn = rec.mat_ptr->Sample(rec, uvw, uvw.WorldToLocal(-_ray.direction()), r.central_wl, generated_vi, wli, bxdf, pdf);
//				if (respawn) {
//					beta *= (bxdf * abs(generated_vi.z()) / pdf);
//					scattered_point = rec.p;
//					scattered_direction = uvw.LocalToWorld(generated_vi);
//					scattering_coefficient = bxdf;
//
//					// when light penetrates through a surface
//					if ((uvw.WorldToLocal(-_ray.direction()).z() * generated_vi.z()) < 0.0) {
//						if (generated_vi.z() > 0.0) { // going out
//							if (!inside_object_stack.empty()) {
//								if (inside_object_stack.top() != rec.hit_object_id)
//									std::cout << "ERROR" << std::endl;
//								else
//									inside_object_stack.pop();
//							} else {
//								//std::cout << "WARNING" << std::endl;
//								// maybe one of the object in the scene is not manifold
//							}
//						} else { // going in
//							inside_object_stack.push(rec.hit_object_id);
//						}
//					}
//					//if (rec.mat_ptr->specular_flag)
//					//	std::cout << bxdf *abs(generated_vi.z())/pdf<< std::endl;
//				}
//			}
//			IsLastBounceSpecular = rec.mat_ptr->specular_flag;
//
//			surface_bounce++;
//		}
//
//		//double prr = std::max(0.8, 1.0-(0.5 + bxdf/2.0));
//		//double prr = std::max(0.5, 1.0 + bxdf/2.0);
//		double prr = 1.0 - std::min(0.7, 0.2+scattering_coefficient/2.0);
//		//double prr = 0.5;
//		if (surface_bounce > 10)
//			prr = 0.9;
//		if (volume_bounce > 10)
//			prr = 0.9;
//		double d = drand48();
//		if (d < prr)
//			break;
//		beta /= (1.0-prr);
//
//		if (!respawn)
//			break;
//
//		ray scattered = ray(scattered_point, scattered_direction);
//		_ray.A = scattered.A;
//		_ray.B = scattered.B;
//
//	}
//
//	return radiance;
//
//}



