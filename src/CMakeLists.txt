add_executable(build build.cpp)
add_executable(web_server web_server.cpp ../external/mongoose/mongoose.c)
add_executable(output_ds2i_format output_ds2i_format.cpp)
add_executable(statistics statistics.cpp)
