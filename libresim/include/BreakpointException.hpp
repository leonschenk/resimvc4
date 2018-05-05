#include <stdexcept>
#include "videocore/VideoCoreIVRegisterFile.hpp"
#include "Memory.hpp"

class BreakpointException : public std::runtime_error
{
public:
	BreakpointException() : std::runtime_error("breakpoint") {
	}

	virtual char const * what() const {
		return "A breakpoint was hit.";
	}
};
