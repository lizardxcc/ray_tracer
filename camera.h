#ifndef CAMERA_H
#define CAMERA_H

class Camera {
	public:
		Camera(){}
		Camera(vec3 lookfrom, vec3 axis, double rot_theta, double vfov, double aspect)
		{
			vec3 forward = vec3(0, -1, 0);
			vec3 up = vec3(0, 0, -1);
			forward = rodrigues(forward, axis, rot_theta);
			up = rodrigues(up, axis, rot_theta);
			double theta = vfov*M_PI/180;
			double half_height = tan(theta/2);
			double half_width = aspect * half_height;
			origin = lookfrom;
			vec3 w = unit_vector(-forward);
			vec3 u = unit_vector(cross(up, w));
			vec3 v = cross(w, u);
			lower_left_corner = origin - half_width * u - half_height * v - w;
			horizontal = 2 * half_width * u;
			vertical = 2 * half_height * v;
		}
		void set_Camera(vec3 lookfrom, vec3 lookat, vec3 vup, double vfov, double aspect)
		{
			double theta = vfov*M_PI/180;
			double half_height = tan(theta/2);
			double half_width = aspect * half_height;
			origin = lookfrom;
			vec3 w = unit_vector(lookfrom - lookat);
			vec3 u = unit_vector(cross(vup, w));
			vec3 v = cross(w, u);
			lower_left_corner = origin - half_width * u - half_height * v - w;
			horizontal = 2 * half_width * u;
			vertical = 2 * half_height * v;
		}

		ray get_ray(double u, double v)
		{
			return ray(origin, lower_left_corner + u*horizontal + v*vertical - origin);
		}

		vec3 origin;
		vec3 lower_left_corner;
		vec3 horizontal;
		vec3 vertical;
};


class PinholeCamera {
	public:
		PinholeCamera(vec3 lookfrom, vec3 lookat, vec3 vup, double aspect, double d)
		{
			double half_height = 0.5;
			double half_width = aspect * half_height;
			this->d = d;
			origin = lookfrom;
			vec3 w = unit_vector(lookfrom - lookat);
			vec3 u = unit_vector(cross(vup, w));
			vec3 v = cross(w, u);
			film_lower_left_corner = origin - half_width * u - half_height * v + d * w;
			pinhole = origin;
			horizontal = 2 * half_width * u;
			vertical = 2 * half_height * v;
		}

		ray get_ray(double u, double v)
		{
			return ray(origin, origin-(film_lower_left_corner + u*horizontal + v*vertical));
		}

		vec3 origin;
		vec3 film_lower_left_corner;
		vec3 pinhole;
		vec3 horizontal;
		vec3 vertical;
		double d;
};


class LensCamera {
	public:
		LensCamera() {}
		void set_Camera(vec3 lookfrom, vec3 lookat, vec3 vup, double aspect, double d, double focal_length, double aperture)
		{
			double half_height = 0.5;
			double half_width = aspect * half_height;
			this->d = d;
			this->focal_length = focal_length;
			this->aperture = aperture;
			this->a = 1.0/(1.0/focal_length - 1.0/d);
			origin = lookfrom;
			vec3 w = unit_vector(lookfrom - lookat);
			vec3 u = unit_vector(cross(vup, w));
			vec3 v = cross(w, u);
			film_lower_left_corner = origin - half_width * u - half_height * v + d * w;
			pinhole = origin;
			horizontal = 2 * half_width * u;
			vertical = 2 * half_height * v;
		}

		vec3 get_random_on_lens(void)
		{
			vec3 p = vec3(0, 0, 0);
			//return p;
			do {
				p[0] = 2*drand48()-1.0;
				p[1] = 2*drand48()-1.0;
			} while(p.length() > 1.0);
			return p*(aperture/2.0);
		}

		ray get_ray(double u, double v)
		{
			//vec3 target = (-a/d * (film_lower_left_corner + u*horizontal + v*vertical - origin));
			vec3 target = ((-a/d) * (film_lower_left_corner + u*horizontal + v*vertical - origin));
			vec3 tmp = get_random_on_lens();
			vec3 random_point_on_lens = tmp[0]*horizontal + tmp[1]*vertical;
			//return ray(random_point_on_disk, target-random_point_on_disk);
			//return ray(origin+random_point_on_lens, target);
			return ray(origin+random_point_on_lens, unit_vector((origin+target)-(origin+random_point_on_lens)));
			//return ray(origin, origin-(film_lower_left_corner + u*horizontal + v*vertical));
		}

		vec3 origin;
		vec3 film_lower_left_corner;
		vec3 pinhole;
		vec3 horizontal;
		vec3 vertical;
		double d;
		double focal_length;
		double aperture;
		double a;
};

#endif
