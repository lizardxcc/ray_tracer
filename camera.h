#ifndef CAMERA_H
#define CAMERA_H


#include "spectrum.h"

class Camera {
	public:
		Camera(){}
		Camera(dvec3 lookfrom, dvec3 axis, double rot_theta, double vfov, double aspect)
		{
			dvec3 forward = dvec3(0, -1, 0);
			dvec3 up = dvec3(0, 0, -1);
			forward = rodrigues(forward, axis, rot_theta);
			up = rodrigues(up, axis, rot_theta);
			double theta = vfov*M_PI/180;
			double half_height = tan(theta/2);
			double half_width = aspect * half_height;
			origin = lookfrom;
			dvec3 w = unit_vector(-forward);
			dvec3 u = unit_vector(cross(up, w));
			dvec3 v = cross(w, u);
			lower_left_corner = origin - half_width * u - half_height * v - w;
			horizontal = 2 * half_width * u;
			vertical = 2 * half_height * v;
		}
		void set_Camera(dvec3 lookfrom, dvec3 lookat, dvec3 vup, double vfov, double aspect)
		{
			double theta = vfov*M_PI/180;
			double half_height = tan(theta/2);
			double half_width = aspect * half_height;
			origin = lookfrom;
			dvec3 w = unit_vector(lookfrom - lookat);
			dvec3 u = unit_vector(cross(vup, w));
			dvec3 v = cross(w, u);
			lower_left_corner = origin - half_width * u - half_height * v - w;
			horizontal = 2 * half_width * u;
			vertical = 2 * half_height * v;
		}

		ray get_ray(double u, double v, double& p_image, double& p_lens)
		{
			return ray(origin, lower_left_corner + u*horizontal + v*vertical - origin);
		}

		dvec3 origin;
		dvec3 lower_left_corner;
		dvec3 horizontal;
		dvec3 vertical;
};


class PinholeCamera {
	public:
		PinholeCamera(void)
		{
		}
		PinholeCamera(dvec3 lookfrom, dvec3 lookat, dvec3 vup, double d, double aspect)
		{
			double half_height = 0.5;
			double half_width = aspect * half_height;
			this->d = d;
			origin = lookfrom;
			dvec3 w = unit_vector(lookfrom - lookat);
			dvec3 u = unit_vector(cross(vup, w));
			dvec3 v = cross(w, u);
			film_lower_left_corner = origin - half_width * u - half_height * v + d * w;
			pinhole = origin;
			horizontal = 2 * half_width * u;
			vertical = 2 * half_height * v;
		}
		void set_Camera(dvec3 lookfrom, dvec3 lookat, dvec3 vup, double vfov, double aspect)
		{
			double half_height = 0.5;
			double half_width = aspect * half_height;
			this->d = 0.5/tan(vfov/2.0);
			origin = lookfrom;
			dvec3 w = unit_vector(lookfrom - lookat);
			dvec3 u = unit_vector(cross(vup, w));
			dvec3 v = cross(w, u);
			film_lower_left_corner = origin - half_width * u - half_height * v + d * w;
			pinhole = origin;
			horizontal = 2.0 * half_width * u;
			vertical = 2.0 * half_height * v;
		}

		ray get_ray(double u, double v, double& p_image, double& p_lens)
		{
			//return ray(origin, unit_vector(origin-(film_lower_left_corner + u*horizontal + v*vertical)));
			return ray(origin, unit_vector(origin-(film_lower_left_corner + u*horizontal + v*vertical)));
		}

		dvec3 origin;
		dvec3 film_lower_left_corner;
		dvec3 pinhole;
		dvec3 horizontal;
		dvec3 vertical;
		double d;
};

class LensCamera {
	public:
		LensCamera() {}
		// Lens Maker's Formula
		// 1/f = (n-1) * (1/R1 - 1/R2)
		// In this source code, LensMakerCoefficient denotes (1/R1 - 1/R2) in Lens Maker's Formula

