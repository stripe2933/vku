add_executable(vku-examples-01-headless-surface main.cpp)
target_compile_features(vku-examples-01-headless-surface PRIVATE cxx_std_23)
target_link_libraries(vku-examples-01-headless-surface PRIVATE
    vku-examples-extlibs
    vku
)