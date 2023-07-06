#include <iostream>
#include <filesystem>
#include <cassert>

#include "/zzzzzzzzzzz/github/zhscript/gjke4/rust.h"

namespace fs = std::filesystem;

extern "C" {

void relative__(rust_add___ add, void* env, const char* s, const char* s2) {
	add(fs::path(s).lexically_relative(s2).c_str(), 0, env);
}

void normal__(rust_add___ add, void* env, const char* s) {
	add(fs::path(s).lexically_normal().c_str(), 0, env);
}

void canonical__(rust_add___ add, void* env, const char* s) {
	add(fs::canonical(fs::path(s)).c_str(), 0, env);
}

}