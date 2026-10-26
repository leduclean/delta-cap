# xSPI2 address of the AI model: flash-model writes it there, ai.c installs it
# from there (AI_MODEL_ADDR). Included by the top and FSBL CMakeLists.
set(AI_MODEL_ADDR 0x70380000)
