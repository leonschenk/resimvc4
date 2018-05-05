
#include "Processor.hpp"

static std::vector<const Processor*> processorTypes;

void Processor::registerProcessorType(const Processor *processor) {
	processorTypes.push_back(processor);
}
const std::vector<const Processor*> &Processor::getProcessorTypes() {
	return processorTypes;
}

void Processor::setBreakpoint(const uint64_t address) {
	this->breakpoints.push_back(address);
}
