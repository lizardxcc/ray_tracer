#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "obj.h"

bool obj::Load(const char *filename)
{
	file.open(filename);

	std::string line;
	std::streampos oldpos = file.tellg();

	while (getline(file, line)) {
		if (std::all_of(line.begin(), line.end(), isspace)) {
		} else if (line.find("#") == 0) {
		} else if (line.find("mtllib") == 0) {
			std::istringstream iss(line);
			std::string s;
			iss >> s;
			iss >> s;
			mtl_file.push_back(s);
		} else if (line.find("o") == 0) {
			file.seekg(oldpos);
			LoadObject();
		} else {
			std::cout << "Error: " << line << std::endl;
			return false;
		}
		oldpos = file.tellg();
	}

	file.close();

	CalcIndices();

	return true;
}


void obj::Clear(void)
{
	mtl_file.clear();
	for (auto & o : objects) {
		delete o;
	}
	objects.clear();
}


bool obj::LoadObject(void)
{
	std::streampos oldpos = file.tellg();
	std::string line;

	objobject *obj = new objobject();
	objects.push_back(obj);

	getline(file, line);
	std::istringstream iss(line);
	std::string s;
	iss >> s;
	iss >> s;

	obj->name = s;
	//std::cout << s << std::endl;


	size_t property_size = 0;
	oldpos = file.tellg();
	while (getline(file, line)) {
		if (std::all_of(line.begin(), line.end(), isspace)) {
		} else if (line.find("#") == 0) {
		} else {
			std::istringstream iss(line);
			std::string s;
			iss >> s;
			if (s == "o") {
				file.seekg(oldpos);
				return true;
			} else if (s == "v") {
				vec3 v;
				iss >> s;
				v.e[0] = atof(s.c_str());
				iss >> s;
				v.e[1] = atof(s.c_str());
				iss >> s;
				v.e[2] = atof(s.c_str());
				obj->v.push_back(v);
			} else if (s == "vt") {
			} else if (s == "vn") {
				vec3 vn;
				iss >> s;
				vn.e[0] = atof(s.c_str());
				iss >> s;
				vn.e[1] = atof(s.c_str());
				iss >> s;
				vn.e[2] = atof(s.c_str());
				obj->vn.push_back(vn);
			} else if (s == "f") {
				std::vector<std::array<boost::optional<size_t>, 3> > face;
				while (!iss.eof()) {
					iss >> s;
					std::istringstream v(s);
					std::string vs;
					std::array<boost::optional<size_t>, 3> va = {boost::none, boost::none, boost::none};

					getline(v, vs, '/');
					if (!std::all_of(vs.begin(), vs.end(), isspace)) {
						//std::cout << " AAA :" << vs << ";" << std::endl;
						va[0] = atoi(vs.c_str());
					}
					getline(v, vs, '/');
					if (!std::all_of(vs.begin(), vs.end(), isspace)) {
						//std::cout << " AAA :" << vs << ";" << std::endl;
						va[1] = atoi(vs.c_str());
					}
					getline(v, vs, '/');
					if (!std::all_of(vs.begin(), vs.end(), isspace)) {
						//std::cout << " AAA :" << vs << ";" << std::endl;
						va[2] = atoi(vs.c_str());
					}
					face.push_back(va);
				}
				obj->f.push_back(face);
			} else if (s == "usemtl") {
				iss >> s;
				obj->Material_name = s;
			} else if (s == "s") {
				iss >> s;
				if (s == "off") {
					obj->s = false;
				} else {
					obj->s = true;
				}
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


void objobject::PrintDebug(void)
{
	std::cout << "Obj: " << name << std::endl;
	//for (const auto& ve : v) {
	//	std::cout << ve.x() << ", " << ve.y() << ", " << ve.z() << std::endl;
	//}
	//for (const auto& vne : vn) {
	//	std::cout << vne.x() << ", " << vne.y() << ", " << vne.z() << std::endl;
	//}
	for (const auto& a : f) {
		for (const auto& b : a) {
			if (b[0])
				std::cout << *b[0];
			std::cout << ",";
			if (b[1])
				std::cout << *b[1];
			std::cout << ",";
			if (b[2])
				std::cout << *b[2];

			std::cout << " ";
		}
		std::cout << std::endl;
	}
}

void obj::PrintDebug(void)
{
	for (const auto& o : objects) {
		o->PrintDebug();
	}
}


void obj::CalcIndices(void)
{
	if (objects.size() == 0)
		return;
	size_t v_index = 1;
	size_t vt_index = 1;
	size_t vn_index = 1;
	for (auto& o : objects) {
		for (auto& a : o->f) {
			for (auto& b : a) {
				if (b[0]) {
					*b[0] -= v_index;
				}
				if (b[1]) {
					*b[1] -= vt_index;
				}
				if (b[2]) {
					*b[2] -= vn_index;
				}
			}
		}
		v_index += o->v.size();
		vt_index += o->vt.size();
		vn_index += o->vn.size();
	}
}
