#ifndef PLY_H
#define PLY_H

#include <fstream>
#include <vector>
#include "vec3.h"

class element {
	public:
		std::string name;
		size_t num_of_elements;

		size_t num_of_property;

		void PrintInfo(void);

};

class ply {
	public:
		~ply();
		void Load(const char *filename);
		element *LoadElement(void);
		void PrintInfo(void);
		void DebugInfo(void);


		std::ifstream file;

		std::vector<element *> elements;

		std::vector<std::vector<vec3> > vertices;
		std::vector<std::vector<int> > faces;

};


#endif
