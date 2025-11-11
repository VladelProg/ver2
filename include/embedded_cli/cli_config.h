#ifndef CLI_CONFIG_H
#define CLI_CONFIG_H

// Конфигурация по умолчанию, можно переопределить через флаги компилятора
#ifndef CLI_INPUT_BUFFER_SIZE
#define CLI_INPUT_BUFFER_SIZE 256
#endif

#ifndef CLI_HISTORY_SIZE
#define CLI_HISTORY_SIZE 10
#endif

#ifndef CLI_MAX_ARGS
#define CLI_MAX_ARGS 8
#endif

#ifndef CLI_PROMPT
#define CLI_PROMPT "cli> "
#endif

// Платформозависимые настройки
#ifdef LINUX
    #define CLI_NEWLINE "\n"
    #define CLI_CLEAR_SCREEN "clear"
#elif defined(STM32)
    #define CLI_NEWLINE "\r\n"
    #define CLI_CLEAR_SCREEN "\033[2J\033[H"
#else
    #define CLI_NEWLINE "\n"
    #define CLI_CLEAR_SCREEN ""
#endif

#endif // CLI_CONFIG_H
