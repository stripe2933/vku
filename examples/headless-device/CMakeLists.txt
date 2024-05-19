add_executable(vku-examples-headless-device main.cpp)
target_compile_features(vku-examples-headless-device PRIVATE cxx_std_23)
target_link_libraries(vku-examples-headless-device PRIVATE
    vku-examples-extlibs
    vku
)