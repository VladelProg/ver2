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

#ifndef CLI_MAX_COMPLETIONS
#define CLI_MAX_COMPLETIONS 10
#endif

// Платформозависимые настройки
#ifdef LINUX
    #define CLI_NEWLINE "\r\n"
    #define CLI_CLEAR_SCREEN "clear"
    #define CLI_BELL "\a"
    #define CLI_CLEAR_LINE "\033[2K\r"
#elif defined(STM32)
    #define CLI_NEWLINE "\r\n"
    #define CLI_CLEAR_SCREEN "\033[2J\033[H"
    #define CLI_BELL "\a"
    #define CLI_CLEAR_LINE "\033[2K\r"
#else
    #define CLI_NEWLINE "\n"
    #define CLI_CLEAR_SCREEN ""
    #define CLI_BELL ""
    #define CLI_CLEAR_LINE ""
#endif

#define ESCAPE_UP "\033[A"
#define ESCAPE_DOWN "\033[B"
#define ESCAPE_RIGHT "\033[C"
#define ESCAPE_LEFT "\033[D"


#endif // CLI_CONFIG_H
