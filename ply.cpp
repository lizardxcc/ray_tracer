#include "ply.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>


void element::PrintInfo(void)
{
	std::cout << name << " " << num_of_elements << std::endl;
	std::cout << num_of_property << std::endl;
}

void ply::Load(const char *filename)
{
	file.open(filename);

	std::string line;
	std::streampos oldpos = file.tellg();

	// read header
	while (getline(file, line)) {
		//std::cout << line << std::endl;
		if (line.find("element") == 0) {
			file.seekg(oldpos);
			element *new_elem = LoadElement();
			if (new_elem != NULL) {
				//new_elem->PrintInfo();
				elements.push_back(new_elem);
			}
		}
		if (line.find("end_header") == 0) {
			break;
		}
		oldpos = file.tellg();
	}

	// read vertices
	vertices.resize(elements[0]->num_of_elements);
	for (size_t i = 0; i < elements[0]->num_of_elements; i++) {
		vertices[i].resize(2);
	}
	for (size_t i = 0; i < elements[0]->num_of_elements; i++) {
		getline(file, line);
		std::istringstream iss(line);
		std::string s;

		vec3 vertex, normal;
		for (size_t j = 0; j < 3; j++) {
			iss >> s;
			vertex.e[j] = atof(s.c_str());
		}
		for (size_t j = 0; j < 3; j++) {
			iss >> s;
			normal.e[j] = atof(s.c_str());
		}
		vertices[i][0] = vertex;
		vertices[i][1] = normal;

	}


	// read faces
	faces.resize(elements[1]->num_of_elements);
	for (size_t i = 0; i < elements[1]->num_of_elements; i++) {
		getline(file, line);
		std::istringstream iss(line);
		std::string s;
		iss >> s;
		size_t size = (size_t)atoi(s.c_str());
		faces[i].resize(size);

		for (size_t j = 0; j < size; j++) {
			iss >> s;
			faces[i][j] = atoi(s.c_str());
		}


		//std::cout << line << std::endl;
	}

}

void ply::PrintInfo(void)
{
	for (size_t i = 0; i < vertices.size(); i++) {
		std::cout << vertices[i][0].x() << " " <<
		vertices[i][0].y() << " " <<
		vertices[i][0].z() << " " <<
		vertices[i][1].x() << " " <<
		vertices[i][1].y() << " " <<
		vertices[i][1].z() << " " << std::endl;
		//for (size_t j = 0; j < vertices[i].size(); j++) {
		//	std::cout << vertices[i][j] << " ";
		//}
		//std::cout << std::endl;
	}

	for (size_t i = 0; i < faces.size(); i++) {
		for (size_t j = 0; j < faces[i].size(); j++) {
			std::cout << faces[i][j] << " ";
		}
		std::cout << std::endl;
	}
}


void ply::DebugInfo(void)
{
	for (size_t i = 0; i < faces.size(); i++) {
		for (size_t j = 0; j < faces[i].size(); j++) {
			for (size_t k = 0; k < 3; k++) {
				std::cout << vertices[(size_t)faces[i][j]][1].e[k] << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
}


element *ply::LoadElement(void)
{
	std::streampos oldpos = file.tellg();
	std::string line;

	element *elm = new element();

	getline(file, line);
	std::istringstream iss(line);
	std::string s;
	iss >> s;
	iss >> s;
	elm->name = s;
	iss >> s;
	elm->num_of_elements = (size_t)atoi(s.c_str());


	size_t property_size = 0;
	oldpos = file.tellg();
	while (getline(file, line)) {
		std::istringstream iss(line);
		std::string s;
		iss >> s;
		if (s != "property") {
			break;
		}
		iss >> s;
		iss >> s;
		property_size++;
		oldpos = file.tellg();
	}
	elm->num_of_property = property_size;

	file.seekg(oldpos);

	return elm;

}


ply::~ply()
{
	if (file.is_open())
		file.close();
}
