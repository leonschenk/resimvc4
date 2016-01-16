
#include "Device.hpp"

#include <iostream>

static std::vector<const Device*> deviceTypes;

void Device::registerDeviceType(const Device *device, bool verbose) {
	if (verbose)
		std::cout << "Registered " << device->getName() << std::endl;
	deviceTypes.push_back(device);
}
const std::vector<const Device*> &Device::getDeviceTypes() {
	return deviceTypes;
}
