from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, CMakeDeps

class FLXML(ConanFile):
    name = "flxml-test"
    settings = "os", "compiler", "build_type", "arch"
    test_type = "explicit"

    def configure(self):
        self.options["sentry-native"].backend = "inproc"

    def requirements(self):
        self.requires(f'flxml/{self.version}')
        self.requires("sentry-native/0.7.11")
        self.requires("gtest/1.12.1")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.user_presets_path = False
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        if not self.conf.get("tools.build:skip_test"):
            self.run("./rapidxml-test", env="conanrun")
