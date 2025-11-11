set(CMAKE_SYSTEM_NAME Linux)

# Опционально: переопределение конфигурации
add_compile_definitions(
    CLI_INPUT_BUFFER_SIZE=128
    CLI_HISTORY_SIZE=5
)
