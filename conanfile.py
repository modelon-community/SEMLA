from conans import ConanFile, CMake


class SemlaConan(ConanFile):
    name = "SEMLA"
    version = "0.1"
    license = "BSD"
    author = "Modelon AB"
    url = "https://github.com/modelon/SEMLA"
    description = "Safe Encryption of Modelica Libs and Artifacts library"
    topics = ("Modelica", "Encryption", "Licensing")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "openssl_version":["1.0.2q", "1.1.1a", "1.1.1d"]}
    default_options = { "shared":False, "openssl_version":"1.1.1a"}
    generators = "cmake"
    exports_sources = "src/*", "ThirdParty/*", "!ThirdParty/openssl*"

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder="src")
        cmake.build()
        # Explicit way:
        # self.run('cmake %s/hello %s'
        #          % (self.source_folder, cmake.command_line))
        # self.run("cmake --build . %s" % cmake.build_config)

    def package(self):
        self.copy("*.h", dst="include", src="src")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)
        self.copy("*", src="bin", dst="bin", keep_path=False)

    def requirements(self):   
        if self.options.openssl_version =="1.1.1a":
            self.requires("OpenSSL/1.1.1a@conan/stable")
        elif self.options.openssl_version =="1.1.1d":
            self.requires("openssl/1.1.1d")
        else:
            self.requires("OpenSSL/1.0.2q@conan/stable")

    def package_info(self):
        self.cpp_info.includedirs = ['include']  # Ordered list of include paths
        self.cpp_info.libs = []  # The libs to link against
        self.cpp_info.libdirs = ['lib']  # Directories where libraries can be found
        self.cpp_info.resdirs = ['res']  # Directories where resources, data, etc can be found
        self.cpp_info.bindirs = ['bin']  # Directories where executables and shared libs can be found
        self.cpp_info.srcdirs = ['src']  # Directories where sources can be found (debugging, reusing sources)
        """
        self.cpp_info.defines = []  # preprocessor definitions
        self.cpp_info.cflags = []  # pure C flags
        self.cpp_info.cppflags = []  # C++ compilation flags
        self.cpp_info.sharedlinkflags = []  # linker flags
        self.cpp_info.exelinkflags = []  # linker flags        self.cpp_info.libs = ["hello"]
        """
