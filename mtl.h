#ifndef MTL_H
#define MTL_H

#include <fstream>
#include <unordered_map>
#include "vec3.h"

class Mtl {
	public:
		double Ns;
		vec3 Ka;
		vec3 Kd;
		vec3 Ks;
		vec3 Ke;
		double Ni;
		double d;
		size_t illum;
};

class MtlLoader {
	public:
		bool Load(const char *filename);
		bool LoadMtl(void);
		std::ifstream file;
		std::unordered_map<std::string, Mtl *> mtls;
};

#endif
