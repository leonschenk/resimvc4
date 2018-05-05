
#include "videocore/VideoCoreIVProcessor.hpp"
#include "VideoCoreIVDecode.hpp"
#include "VideoCoreIVExecute.hpp"
#include "BreakpointException.hpp"

#include <cassert>
#include <iostream>

class RegisterVideoCoreIVProcessor {
public:
	RegisterVideoCoreIVProcessor() {
		static VideoCoreIVProcessor processor;
		Processor::registerProcessorType(&processor);
	}
};

static RegisterVideoCoreIVProcessor reg;

VideoCoreIVProcessor::VideoCoreIVProcessor() {
}
VideoCoreIVProcessor::VideoCoreIVProcessor(const VideoCoreIVProcessor &other) {
	registers = other.registers;
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
	bool steppingMode = false;
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
				std::cout << "Breakpoint, what to do?:";
				std::cin >> a;
			}
		}
		decode.step();
	}
}

Processor *VideoCoreIVProcessor::clone() const {
	return new VideoCoreIVProcessor(*this);
}
