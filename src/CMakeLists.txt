set(SOURCE_FILES compiler.cpp lexer.cpp parser.cpp)

FIND_PACKAGE(Boost 1.40 COMPONENTS program_options REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

add_executable(compiler ${SOURCE_FILES})
target_link_libraries(compiler ${Boost_LIBRARIES})
