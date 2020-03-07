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

void Renderer::LoadMaterials(const std::map<std::string, std::shared_ptr<NodeMaterial>>& materials)
{
	light_objects.clear();
	for (size_t i = 0; i < world->models.size(); i++) {
		auto itr = materials.find(obj_loader.objects[i]->name);
		if (itr == materials.end()) {
			continue;
		}
		world->models[i]->SetMaterial(itr->second.get());
		if (itr->second->light_flag) {
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


void Renderer::RenderImage(int nx, int ny, int ns, int spectral_samples, bool enable_openmp, bool print_progress)
{
	spectrum_img.resize(nx*ny);
	preview_img.resize(nx*ny);
	std::fill(spectrum_img.begin(), spectrum_img.end(), 0.0);

	rendering_runnnig = true;
	if (print_progress) {
#ifdef _OPENMP
		std::cout << "max threads: " << omp_get_max_threads() << std::endl;
#endif
	}

	int s;

	for (s = 0; s < ns; s++) {
		if (stop_rendering)
			break;
		int i, j;
		int count = 0;
#ifdef _OPENMP
		//#pragma omp parallel for private(j, s) schedule(dynamic) if (enable_openmp)
#pragma omp parallel for private(j) schedule(dynamic) if (enable_openmp)
#endif
		for (i = 0; i < nx; i++) {
			for (j = 0; j < ny; j++) {

				double u = (i + drand48()) / nx;
				double v = (j + drand48()) / ny;
				double p_image, p_lens, cos_theta;

				int k = mt()%spectral_samples;
				double min_wl = 400.0 + 300.0/spectral_samples*k;
				double max_wl = min_wl + 300.0/spectral_samples - 0.00001;
				double central_wl = (min_wl + max_wl) / 2.0;
				//double max_wl = min_wl + SAMPLE_SIZE - 0.00001;
				//double max_wl = 400 + 300.0/(double)num*(double)(k+1) - 0.00001;
				ray r = cam.get_ray(u, v, central_wl, p_image, p_lens, cos_theta);
				r.min_wl = min_wl;
				r.max_wl = max_wl;
				r.central_wl = central_wl;
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
						break;
					case NaiveBDPTType:
						rad = NaiveBDPT(r);
						break;
					case MISBDPTType:
						rad = MISBDPT(r);
						break;
				}
				double geometry_factor = pow(cos_theta, 4.0)/(cam.d*cam.d);
				rad *= cam.film_sensitivity * geometry_factor /(p_image*p_lens);
				if (!std::isnan(rad)) {
					spectrum_img[i*ny+j].add(rad/ns, min_wl, max_wl);
				}
			}
#ifdef _OPENMP
			if (print_progress)
				count = omp_get_num_threads();
#endif
		}
		if (preview_img_flag || s == ns-1) {
			int c;
#ifdef _OPENMP
#pragma omp parallel for private(j, c) if (enable_openmp)
#endif
			for (i = 0; i < nx; i++) {
				for (j = 0; j < ny; j++) {
					dvec3 rgb_col = rgb(spectrum_img[i*ny+j]*(ns/static_cast<double>(s+1)));
					for (c = 0; c < 3; c++) {
						if (rgb_col[c] >= 0.0) {
							rgb_col[c] = pow(rgb_col[c], 1.0/2.2);
							rgb_col[c] = std::min(rgb_col[c], 1.0);
						} else
							rgb_col[c] = 0.0;
					}
					preview_img[i*ny+j] = rgb_col;
				}
			}
			img_updated = true;
		}
		progress = static_cast<float>(s+1) / ns;
		if (print_progress) {
			std::cout << progress*100.0 << "%";
#ifdef _OPENMP
			std::cout << " running " << count << " threads";
#endif
			std::cout << std::endl;
		}
	}

	img_updated = true;
	if (stop_rendering)
		stop_rendering = false;
	rendering_runnnig = false;
	if (print_progress)
		std::cout << "Completed" << std::endl;
}


double Renderer::NaivePathTracing(const ray& r)
{
	HitRecord rec;
	ray _ray = r;
	double radiance = 0.0;
	double beta = 1.0;
	Sphere sphere;
	sphere.center = dvec3(0.0, 0.0, 0.0);
	sphere.radius = 100.0;
	while (1) {
		bool hit = world->Hit(_ray, 0.001, std::numeric_limits<double>::max(), rec);

		if (!hit) {
			if (env_mapping_texture == nullptr)
				break;
			bool hit = sphere.Hit(_ray, 0.001, std::numeric_limits<double>::max(), rec);
			if (hit) {
				dvec3 p = unit_vector(_ray.point_at_parameter(rec.t));
				double theta = acos(p.y());
				double phi = atan2(p.x()/sin(theta), p.z()/sin(theta));
				phi += M_PI;
				int x = env_mapping_width - (int)(phi/ (2.0*M_PI) * env_mapping_width);
				int y = (int)(theta/ M_PI * env_mapping_height);
				dvec3 rgb;
				rgb[0] = static_cast<double>(env_mapping_texture[env_mapping_bpp*(x+y*env_mapping_width)])/255.0;
				rgb[1] = static_cast<double>(env_mapping_texture[env_mapping_bpp*(x+y*env_mapping_width)+1])/255.0;
				rgb[2] = static_cast<double>(env_mapping_texture[env_mapping_bpp*(x+y*env_mapping_width)+2])/255.0;
				Spectrum spectrum = RGBtoSpectrum(rgb * env_brightness);
				radiance += beta * spectrum.get(_ray.central_wl);
			}
			break;
		}

		//rec.mat_ptr->PreProcess(rec);
		radiance += beta * rec.mat_ptr->Emitted(_ray, rec, rec.vt);

		double bxdf_divided_by_pdf;
		double bxdf, pdf;
		ONB uvw;
		uvw.BuildFromW(rec.normal); //法線をW軸としてローカルONBを生成
		dvec3 generated_vi;
		double wli;
		bool respawn = rec.mat_ptr->SampleBSDF(ImportanceType, rec, uvw, uvw.WorldToLocal(-_ray.direction()), r.central_wl, generated_vi, wli, bxdf_divided_by_pdf, bxdf, pdf);
		if (!respawn)
			break;
		beta *= (bxdf_divided_by_pdf * abs(generated_vi.z()));

		// パスを打ち切るロシアンルーレット
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

		//bool preprocessed = false;

			if (rec.mat_ptr->light_flag) {
				if (bounce != 0) {
					// we assume that light objects absorb light completeley
					break;
				}
				//rec.mat_ptr->PreProcess(rec);
				//preprocessed = true;
				const double light = rec.mat_ptr->Emitted(_ray, rec, rec.vt);
				radiance += light;
			}
		// sample light
		{
			int selected_light = mt() % light_objects.size();
			dvec3 p, light_normal;
			double area;
			light_objects[selected_light]->GetRandomPointOnPolygon(p, area, light_normal);
			ray shadow_ray = ray(rec.p, unit_vector(p - rec.p));
			shadow_ray.central_wl = r.central_wl;
			shadow_ray.min_wl = r.min_wl;
			shadow_ray.max_wl = r.max_wl;

			if (!world->Occluded(shadow_ray, 0.0001, (p-rec.p).length()-0.0001)) {
				//if (!preprocessed) {
				//	rec.mat_ptr->PreProcess(rec);
				//	preprocessed = true;
				//}
				ONB uvw_;
				uvw_.BuildFromW(rec.normal);
				const dvec3 vi = uvw_.WorldToLocal(unit_vector(p - rec.p));
				const dvec3 vo = uvw_.WorldToLocal(-_ray.direction());
				const double wlo = _ray.central_wl;
				const double wli = wlo;
				const double BSDF = rec.mat_ptr->BSDF(vi, wli, vo, wlo, rec.vt);
				const double G = abs(dot(shadow_ray.direction(), light_objects[selected_light]->face_normal) * dot(shadow_ray.direction(), rec.normal) / (p-rec.p).squared_length());
				const double pdfarea = 1.0 / area / light_objects.size();
				HitRecord light_rec;
				if (light_objects[selected_light]->Hit(shadow_ray, 0.0001, (p-rec.p).length()+0.0001, light_rec)) {
					//light_rec.mat_ptr->PreProcess(light_rec);
					const double light = light_rec.mat_ptr->Emitted(shadow_ray, light_rec, light_rec.vt);
					const double add = BSDF * beta * light * G / pdfarea;
					radiance += add;
				} else {
					// bug?
				}
			}
		}


		dvec3 generated_vi;
		double wli;
		double bxdf_divided_by_pdf;
		double bxdf, pdf;
		ONB uvw;
		uvw.BuildFromW(rec.normal);
		//if (!preprocessed)
		//	rec.mat_ptr->PreProcess(rec);
		bool respawn = rec.mat_ptr->SampleBSDF(ImportanceType, rec, uvw, uvw.WorldToLocal(-_ray.direction()), _ray.central_wl, generated_vi, wli, bxdf_divided_by_pdf, bxdf, pdf);
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

		// estimate direct illumination

		//bool preprocessed = false;

		if (rec.mat_ptr->light_flag) {
			// We assume that light objects absorb light completeley
			if (bounce != 0) {
				break;
			}
			//rec.mat_ptr->PreProcess(rec);
			//preprocessed = true;
			const double light = rec.mat_ptr->Emitted(_ray, rec, rec.vt);
			radiance += light;
		}
		int selected_light = mt() % light_objects.size();

		double addition = 0.0;

		// sample light
		{
			dvec3 point_on_light, light_normal;
			double area;
			light_objects[selected_light]->GetRandomPointOnPolygon(point_on_light, area, light_normal);
			ray shadow_ray = ray(rec.p, unit_vector(point_on_light - rec.p));
			shadow_ray.central_wl = r.central_wl;
			shadow_ray.min_wl = r.min_wl;
			shadow_ray.max_wl = r.max_wl;

			bool notoccluded = false;
			HitRecord light_rec;
			bool light_hit = world->Hit(shadow_ray, 0.001, (point_on_light-rec.p).length()+0.001, light_rec);
			if (light_hit) {
				if (light_rec.hit_object == light_objects[selected_light])
					notoccluded = true;
			}

			if (notoccluded) {
				//if (!preprocessed) {
				//	rec.mat_ptr->PreProcess(rec);
				//	preprocessed = true;
				//}
				ONB uvw_;
				uvw_.BuildFromW(rec.normal);
				const dvec3 vi = uvw_.WorldToLocal(unit_vector(point_on_light - rec.p));
				const dvec3 vo = uvw_.WorldToLocal(-_ray.direction());
				const double wlo = _ray.central_wl;
				const double wli = wlo;
				const double BSDF = rec.mat_ptr->BSDF(vi, wli, vo, wlo, rec.vt);
				const double G = GeometryTerm(light_rec, rec);
				const double light_pdf_area = 1.0 / area / light_objects.size();
				//light_rec.mat_ptr->PreProcess(light_rec);
				const double light = light_rec.mat_ptr->Emitted(shadow_ray, light_rec, light_rec.vt);
				const double light_scale = beta * BSDF * G / light_pdf_area;

				const double scattering_pdf_solidangle = rec.mat_ptr->PDF(vi, wli, vo, wlo, rec.vt);
				const double scattering_pdf_area = scattering_pdf_solidangle * G/std::abs(dot(shadow_ray.direction(), rec.normal));
				double mis_weight = PowerHeuristic(light_pdf_area, scattering_pdf_area, 2.0);
				addition += mis_weight * light_scale * light;
			}
		}

		// sample bsdf
		{
			double bsdf_divided_by_pdf;
			double bsdf, bsdf_scattering_pdf_solidangle;
			ONB uvw;
			uvw.BuildFromW(rec.normal);
			dvec3 generated_vi;
			double wli;
			bool respawn = rec.mat_ptr->SampleBSDF(ImportanceType, rec, uvw, uvw.WorldToLocal(-_ray.direction()), r.central_wl, generated_vi, wli, bsdf_divided_by_pdf, bsdf, bsdf_scattering_pdf_solidangle);
			if (respawn) {
				ray shadow_ray = ray(rec.p, uvw.LocalToWorld(generated_vi));
				shadow_ray.central_wl = r.central_wl;
				shadow_ray.min_wl = r.min_wl;
				shadow_ray.max_wl = r.max_wl;
				HitRecord light_rec;
				bool light_hit = world->Hit(shadow_ray, 0.001, std::numeric_limits<double>::max(), light_rec);
				if (light_hit) {

					if (light_rec.hit_object == light_objects[selected_light]) { // 必要？
						const double light = light_rec.mat_ptr->Emitted(shadow_ray, light_rec, light_rec.vt);
						const double G = GeometryTerm(light_rec, rec);
						const double light_pdf_area = 1.0 / light_objects[selected_light]->polygon_area / light_objects.size();
						const double bsdf_scale = beta * bsdf_divided_by_pdf * abs(generated_vi.z());
						const double bsdf_scattering_pdf_area = G * bsdf_scattering_pdf_solidangle / abs(generated_vi.z());

						const double mis_weight = PowerHeuristic(bsdf_scattering_pdf_area, light_pdf_area, 2.0);
						addition += mis_weight * bsdf_scale * light;
					}
				}
			}
		}

		radiance += addition;

		// Estiamting direct illumination finished


		dvec3 generated_vi;
		double wli;
		double bxdf_divided_by_pdf;
		double bxdf, pdf;
		ONB uvw;
		uvw.BuildFromW(rec.normal);
		//if (!preprocessed)
		//	rec.mat_ptr->PreProcess(rec);
		bool respawn = rec.mat_ptr->SampleBSDF(ImportanceType, rec, uvw, uvw.WorldToLocal(-_ray.direction()), _ray.central_wl, generated_vi, wli, bxdf_divided_by_pdf, bxdf, pdf);
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


double Renderer::NaiveBDPT(const ray& r)
{
	double radiance = 0.0;
	//sample eye path
	ray _ray = r;
	size_t eyepath_size = 0;
	double alpha = 1.0, alpha_ = 1.0;

	std::vector<double> alphas;
	std::vector<HitRecord> eyepath_edge_recs;
	std::vector<ray> eyepath_edge_rays;
	while (1) {
		HitRecord rec;
		bool hit = world->Hit(_ray, 0.001, std::numeric_limits<double>::max(), rec);
		if (!hit) {
			break;
		}
		if (rec.mat_ptr->light_flag)
			break;
		alpha *= alpha_;
		dvec3 generated_vi;
		double wli, bsdf_divided_by_pdf, bsdf, pdf;
		ONB uvw;
		uvw.BuildFromW(rec.normal);
		//rec.mat_ptr->PreProcess(rec);
		bool respawn = rec.mat_ptr->SampleBSDF(ImportanceType, rec, uvw, uvw.WorldToLocal(-_ray.direction()), r.central_wl, generated_vi, wli, bsdf_divided_by_pdf, bsdf, pdf);
		if (!respawn)
			break;

		alpha_ = bsdf_divided_by_pdf * abs(generated_vi.z());

		alphas.push_back(alpha);
		eyepath_edge_recs.push_back(rec);
		eyepath_edge_rays.push_back(_ray);


		double prr = 0.5;
		eyepath_size++;
		if (eyepath_size > 2)
			prr = 0.9;
		double d = drand48();
		if (d < prr)
			break;
		alpha_ /= (1.0-prr);

		_ray.A = rec.p;
		_ray.B = unit_vector(uvw.LocalToWorld(generated_vi));
		_ray.central_wl = r.central_wl;
		_ray.min_wl = r.min_wl;
		_ray.max_wl = r.max_wl;

	}
	assert(eyepath_size == alphas.size());
	assert(eyepath_size == eyepath_edge_recs.size());
	assert(eyepath_size == eyepath_edge_rays.size());

	
	//sample light path
	
	// sample point on light
	dvec3 point_on_light, light_normal;
	int selected_light = mt() % light_objects.size();
	double light_area;
	light_objects[selected_light]->GetRandomPointOnPolygon(point_on_light, light_area, light_normal);
	//double light = light_objects[selected_light]
	double light_pdf_area = 1.0/light_area/light_objects.size();
	// sample direction from light
	CosinePdf pdf(light_normal);
	dvec3 dir = pdf.Generate();
	double light_dir_pdf_solidangle = pdf.PdfVal(dir);
	double light_dir_pdf_projected_solidangle = light_dir_pdf_solidangle / dot(light_normal, dir);
	double light_value = 0.0;

	// caluclate light value
	HitRecord sampled_light_rec;
	{
	ray shadow_ray = ray(point_on_light-0.002*dir, -dir);
	shadow_ray.central_wl = r.central_wl;
	shadow_ray.min_wl = r.min_wl;
	shadow_ray.max_wl = r.max_wl;
	bool hit = world->Hit(_ray, 0.001, std::numeric_limits<double>::max(), sampled_light_rec);
	if (hit)
		light_value = light_objects[selected_light]->mat_ptr->Emitted(shadow_ray, sampled_light_rec, sampled_light_rec.vt);
	}

	
	_ray.A = point_on_light;
	_ray.B = dir;
	_ray.central_wl = r.central_wl;
	_ray.max_wl = r.max_wl;
	_ray.min_wl = r.min_wl;
	//HitRecord lightpath_edge_rec;
	//ray lightpath_edge_ray;
	double beta = 1.0, beta_ = 1.0;
	std::vector<double> betas;
	std::vector<HitRecord> lightpath_edge_recs;
	std::vector<ray> lightpath_edge_rays;
	size_t lightpath_bounce = 0;
	while (1) {
		HitRecord rec;
		bool hit = world->Hit(_ray, 0.001, std::numeric_limits<double>::max(), rec);
		if (!hit)
			break;
		if (rec.mat_ptr->light_flag)
			break;
		beta *= beta_;
		dvec3 generated_vi;
		double wli, bsdf_divided_by_pdf, bsdf, pdf;
		ONB uvw;
		uvw.BuildFromW(rec.normal);
		//rec.mat_ptr->PreProcess(rec);
		bool respawn = rec.mat_ptr->SampleBSDF(LightType, rec, uvw, uvw.WorldToLocal(-_ray.direction()), _ray.central_wl, generated_vi, wli, bsdf_divided_by_pdf, bsdf, pdf);
		if (!respawn)
			break;
		beta_ = bsdf_divided_by_pdf * abs(generated_vi.z());

		betas.push_back(beta);
		lightpath_edge_recs.push_back(rec);
		lightpath_edge_rays.push_back(_ray);


		double prr = 0.5;
		lightpath_bounce++;
		if (lightpath_bounce > 2)
			prr = 0.9;
		double d = drand48();
		if (d < prr)
			break;
		beta_ /= (1.0-prr);

		_ray.A = rec.p;
		_ray.B = unit_vector(uvw.LocalToWorld(generated_vi));
		_ray.central_wl = r.central_wl;
		_ray.min_wl = r.min_wl;
		_ray.max_wl = r.max_wl;
	}

	assert(lightpath_bounce == betas.size());
	assert(lightpath_bounce == lightpath_edge_recs.size());
	assert(lightpath_bounce == lightpath_edge_rays.size());

	std::vector<double> radiances(eyepath_size+(lightpath_bounce+1), 0.0); // 経路長毎に分ける
	std::vector<size_t> path_numbers(eyepath_size+(lightpath_bounce+1), 0);
	// radiances[i] = total length i+1のradianceの和
	// path_nubmers[i] = total length i+1のradianceの計算に使った経路の個数
	// calculation of total length
	// s = eyepath_size
	// t = lightpath_bounce+1
	// if t == 0 then total length = s+1 = eyepath_size+1
	// else total length = s+t = eyepath_size+lightpath_bounce+1

	// path contribution for paths such that 
	// s = 0 and t = 0 (t = 0 means implicit path in terms of implicit path)
	// total length = 1
	{
		size_t total_length = 1;
		path_numbers[total_length-1]++;
		HitRecord rec;
		if (world->Hit(r, 0.001, std::numeric_limits<double>::max(), rec)) {
			if (rec.mat_ptr->light_flag) {
				//radiance += rec.mat_ptr->Emitted(r, rec, rec.vt)/(0+0+1);
				radiances[total_length-1] += rec.mat_ptr->Emitted(r, rec, rec.vt)/(0+0+1);
			}
		}
	}

	// We ignore path contributios for paths such that 
	// s =  0 and t = 1
	// We ignore path contributios for paths such that 
	// s =  0 and t >= 1
	

	// path contribution for paths such that
	// s >= 1
	// t = 0 (t = 0 means implicit path in terms of naive path tracing)
	// total length = s+1 = e+2
	for (size_t e = 0; e < eyepath_size; e++) {
		size_t total_length = e+2;
		path_numbers[total_length-1]++;
		double bsdf_divided_by_pdf;
		double bsdf, bsdf_scattering_pdf_solidangle;
		ONB uvw;
		uvw.BuildFromW(eyepath_edge_recs[e].normal);
		dvec3 generated_vi;
		double wli;
		bool respawn = eyepath_edge_recs[e].mat_ptr->SampleBSDF(ImportanceType, eyepath_edge_recs[e], uvw, uvw.WorldToLocal(-eyepath_edge_rays[e].direction()), r.central_wl, generated_vi, wli, bsdf_divided_by_pdf, bsdf, bsdf_scattering_pdf_solidangle);
		if (!respawn)
			continue;
		ray tmp_ray = ray(eyepath_edge_recs[e].p, uvw.LocalToWorld(generated_vi));
		tmp_ray.central_wl = r.central_wl;
		tmp_ray.min_wl = r.min_wl;
		tmp_ray.max_wl = r.max_wl;

		HitRecord light_rec;
		bool hit = world->Hit(tmp_ray, 0.001, std::numeric_limits<double>::max(), light_rec);
		if (!hit)
			continue;
		if (!light_rec.mat_ptr->light_flag)
			continue;

		const double light = light_rec.mat_ptr->Emitted(tmp_ray, light_rec, light_rec.vt);


		////radiance += alphas[e] * bsdf * GeometryTerm(eyepath_edge_recs[e], light_rec) * light_rec.mat_ptr->Emitted(tmp_ray, light_rec, light_rec.vt)/light_pdf_area/(e+0+1);
		//radiance += alphas[e] * bsdf_divided_by_pdf * std::abs(generated_vi.z()) *light/((e+1)+0+1 -1); // last -1 is due to ignoring path contributions (s = 0, t>=1)
		//radiance += alphas[e] * bsdf_divided_by_pdf * std::abs(generated_vi.z()) *light/(eyepath_size*lightpath_bounce); // last -1 is due to ignoring path contributions (s = 0, t>=1)
		//radiance += alphas[e] * bsdf_divided_by_pdf * std::abs(generated_vi.z()) *light; // last -1 is due to ignoring path contributions (s = 0, t>=1)
		radiances[total_length-1] += alphas[e] * bsdf_divided_by_pdf * std::abs(generated_vi.z()) *light; // last -1 is due to ignoring path contributions (s = 0, t>=1)
	}

	// path contribution for paths such that
	// s >= 1
	// t = 1 (explicit path in terms of naive path tracing)
	// total length = s+t = s+1 = e+2
	for (size_t e = 0; e < eyepath_size; e++) {
		size_t total_length = e+2;
		path_numbers[total_length-1]++;
		ray tmp_ray = ray(eyepath_edge_recs[e].p, unit_vector(point_on_light - eyepath_edge_recs[e].p));
		tmp_ray.central_wl = r.central_wl;
		tmp_ray.min_wl = r.min_wl;
		tmp_ray.max_wl = r.max_wl;
		HitRecord light_rec;
		bool hit = world->Hit(tmp_ray, 0.001, std::numeric_limits<double>::max(), light_rec);
		if (!hit)
			continue;
		if (light_rec.hit_object != light_objects[selected_light])
			continue;
		ONB uvw;
		uvw.BuildFromW(eyepath_edge_recs[e].normal);
		dvec3 vi = uvw.WorldToLocal(unit_vector(point_on_light-eyepath_edge_recs[e].p));
		dvec3 vo = uvw.WorldToLocal(-eyepath_edge_rays[e].direction());
		double wli = r.central_wl;
		double wlo = r.central_wl;
		// phsyciall light passes from vi to vo
		double bsdf = eyepath_edge_recs[e].mat_ptr->BSDF(vi, wli, vo, wlo, eyepath_edge_recs[e].vt);
		//radiance += alphas[e] * bsdf * GeometryTerm(eyepath_edge_recs[e], light_rec) * light_rec.mat_ptr->Emitted(tmp_ray, light_rec, light_rec.vt)/light_pdf_area/((e+1)+1+1 -1);
		radiances[total_length-1] += alphas[e] * bsdf * GeometryTerm(eyepath_edge_recs[e], light_rec) * light_rec.mat_ptr->Emitted(tmp_ray, light_rec, light_rec.vt)/light_pdf_area;

	}

	// path contribution for paths whose
	// s >= 1
	// t > 1
	// total length = s+t = eyepath_size + lightpath_bounce+1 = e+1 + l+1+1
	for (size_t e = 0; e < eyepath_size; e++) {
		for (size_t l = 0; l < lightpath_bounce; l++) {
			size_t total_length = (e+1)+(l+2);
			path_numbers[total_length-1]++;
			const HitRecord& eyepath_edge_rec = eyepath_edge_recs[e];
			const HitRecord& lightpath_edge_rec = lightpath_edge_recs[l];
			const ray& eyepath_edge_ray = eyepath_edge_rays[e];
			const ray& lightpath_edge_ray = lightpath_edge_rays[l];
			ray tmp_ray = ray(eyepath_edge_rec.p, unit_vector(lightpath_edge_rec.p-eyepath_edge_rec.p));
			tmp_ray.central_wl = r.central_wl;
			tmp_ray.min_wl = r.min_wl;
			tmp_ray.max_wl = r.max_wl;
			if (world->Occluded(tmp_ray, 0.001, (lightpath_edge_rec.p-eyepath_edge_rec.p).length()-0.001))
				continue;
			double eyepath_bsdf, lightpath_bsdf;
			{
				ONB uvw;
				uvw.BuildFromW(eyepath_edge_rec.normal);
				dvec3 vi = uvw.WorldToLocal(unit_vector(lightpath_edge_rec.p-eyepath_edge_rec.p));
				dvec3 vo = uvw.WorldToLocal(-eyepath_edge_ray.direction());
				double wli = r.central_wl;
				double wlo = r.central_wl;
				// phsyciall light passes from vi to vo
				eyepath_bsdf = eyepath_edge_rec.mat_ptr->BSDF(vi, wli, vo, wlo, eyepath_edge_rec.vt);
			}
			{
				ONB uvw;
				uvw.BuildFromW(lightpath_edge_rec.normal);
				dvec3 vi = uvw.WorldToLocal(-lightpath_edge_ray.direction());
				dvec3 vo = uvw.WorldToLocal(unit_vector(eyepath_edge_rec.p-lightpath_edge_rec.p));
				double wli = r.central_wl;
				double wlo = r.central_wl;
				lightpath_bsdf = lightpath_edge_rec.mat_ptr->BSDF(vi, wli, vo, wlo, lightpath_edge_rec.vt);
			}
			//std::cout << "eyebsdf: " << eyepath_bsdf << " lightbsdf: " << lightpath_bsdf << std::endl;
			//radiance += alphas[e] * betas[l] * eyepath_bsdf * lightpath_bsdf * GeometryTerm(eyepath_edge_rec, lightpath_edge_rec)*light_value/(light_pdf_area*light_dir_pdf_projected_solidangle) / ((e+1)+(l+1)+1 -1);
			//radiance += alphas[e] * betas[l] * eyepath_bsdf * lightpath_bsdf * GeometryTerm(eyepath_edge_rec, lightpath_edge_rec)*light_value/(light_pdf_area*light_dir_pdf_projected_solidangle) / (eyepath_size*lightpath_bounce);
			//radiance += alphas[e] * betas[l] * eyepath_bsdf * lightpath_bsdf * GeometryTerm(eyepath_edge_rec, lightpath_edge_rec)*light_value/(light_pdf_area*light_dir_pdf_projected_solidangle) / (eyepath_size*lightpath_bounce);
			radiances[total_length-1] += alphas[e] * betas[l] * eyepath_bsdf * lightpath_bsdf * GeometryTerm(eyepath_edge_rec, lightpath_edge_rec)*light_value/(light_pdf_area*light_dir_pdf_projected_solidangle);
		}
	}

	radiance = 0.0;
	for (size_t i = 0; i < radiances.size(); i++) {
		if (path_numbers[i] == 0)
			continue;
		radiance += radiances[i]/path_numbers[i];
	}
	return radiance;
}



double Renderer::MISBDPT(const ray& r)
{
	return 0.0;
}

double Renderer::GeometryTerm(double cos_theta0, double cos_theta1, double r)
{
	return std::abs(cos_theta0*cos_theta1)/(r*r);
}
double Renderer::GeometryTerm(const HitRecord& rec0, const HitRecord& rec1)
{
	dvec3 v = rec1.p - rec0.p;
	dvec3 dir = unit_vector(rec1.p - rec0.p);
	return std::abs(dot(dir, rec0.normal)*dot(dir, rec1.normal)/v.squared_length());
}