		void SetLens(double vfov, double focal_length)
		{
			//this->film_height = film_height;
			//this->film_width = aspect*film_height;
			//double half_height = 0.5 * film_height;
			//double half_width = 0.5 * film_width;
			double theta = vfov*M_PI/180;

			// イメージセンサとレンズの間の距離
			this->d = (0.5*film_height)/tan(theta/2.0);
			//this->focal_length = focal_length;
			//this->aperture = aperture; // aperture of the lens

			//LensMakerCoefficient = 1.0/(focal_length*(n.data[0]-1));
			LensMakerCoefficient = 1.0/(focal_length*(n.data[N_SAMPLE/2]-1));
			// レンズとオブジェクトプレーンの間の距離
			//this->a = 1.0/(1.0/focal_length - 1.0/d);
		}

		//void set_Camera(dvec3 lookfrom, dvec3 lookat, dvec3 vup, double aspect, double d, double focal_length, double aperture)
		void set_Camera(dvec3 lookfrom, dvec3 lookat, dvec3 vup)
		{
			//this->n = n;
			//this->film_height = film_height;
			//this->film_width = aspect*film_height;
			//double half_height = 0.5 * film_height;
			//double half_width = aspect * half_height;

			// レンズの中心
			origin = lookfrom;
			w = unit_vector(lookfrom - lookat);
			u = unit_vector(cross(vup, w));
			v = cross(w, u);
			film_lower_left_corner = origin - 0.5*film_width * u - 0.5*film_height * v + d * w;
			pinhole = origin;
			horizontal = film_width * u;
			vertical = film_height * v;
		}

		// sample from a circle whose diameter equals to aperture
		dvec3 get_random_on_lens(void)
		{
			dvec3 p = dvec3(0, 0, 0);
			//return p;
			do {
				p[0] = 2*drand48()-1.0;
				p[1] = 2*drand48()-1.0;
			} while(p.length() > 1.0);
			return p*(aperture/2.0);
		}

		ray get_ray(double u, double v, double wavelength, double& p_image, double& p_lens, double& cos_theta)
		{
			double focal_length = LensMakersFormula(wavelength);
			// レンズとオブジェクトプレーンの間の距離
			double a = 1.0/(1.0/focal_length - 1.0/d);

			//dvec3 target = (-a/d * (film_lower_left_corner + u*horizontal + v*vertical - origin));

			// イメージセンサの座標
			dvec3 image_sensor_pos = film_lower_left_corner + u*horizontal + v*vertical;
			// イメージセンサ上のuvからレンズの中心を通る直線とオブジェクトプレーン上の交点
			dvec3 target = origin+((-a/d) * (image_sensor_pos - origin));
			dvec3 tmp = get_random_on_lens();
			dvec3 random_point_on_lens = origin+tmp[0]*horizontal + tmp[1]*vertical;


			{
				cos_theta = dot(unit_vector(random_point_on_lens - image_sensor_pos), -w);
				p_image = 1.0/(film_width*film_height);
				p_lens = 1.0/(M_PI*aperture*aperture/4.0);
			}
			//return ray(random_point_on_disk, target-random_point_on_disk);
			//return ray(origin+random_point_on_lens, target);
			if (a > 0.0)
				return ray(random_point_on_lens, unit_vector(target-random_point_on_lens));
			else
				return ray(random_point_on_lens, -unit_vector(target-random_point_on_lens));
			//return ray(origin, origin-(film_lower_left_corner + u*horizontal + v*vertical));
		}

		double film_sensitivity = 1.0;
		double aperture = 0.5;
		double film_width = 0.024;
		double film_height = 0.024;
		Spectrum n = Spectrum(2.0);
		double d;

		// returns focal length
		double LensMakersFormula(double wavelength)
		{
			return 1.0/((n.get(wavelength)-1)*LensMakerCoefficient);
		}
	private:
		double LensMakerCoefficient;
		dvec3 origin;
		dvec3 film_lower_left_corner;
		dvec3 pinhole;
		dvec3 horizontal;
		dvec3 vertical;
		dvec3 w, u, v;
};

#endif
