from conan import ConanFile
from conan.tools.files import copy

class FLXML(ConanFile):
    name = "flxml"
    version = "2.1.0"
    exports_sources = "include/*"
    no_copy_source = True

    def package(self):
        copy(self, "include/*.hpp", self.source_folder, self.package_folder)

    def package_info(self):
        self.cpp_info.includedirs = ['include']
        self.cpp_info.libdirs = []
        self.cpp_info.bindirs = []
