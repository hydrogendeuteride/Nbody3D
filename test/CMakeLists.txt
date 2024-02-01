add_executable(Tests SVOctreeTest.cpp ../src/svotree.cpp)

target_link_libraries(Tests gtest_main)

include(GoogleTest)
gtest_discover_tests(Tests)