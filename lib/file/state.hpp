#ifndef DESCENT_LIB_FILE_STATE_HPP
#define DESCENT_LIB_FILE_STATE_HPP

#include <descent/thread/atomic.hpp>
#include <descent/thread/mutex.hpp>
#include <descent/type/string.hpp>
#include "file.hpp"

namespace descent {

constexpr u64 MAX_PROGRAM_NAME_SIZE = 256;

inline thread::Mutex mutex;
inline thread::atomic::Flag initialized(false);
inline const Directory INVALID_DIRECTORY;
inline char program_buffer[MAX_PROGRAM_NAME_SIZE];
inline string program_name = {};

}

#endif
