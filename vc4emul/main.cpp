
#include "Processor.hpp"
#include "Device.hpp"
#include "Log.hpp"
#include "Memory.hpp"

#include <iostream>
#include <QCoreApplication>
#include <QFile>
#include <cassert>

static Device *getDevice(const char *name, bool verbose) {
	auto devices = Device::getDeviceTypes();
	assert(devices.size() > 0);
	for (unsigned int i = 0; i < devices.size(); i++) {
		if (verbose)
			std::cout << "Device: " << devices[i]->getName()
				  << std::endl;
		if (std::string(devices[i]->getName()) == name) {
			return devices[i]->clone();
		}
	}
	return NULL;
}

static bool createSimulator(Processor **processor, QList<Device*> *devices,
			    bool verbose) {
	// Create the processor
	auto processors = Processor::getProcessorTypes();
	assert(processors.size() > 0);
	*processor = NULL;
	for (unsigned int i = 0; i < processors.size(); i++) {
		if (processors[i]->getName() == "VideoCore IV") {
			*processor = processors[i]->clone();
			break;
		}
	}
	if (*processor == NULL) {
		std::cerr << "VideoCore IV processor not available!" << std::endl;
		return false;
	}
	// Create the devices
	Device *dram = getDevice("BCM2835", verbose);
	if (dram == NULL) {
		std::cerr << "DRAM device not available!" << std::endl;
		return false;
	}
	devices->append(dram);
	return true;
}

int main(int argc, char **argv) {
	QCoreApplication app(argc, argv);
	int i = 1, first_pos_arg = 1, remaining_args;
	bool logging = false;
	bool verbose = false;
	
	if (argc <= 1)
		goto usage;
	
	while (true)
	  {
	    if (strcmp (argv[i], "-log") == 0)
	      logging = true;
	    else if (strcmp (argv[i], "-verbose") == 0)
	      verbose = true;
	    else
	      break;
	    
	    i++;
	  }

	first_pos_arg = i;
	remaining_args = argc - first_pos_arg;

	// Parse the input parameters
	if (remaining_args != 3 && remaining_args != 4) {
		usage:
		std::cout << "Invalid number of parameters.\n"
				"Usage: " << argv[0]
				<< " [-log] <memory-image> <image-address> <start-address> [<bootrom>]"
				<< std::endl;
		return -1;
	}
	QString imageFileName = argv[first_pos_arg];
	QString bootromFileName = "";
	if (remaining_args == 4) {
		bootromFileName = argv[first_pos_arg + 3];
	}
	bool ok;
	uintptr_t imageAddress
	  = QString(argv[first_pos_arg + 1]).toULong(&ok, 0);
	if (!ok) {
		std::cerr << "Could not parse the image base address." << std::endl;
		return -1;
	}
	uintptr_t entryAddress
	  = QString(argv[first_pos_arg + 2]).toULong(&ok, 0);
	if (!ok) {
		std::cerr << "Could not parse the program entry address." << std::endl;
		return -1;
	}
	// Read the image file
	QFile imageFile(imageFileName);
	if (!imageFile.open(QFile::ReadOnly)) {
		std::cerr << "Could not read the memory image." << std::endl;
		return -1;
	}
	QByteArray image = imageFile.readAll();
	QByteArray bootrom;
	if (bootromFileName != "") {
		QFile bootromFile(bootromFileName);
		if (!bootromFile.open(QFile::ReadOnly)) {
			std::cerr << "Could not read the bootrom image." << std::endl;
			return -1;
		}
		bootrom = bootromFile.readAll();
	}
	// Create the simulator
	Processor *processor;
	QList<Device*> devices;
	if (!createSimulator(&processor, &devices, verbose)) {
		return -1;
	}
	/*// Create a log file
	QFile logFile("vc4emul.log");
	if (!logFile.open(QFile::WriteOnly | QFile::Text)) {
		std::cerr << "Could not create a log file." << std::endl;
		return -1;
	}*/
	// Initialize the simulator
	Memory memory;
	Log log;
	if (logging)
		log.tofile("vc4emul.log");
	log.info("", "Initializing the simulator...");
	processor->initialize(&log, &memory);
	foreach (auto device, devices) {
		device->initialize(&log, &memory);
	}
	processor->setRegister("pc", entryAddress);
	log.info("", "Loading the memory image...");
	for (int offset = 0; offset < image.length(); offset++) {
		memory.writeByte(imageAddress + offset, image.at(offset));
	}
	if (bootromFileName != "") {
		log.info("", "Loading the bootrom image...");
		for (int offset = 0; offset < bootrom.length(); offset++) {
			memory.writeByte(0x60000000 + offset, bootrom.at(offset));
		}
	}
	// Start the simulation
	log.info("", "Starting the simulation...");
	try {
		while (true) {
			processor->run(1000);
		}
	} catch (const std::exception &e) {
		std::cerr << "Exception: " << e.what() << std::endl;
		const std::vector<std::string> &registers = processor->getRegisterList();
		for (auto it = registers.begin(); it != registers.end(); it++) {
			std::cerr << *it << ": " << std::hex << processor->getRegister(*it) << std::endl;
		}
		return 1;
	}
	return 0;
}
