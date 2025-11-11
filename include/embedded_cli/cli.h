#ifndef EMBEDDED_CLI_H
#define EMBEDDED_CLI_H

#include <stdint.h>
#include <stdbool.h>
#include "cli_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// Типы данных
typedef struct CliCommand CliCommand;
typedef struct Cli Cli;

// Callback тип для команд
typedef void (*CliCommandHandler)(Cli *cli, int argc, char **argv);

// Структура команды
struct CliCommand {
    const char *name;
    const char *help;
    CliCommandHandler handler;
    CliCommand *next;
};

// Основная структура CLI
struct Cli {
    char input_buffer[CLI_INPUT_BUFFER_SIZE];
    char history[CLI_HISTORY_SIZE][CLI_INPUT_BUFFER_SIZE];
    uint16_t input_pos;
    uint16_t history_pos;
    uint16_t history_count;
    bool echo_enabled;
    CliCommand *commands;
    
    // Платформозависимые функции
    void (*write_char)(char c);
    char (*read_char)(void);
    bool (*data_available)(void);
};

// Основные функции API
void cli_init(Cli *cli);
void cli_process(Cli *cli);
void cli_add_command(Cli *cli, CliCommand *command);
void cli_remove_command(Cli *cli, const char *name);
void cli_print_help(Cli *cli);
void cli_write_char(Cli *cli, char c);
void cli_print(Cli *cli, const char *str);
void cli_println(Cli *cli, const char *str);

// Вспомогательные функции
void cli_clear_screen(Cli *cli);
void cli_show_prompt(Cli *cli);

#ifdef __cplusplus
}
#endif

#endif // EMBEDDED_CLI_H
