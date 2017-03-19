#ifndef MASTERFILEEXTENSION_HPP
#define MASTERFILEEXTENSION_HPP

#include "../../IExtension.hpp"
#include "../Assets/MasterFile.hpp"


class MasterFileExtension : public IExtension {

public:
	void saveMasterFile();
	virtual void execute(int nrArgs, void** args);

private:
	MasterFile masterFile;

};


#endif