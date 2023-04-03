
project(vainCrypto)
include_directories(include)
add_executable(${PROJECT_NAME} prime.c)

add_subdirectory(lib)
target_link_libraries(${PROJECT_NAME} PUBLIC mini-gmp)
