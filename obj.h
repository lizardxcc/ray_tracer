#ifndef OBJ_H
#define OBJ_H

#include <fstream>
#include <vector>
#include <boost/optional.hpp>
#include <array>
#include "vec3.h"

class objobject {
	public:
		void PrintDebug(void);
		std::vector<std::vector<std::array<boost::optional<size_t>, 3> > > faces;

		std::string name;
		std::string material_name;
		bool s;
};

class obj {
	public:
		bool Load(const char *filename);
		void Clear(void);
		void PrintDebug(void);

		std::ifstream file;
		std::vector<std::string> mtl_file;
		std::vector<objobject *> objects;
		std::vector<dvec3> v;
		std::vector<dvec3> vt;
		std::vector<dvec3> vn;

	private:
		bool LoadObject(void);

};


#endif
