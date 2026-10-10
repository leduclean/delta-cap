file(GLOB_RECURSE USER_SRC CONFIGURE_DEPENDS Src/*.c)
add_library(user STATIC ${USER_SRC})

target_include_directories(user PUBLIC Inc)
target_link_libraries(user PUBLIC camera lcd screenl stm32cubemx)
