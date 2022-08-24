cmake_minimum_required(VERSION 3.0.0)
project(GTesting VERSION 0.1.0)

# include(CTest)
# enable_testing()

add_executable(GTesting testing.cpp)
target_link_libraries(GTesting gtest gtest_main)

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
