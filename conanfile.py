import os

from conans import ConanFile, CMake


def get_version():
    with open(os.path.join(os.path.dirname(__file__), 'version'), 'r') as f:
        content = f.read()
        try:
            content = content.decode()
        except AttributeError:
            pass
        return content.strip()


class MinterTxConan(ConanFile):
    name = "mhwallet"
    version = get_version()
    license = "MIT"
    author = "Eduard Maximovich edward.vstock@gmail.com"
    url = "https://github.com/MinterTeam/mhwallet"
    description = "Minter C++ Ledger wallet SDK: connect with ledger minter application using this library"
    topics = ("minter", "minter-ledger", "ledger", "minter-network", "minter-blockchain", "blockchain")
    settings = "os", "compiler", "build_type", "arch"
    options = {"cli": [True, False]}
    default_options = {
        "cli": False,
        "bip39:shared": False,
        "bip39:enableC": False,
        "bip39:enableJNI": False,
        "fmt:shared": False,
        "toolbox:shared": False,
    }
    exports = "version"
    exports_sources = (
        "modules/*",
        "include/*",
        "cfg/*",
        "tests/*",
        "src/*",
        "libs/*",
        "CMakeLists.txt",
        "conanfile.py",
        "LICENSE",
        "README.md",
    )
    generators = "cmake"
    default_user = "minter"
    default_channel = "latest"

    requires = (
        'minter_tx/2.0.3@minter/latest',
        'toolbox/3.2.3@edwardstock/latest',
        'fmt/7.1.3',
    )

    build_requires = (
        # "gtest/1.10.0",
    )

    def source(self):
        if "CONAN_LOCAL" not in os.environ:
            self.run("rm -rf *")
            self.run("git clone --recursive https://github.com/MinterTeam/mhwallet.git .")

    def configure(self):
        if self.settings.compiler == "Visual Studio":
            del self.settings.compiler.runtime

        if self.options.cli:
            self.requires.add('boost/1.76.0')

    def build(self):
        cmake = CMake(self)
        opts = {
            'CMAKE_BUILD_TYPE': self.settings.build_type,
            'ENABLE_SHARED': "Off",
        }

        cmake.configure(defs=opts)
        cmake.build()

    def package(self):
        self.copy("*", dst="include", src="include", keep_path=True)
        dir_types = ['bin', 'lib', 'Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel']
        file_types = ['lib', 'dll', 'dll.a', 'a', 'so', 'exp', 'pdb', 'ilk', 'dylib']

        for dirname in dir_types:
            for ftype in file_types:
                self.copy("*." + ftype, src=dirname, dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ['mhwallet']
        if self.settings.os == "Linux":
            self.cpp_info.system_libs = ['udev', 'usb-1.0']
        elif self.settings.os == "Macos":
            self.cpp_info.frameworks = ['IOKit', 'CoreFoundation']
            self.cpp_info.sharedlinkflags.append("-L/usr/local/lib")
            self.cpp_info.exelinkflags.append("-L/usr/local/lib")
            self.cpp_info.system_libs = ['usb']
        elif self.settings.os == "Windows" and self.settings.compiler == "Visual Studio":
            self.cpp_info.system_libs = ['hid', 'setupapi']
