#ifndef WRITE_VERSION_HPP
#define WRITE_VERSION_HPP

#include "../ModelStucts.hpp"
#include "../AssetLib.hpp"

namespace AssetLib {

	void saveMeshV2_0(const char* fileName, const Model& mdl, const SaveFlags& rd_only_flags);
	void saveMeshV2_1(const char* fileName, const Model& mdl, const SaveFlags& rd_only_flags);

}

#endif