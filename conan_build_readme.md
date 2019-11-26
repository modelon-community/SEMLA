Following commands can be used to build SEMLA using conan OpenSSL package.
This setup is considered experimental.

# install conan (either pip or pip3)
pip install conan

# If using internal conan server add it and authenticate, for instance:
# conan remote add modelon-test https://artifactory01.modelon.com/api/conan/conan-test
# conan user -r modelon-test -p PASSWORD USERNAME

# Configure build directory and build dependencies if needed configuration is missing in the conan repo
# This will bring OpenSSL and zlib into lical conan cache making them available for linking.
# conan install /Projects/SEMLA/SEMLA -if /tmp/build --build missing -r modelon-test
conan install /Projects/SEMLA/SEMLA -if /tmp/build --build missing 

# build SEMLA (this can be also done with explicit cmake configure + build)
conan build /Projects/SEMLA/SEMLA -if /tmp/build -bf /tmp/build