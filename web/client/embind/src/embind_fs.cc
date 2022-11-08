#include "emscripten/bind.h"

#include <filesystem>

std::string string(std::filesystem::path const& p) { return p.string(); }

EMSCRIPTEN_BINDINGS(std_filesystem_path) {
    emscripten::class_<std::filesystem::path>("FilesystemPath")
        .constructor<std::string>()
        .function("string", &string);
}

