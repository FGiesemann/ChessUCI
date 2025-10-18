from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout, CMakeDeps


class ChessUCIConan(ConanFile):
    name = "chessuci"
    version = "0.2.0"
    package_type = "library"

    license = "MIT License"
    author = "Florian Giesemann <florian.giesemann@gmail.com>"
    url = "https://github.com/FGiesemann/ChessUCI.git"
    homepage = "https://github.com/FGiesemann/ChessUCI"

    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    exports_sources = (
        "CMakeLists.txt",
        "cmake/*",
        "src/*",
        "include/*",
        "test/*",
        "LICENSE",
    )

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.15]")

    def requirements(self):
        self.requires("chesscore/1.0.0", transitive_headers=True)
        self.test_requires("catch2/3.7.1")

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["ChessUCI"]
        self.cpp_info.set_property("cmake_target_name", "ChessUCI::ChessUCI")
