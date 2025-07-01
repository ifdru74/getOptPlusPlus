# CMakeList.txt : CMake project for getOptPlusPlus, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("getOptPlusPlus")

# Add source to this project's executable.
include_directories(.)
add_library (getOptPlusPlus STATIC "appConfig.cpp" "appConfig.h" "appArguments.h")
add_executable (getOptPlusPlus_test "getOptPlusPlus_test.cpp" "getOptPlusPlus.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET getOptPlusPlus PROPERTY CXX_STANDARD 17)
  set_property(TARGET getOptPlusPlus_test PROPERTY CXX_STANDARD 17)
endif()
target_link_libraries(getOptPlusPlus_test LINK_PUBLIC getOptPlusPlus)
# test
enable_testing()
add_test(getOptTest getOptPlusPlus_test)
# install section
file(GLOB INT_FILES "appConfig.h" "appArguments.h")
include(GNUInstallDirs)
install(FILES ${INT_FILES} DESTINATION include/getOptPlusPlus)
install(TARGETS getOptPlusPlus LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})