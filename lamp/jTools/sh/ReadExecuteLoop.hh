// ==================
// ReadExecuteLoop.hh
// ==================

#ifndef READEXECUTELOOP_HH
#define READEXECUTELOOP_HH

// Standard C++
#include <vector>

// POSeven
#include "POSeven/FileDescriptor.hh"


namespace P7 = POSeven;

extern int gLastResult;

int ReadExecuteLoop( P7::FileDescriptor  fd,
                     bool                prompts );

#endif

