/*
Copyright (C) 2012, Mathias Gottschlag

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef RESIM_MmioHost_HPP_INCLUDED
#define RESIM_MmioHost_HPP_INCLUDED

#include "Log.hpp"

#include <string>
#include <cstdint>

class MmioHost {
public:
	MmioHost(Log *);
	~MmioHost();

	void executeWrite(uintptr_t address,
	                  uint64_t value,
	                  unsigned int size);
	uint64_t executeRead(uintptr_t address,
	                     unsigned int size);

	static void executeWriteStatic(void *thisPtr,
	                               uintptr_t address,
	                               uint64_t value,
                                   unsigned int size) {
		((MmioHost*)thisPtr)->executeWrite(address, value, size);
	}
	static uint64_t executeReadStatic(void *thisPtr,
	                                  uintptr_t address,
	                                  unsigned int size) {
		return ((MmioHost*)thisPtr)->executeRead(address, size);
	}
private:
	Log *log;

	struct {
		uint32_t enables;
		struct {
			uint32_t baud_reg;
		} mini_uart;
	} aux;

	struct {
		uint32_t gpfsel[5];
		uint32_t gppud;
		uint32_t gppudclk0;
	} gpio;
};

#endif
