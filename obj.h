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
		std::vector<vec3> v;
		std::vector<vec3> vt;
		std::vector<vec3> vn;
		std::vector<std::vector<std::array<boost::optional<size_t>, 3> > > f;

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

	private:
		bool LoadObject(void);
		void CalcIndices(void);

};


#endif
