#include <iostream>
#include "vec3.h"


class ray {
public:
	ray()
	{
	}

	ray(const vec3& a, const vec3& b)
	{
		A = a;
		B = b;
	}

	vec3 origin() const { return A; }
	vec3 direction() const { return B; }
	vec3 point_at_parameter(float t) const {
		return A + t * B;
	}

	vec3 A;
	vec3 B;
};



bool hit_sphere(const vec3& center, float radius, const ray& r)
{
	float a = pow(r.B.length(), 2.0);
	float b_prime = dot(r.A - center, r.B);
	float c = pow((r.A - center).length(), 2.0) - pow(radius, 2.0);
	float discriminant_prime = pow(b_prime, 2.0) - a * c;
	if (discriminant_prime >= 0.0) {
		return true;
	}
	return false;
}


vec3 color(const ray& r)
{
	if (hit_sphere(vec3(0, 0, -1), 0.5, r))
		return vec3(1, 0, 0);

	vec3 unit_direction = unit_vector(r.direction());
	float t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
}

int main(void)
{
	int nx = 200;
	int ny = 100;

	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	vec3 lower_left_corner(-2.0, -1.0, -1.0);
	vec3 horizontal(4.0, 0.0, 0.0);
	vec3 vertical(0.0, 2.0, 0.0);
	vec3 origin(0.0, 0.0, 0.0);


	for (int j = ny-1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {
			float u = float(i) / float(nx);
			float v = float(j) / float(ny);
			ray r(origin, lower_left_corner + u*horizontal + v*vertical);
			vec3 col = color(r);

			//vec3 col(float(i) / float(nx), float(j) / float(ny), 0.2);
			int ir = int(255.99*col[0]);
			int ig = int(255.99*col[1]);
			int ib = int(255.99*col[2]);

			std::cout << ir << " " << ig << " " << ib << "\n";
		}
	}
	return 0;
}
