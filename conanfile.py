import os

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout
from conan.tools.build import can_run


class GuanaqoRecipe(ConanFile):
    name = "guanaqo"
    version = "1.0.0-alpha.7"

    # Optional metadata
    license = "LGPLv3"
    author = "Pieter P <pieter.p.dev@outlook.com>"
    url = "https://github.com/tttapa/guanaqo"
    description = "Utilities for scientific software."
    topics = "scientific software"

    # Binary configuration
    package_type = "library"
    settings = "os", "compiler", "build_type", "arch"
    bool_guanaqo_options = {
        "with_quad_precision": False,
        "with_itt": False,
        "with_tracing": False,
    }
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    } | {k: [True, False] for k in bool_guanaqo_options}
    default_options = {
        "shared": False,
        "fPIC": True,
    } | bool_guanaqo_options

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = (
        "CMakeLists.txt",
        "src/*",
        "cmake/*",
        "test/*",
        "LICENSE",
        "README.md",
    )

    generators = ("CMakeDeps",)

    def requirements(self):
        if self.options.with_itt:
            self.requires("ittapi/3.24.4", transitive_headers=True)
        self.test_requires("gtest/1.15.0")
        self.test_requires("eigen/3.4.0")

    def config_options(self):
        if self.settings.get_safe("os") == "Windows":
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        for k in self.bool_guanaqo_options:
            value = getattr(self.options, k, None)
            if value is not None and value.value is not None:
                tc.variables["GUANAQO_" + k.upper()] = bool(value)
        if can_run(self):
            tc.variables["GUANAQO_FORCE_TEST_DISCOVERY"] = True
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "none")
        self.cpp_info.builddirs.append(os.path.join("lib", "cmake", "guanaqo"))
