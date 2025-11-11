#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "embedded_cli/cli.h"
#include "embedded_cli/cli_commands.h"

static void cli_parse_command(Cli *cli, const char *input);
static void cli_execute_command(Cli *cli, int argc, char **argv);
static void cli_add_to_history(Cli *cli, const char *command);

void cli_init(Cli *cli) {
    // Сохраняем платформозависимые функции
    void (*write_char)(char c) = cli->write_char;
    char (*read_char)(void) = cli->read_char;
    bool (*data_available)(void) = cli->data_available;
    
    // Обнуляем структуру
    memset(cli, 0, sizeof(Cli));
    
    // Восстанавливаем функции
    cli->write_char = write_char;
    cli->read_char = read_char;
    cli->data_available = data_available;
    
    cli->echo_enabled = true;
    
    // Регистрируем стандартные команды
    cli_register_default_commands(cli);
    
    // Показываем приветственное сообщение
    cli_show_prompt(cli);
}

// ... остальные функции без изменений ...
void cli_process(Cli *cli) {
    if (!cli->data_available()) {
        return;
    }

    char c = cli->read_char();
    
    switch (c) {
        case '\r':
        case '\n':
            if (cli->input_pos > 0) {
                cli->input_buffer[cli->input_pos] = '\0';
                if (cli->echo_enabled) {
                    cli_println(cli, "");
                }
                cli_add_to_history(cli, cli->input_buffer);
                cli_parse_command(cli, cli->input_buffer);
                cli->input_pos = 0;
                cli_show_prompt(cli);
            }
            break;
            
        case '\b':
        case 127: // Backspace
            if (cli->input_pos > 0) {
                cli->input_pos--;
                if (cli->echo_enabled) {
                    cli_print(cli, "\b \b");
                }
            }
            break;
            
        case '\t': // Tab
            // TODO: автодополнение
            break;
            
        default:
            if (cli->input_pos < (CLI_INPUT_BUFFER_SIZE - 1) && c >= 32 && c <= 126) {
                cli->input_buffer[cli->input_pos++] = c;
                if (cli->echo_enabled) {
                    cli_write_char(cli, c);
                }
            }
            break;
    }
}

static void cli_parse_command(Cli *cli, const char *input) {
    char *argv[CLI_MAX_ARGS];
    int argc = 0;
    char *token;
    char *input_copy = strdup(input);
    
    token = strtok(input_copy, " \t");
    while (token != NULL && argc < CLI_MAX_ARGS) {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
    }
    
    if (argc > 0) {
        cli_execute_command(cli, argc, argv);
    }
    
    free(input_copy);
}

static void cli_execute_command(Cli *cli, int argc, char **argv) {
    CliCommand *cmd = cli->commands;
    
    while (cmd != NULL) {
        if (strcmp(cmd->name, argv[0]) == 0) {
            cmd->handler(cli, argc, argv);
            return;
        }
        cmd = cmd->next;
    }
    
    cli_println(cli, "Command not found. Type 'help' for available commands.");
}

static void cli_add_to_history(Cli *cli, const char *command) {
    if (cli->history_count < CLI_HISTORY_SIZE) {
        strncpy(cli->history[cli->history_count], command, CLI_INPUT_BUFFER_SIZE - 1);
        cli->history[cli->history_count][CLI_INPUT_BUFFER_SIZE - 1] = '\0';
        cli->history_count++;
    } else {
        // Сдвигаем историю
        for (int i = 0; i < CLI_HISTORY_SIZE - 1; i++) {
            strcpy(cli->history[i], cli->history[i + 1]);
        }
        strncpy(cli->history[CLI_HISTORY_SIZE - 1], command, CLI_INPUT_BUFFER_SIZE - 1);
        cli->history[CLI_HISTORY_SIZE - 1][CLI_INPUT_BUFFER_SIZE - 1] = '\0';
    }
    cli->history_pos = cli->history_count;
}

void cli_add_command(Cli *cli, CliCommand *command) {
    command->next = cli->commands;
    cli->commands = command;
}

void cli_remove_command(Cli *cli, const char *name) {
    CliCommand **current = &cli->commands;
    
    while (*current != NULL) {
        if (strcmp((*current)->name, name) == 0) {
            CliCommand *to_remove = *current;
            *current = (*current)->next;
            // Не освобождаем память, так как команды обычно статические
            return;
        }
        current = &(*current)->next;
    }
}

void cli_print_help(Cli *cli) {
    cmd_help(cli, 0, NULL);
}

void cli_write_char(Cli *cli, char c) {
    cli->write_char(c);
}

void cli_print(Cli *cli, const char *str) {
    while (*str) {
        cli_write_char(cli, *str++);
    }
}

void cli_println(Cli *cli, const char *str) {
    cli_print(cli, str);
    cli_print(cli, CLI_NEWLINE);
}

void cli_clear_screen(Cli *cli) {
    cli_print(cli, CLI_CLEAR_SCREEN);
}

void cli_show_prompt(Cli *cli) {
    cli_print(cli, CLI_PROMPT);
}
