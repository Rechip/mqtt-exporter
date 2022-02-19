import os
from conans import ConanFile, CMake, tools

class LoggerConan(ConanFile):
    name = "logger"
    version = "0.1.0"
    license = "MIT"
    url = "https://github.com/Rechip/logger"
    homepage = "https://github/Rechip/logger"
    description = "Modular logger"
    topics = ("logger", "cpp")
    settings = "os", "compiler", "arch", "build_type"
  
    generators = "cmake"
    exports_sources = "include/*", "src/*", "CMakeLists.txt", "tests/*"
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def validate(self):
        tools.check_min_cppstd(self, "20")

    def requirements(self):
        self.requires("simple-yaml/0.2.2")

    def package(self):
        self.copy("*.hpp")
        self.copy("*.a", dst="lib", keep_path=False)
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.so", dst="bin", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["logger"]
