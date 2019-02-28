#include <sstream>
#include <algorithm>
#include "mtl.h"


bool MtlLoader::Load(const char *filename)
{
	file.open(filename);

	std::string line;
	std::streampos oldpos = file.tellg();

	while (getline(file, line)) {
		if (std::all_of(line.begin(), line.end(), isspace)) {
		} else if (line.find("#") == 0) {
		} else if (line.find("newmtl") == 0) {
			file.seekg(oldpos);
			LoadMtl();
		} else {
			std::cout << "Error: " << line << std::endl;
			return false;
		}
		oldpos = file.tellg();
	}

	return true;
}



bool MtlLoader::LoadMtl(void)
{
	std::streampos oldpos = file.tellg();
	std::string line;

	Mtl *mtl = new Mtl();

	getline(file, line);
	std::istringstream iss(line);
	std::string s;
	iss >> s;
	iss >> s;

	mtls[s] = mtl;


	oldpos = file.tellg();
	while (getline(file, line)) {
		if (std::all_of(line.begin(), line.end(), isspace)) {
		} else if (line.find("#") == 0) {
		} else {
			std::istringstream iss(line);
			std::string s;
			iss >> s;
			if (s == "newmtl") {
				file.seekg(oldpos);
				return true;
			} else if (s == "Ns") {
				iss >> s;
				mtl->Ns = atof(s.c_str());
			} else if (s == "Ka") {
				vec3 Ka;
				for (size_t i = 0; i < 3; i++) {
					iss >> s;
					Ka.e[i] = atof(s.c_str());
				}
				mtl->Ka = Ka;
			} else if (s == "Kd") {
				vec3 Kd;
				for (size_t i = 0; i < 3; i++) {
					iss >> s;
					Kd.e[i] = atof(s.c_str());
				}
				mtl->Kd = Kd;
			} else if (s == "Ks") {
				vec3 Ks;
				for (size_t i = 0; i < 3; i++) {
					iss >> s;
					Ks.e[i] = atof(s.c_str());
				}
				mtl->Ks = Ks;
			} else if (s == "Ke") {
				vec3 Ke;
				for (size_t i = 0; i < 3; i++) {
					iss >> s;
					Ke.e[i] = atof(s.c_str());
				}
				mtl->Ke = Ke;
			} else if (s == "Ni") {
				iss >> s;
				mtl->Ni = atof(s.c_str());
			} else if (s == "d") {
				iss >> s;
				mtl->d = atof(s.c_str());
			} else if (s == "illum") {
				iss >> s;
				mtl->illum = atoi(s.c_str());
			} else {
				std::cout << "Error: " << line << std::endl;
				oldpos = file.tellg();
				return false;
			}
		}

		oldpos = file.tellg();
	}

	file.seekg(oldpos);

	return true;
}
