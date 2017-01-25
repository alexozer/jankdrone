#pragma once

// External declaration so STL doesn't bork
extern "C" {
   int _getpid();
   int _kill(int, int);
}
