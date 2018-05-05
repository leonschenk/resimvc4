
#include "videocore/VideoCoreIVProcessor.hpp"
#include "VideoCoreIVDecode.hpp"
#include "VideoCoreIVExecute.hpp"
#include "BreakpointException.hpp"

#include <cassert>
#include <iostream>
#include <string>

using namespace std;

class RegisterVideoCoreIVProcessor {
public:
	RegisterVideoCoreIVProcessor() {
		static VideoCoreIVProcessor processor;
		Processor::registerProcessorType(&processor);
	}
};

static RegisterVideoCoreIVProcessor reg;

VideoCoreIVProcessor::VideoCoreIVProcessor() : steppingMode(false) {
}
VideoCoreIVProcessor::VideoCoreIVProcessor(const VideoCoreIVProcessor &other) {
	registers = other.registers;
	breakpoints = other.breakpoints;
	steppingMode = other.steppingMode;
}

void VideoCoreIVProcessor::setRegister(const std::string &name,
                                       uint64_t value) {
	unsigned int index = registers.getRegisterIndex(name);
	registers.setRegister(index, value);
}
uint64_t VideoCoreIVProcessor::getRegister(const std::string &name) {
	unsigned int index = registers.getRegisterIndex(name);
	return registers.getRegister(index);
}
int VideoCoreIVProcessor::getRegisterSize(const std::string &name) {
	return 4;
}
const std::vector<std::string> &VideoCoreIVProcessor::getRegisterList() {
	return registers.getRegisterNames();
}

void VideoCoreIVProcessor::run(unsigned int steps) {
	VideoCoreIVDecode decode(getMemory(), registers, getLog());
	for (unsigned int i = 0; i < steps; i++) {
		try {
			if(steppingMode) {
				throw BreakpointException();
			}
			for(std::vector<uint64_t>::const_iterator it = breakpoints.begin(); it != breakpoints.end(); it++) {
				if(decode.pc() == *it) {
					throw BreakpointException();
				}
			}
		} catch(const BreakpointException& ex) {
			std::string a("");
			while(a.compare(0, 1, "c") != 0) {
				std::cout << "Breakpoint hit at " << decode.pc() << ", what to do?:" << std::endl;
				std::cout << "'(s)tep', '(r)egisters', '(c)ontinue', '(m)emory 1/2/4', '(b)reakpoint'";
				std::cin >> a;
				if(a.compare(0, 1, "s") == 0) {
					steppingMode = true;
					break;
				} else if(a.compare(0, 1, "r")  == 0) {
					const std::vector<std::string> &registers = this->getRegisterList();
					for (std::vector<std::string>::const_iterator it = registers.begin(); it != registers.end(); it++) {
						std::cerr << *it << ": " << std::hex << this->getRegister(*it) << std::endl;
					}
				} else if(a.compare(0, 1, "c") == 0) {
					steppingMode = false;
				} else if(a.compare(0, 1, "m") == 0) {
					uint32_t address;
					std::cin >> address;
					if(a.compare(1, 1, "1") == 0) {
						std::cout << "Value: " << std::hex << this->memory->readByte(address) << std::endl;
					} else if(a.compare(1, 1, "2") == 0) {
						std::cout << "Value: " << std::hex << this->memory->readHalfWord(address) << std::endl;
					} else if(a.compare(1, 1, "4") == 0) {
						std::cout << "Value: " << std::hex << this->memory->readWord(address) << std::endl;
					}
				} else if(a.compare(0, 1, "b") == 0) {
					uint32_t address;
					std::cin >> address;
					bool found = false;
					for(std::vector<uint64_t>::const_iterator it = breakpoints.begin(); it != breakpoints.end(); it++) {
						if(address == *it) {
							std::cout << "Deactivating breakpoint" << std::endl;
							breakpoints.erase(it);
							found = true;
						}
					}
					if(!found) {
						std::cout << "Activating breakpoint" << std::endl;
						breakpoints.push_back(address);
					}
				}
			}
		}
		decode.step();
	}
}

Processor *VideoCoreIVProcessor::clone() const {
	return new VideoCoreIVProcessor(*this);
}
