file(GLOB_RECURSE USER_SRC CONFIGURE_DEPENDS Src/*.c)
add_library(user STATIC ${USER_SRC})

target_include_directories(user PUBLIC Inc)
target_link_libraries(user PUBLIC camera lcd screenl stm32cubemx ai-pp stepper)

# Linux like flags
set_target_properties(user PROPERTIES C_STANDARD 23)
target_compile_options(user PRIVATE
  -Wall -Wextra -Wundef -Wvla -Wshadow
  -Wdouble-promotion
  -Wstrict-prototypes -Wmissing-prototypes
  -Wformat=2 -Wcast-align -Wnull-dereference
  -fno-strict-aliasing
  -Werror)

# External lib are not compiled with our flags
set_target_properties(stm32cubemx camera lcd screenl ai ai-pp stepper PROPERTIES SYSTEM TRUE)
