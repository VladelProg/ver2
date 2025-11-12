#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "embedded_cli/cli.h"
#include "embedded_cli/cli_commands.h"

static void cli_parse_command(Cli *cli, const char *input);
static void cli_execute_command(Cli *cli, int argc, char **argv);
static void cli_add_to_history(Cli *cli, const char *command);
static int cli_find_completions(Cli *cli, const char *prefix, CliCommand *results[], int max_results);
static void cli_show_completions(Cli *cli,CliCommand *completions[], int count );
static void cli_handle_escape_sequence(Cli *cli, char c);
static void cli_restore_saved_command(Cli *cli);
static void cli_load_history_command(Cli *cli);

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
    cli->last_completion = NULL;
    cli->escape_len = 0;
    cli->history_pos = 0;
    cli->history_count = 0;
    
    // Регистрируем стандартные команды
    cli_register_default_commands(cli);
    
    // Показываем приветственное сообщение
    cli_show_prompt(cli);
}

void cli_process(Cli *cli) {
    if (!cli->data_available()) {
        return;
    }

    char c = cli->read_char();
    
    // Обработка escape-последовательностей
    if (cli->escape_len > 0 || c == '\033') {
        cli_handle_escape_sequence(cli, c);
        return;
    }
    
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
                cli->last_completion = NULL; // Сбрасываем состояние автодополнения
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
                cli->last_completion = NULL; // Сбрасываем при редактировании
            }
            break;
            
        case '\t': // Tab - автодополнение
            cli_autocomplete(cli);
            break;
            
        case '\x01': // Ctrl+A - начало строки
            // TODO: реализовать перемещение курсора
            break;
            
        case '\x05': // Ctrl+E - конец строки
            // TODO: реализовать перемещение курсора  
            break;
            
        case '\033': // Начало escape-последовательности
            cli->escape_sequence[0] = c;
            cli->escape_len = 1;
            break;
            
        default:
            if (cli->input_pos < (CLI_INPUT_BUFFER_SIZE - 1) && c >= 32 && c <= 126) {
                cli->input_buffer[cli->input_pos++] = c;
                if (cli->echo_enabled) {
                    cli_write_char(cli, c);
                }
                cli->last_completion = NULL; // Сбрасываем при вводе новых символов
            }
            break;
    }
}


static void cli_handle_escape_sequence(Cli *cli, char c) {
    cli->escape_sequence[cli->escape_len++] = c;
    
    // Проверяем завершена ли последовательность
    if (cli->escape_len >= 3) {
        if (cli->escape_sequence[1] == '[') {
            switch (cli->escape_sequence[2]) {
                case 'A': // Стрелка вверх
                    cli_history_up(cli);
                    break;
                case 'B': // Стрелка вниз
                    cli_history_down(cli);
                    break;
                case 'C': // Стрелка вправо
                    // TODO: перемещение курсора вправо
                    break;
                case 'D': // Стрелка влево
                    // TODO: перемещение курсора влево
                    break;
            }
        }
        
        // Сбрасываем обработку escape-последовательности
        cli->escape_len = 0;
    }
    
    // Защита от слишком длинных последовательностей
    if (cli->escape_len >= sizeof(cli->escape_sequence)) {
        cli->escape_len = 0;
    }
}

// Перемещение вверх по истории
// Упрощенная версия истории
void cli_history_up(Cli *cli) {
    if (cli->history_count == 0) return;
    
    // Сохраняем текущий ввод если мы в конце истории
    if (cli->history_pos == cli->history_count) {
        strncpy(cli->completion_buffer, cli->input_buffer, CLI_INPUT_BUFFER_SIZE);
        cli->completion_pos = cli->input_pos;
    }
    
    if (cli->history_pos > 0) {
        cli->history_pos--;
        cli_load_history_command(cli);
    }
}

void cli_history_down(Cli *cli) {
    if (cli->history_count == 0) return;
    
    if (cli->history_pos < cli->history_count - 1) {
        cli->history_pos++;
        cli_load_history_command(cli);
    } else if (cli->history_pos == cli->history_count - 1) {
        cli->history_pos = cli->history_count;
        cli_restore_saved_command(cli);
    }
}

// Загрузка команды из истории в буфер ввода и отображение
void cli_load_history_command(Cli *cli) {
    cli_clear_line(cli);
    cli_show_prompt(cli);
    
    const char *cmd = cli->history[cli->history_pos];
    size_t len = strlen(cmd);
    
    // Копируем в буфер ввода
    strncpy(cli->input_buffer, cmd, CLI_INPUT_BUFFER_SIZE - 1);
    cli->input_buffer[CLI_INPUT_BUFFER_SIZE] = '\0';
    cli->input_pos = len;
    
    // Отображаем из буфера ввода
    cli_print(cli, cli->input_buffer);
}

// Восстановление сохраненной команды
void cli_restore_saved_command(Cli *cli) {
    cli_clear_line(cli);
    cli_show_prompt(cli);
    
    // Восстанавливаем из буфера сохранения в буфер ввода
    strncpy(cli->input_buffer, cli->completion_buffer, CLI_INPUT_BUFFER_SIZE - 1);
    cli->input_buffer[CLI_INPUT_BUFFER_SIZE - 1] = '\0';
    cli->input_pos = cli->completion_pos;
    
    // Отображаем из буфера ввода
    cli_print(cli, cli->input_buffer);
}

void cli_autocomplete(Cli *cli){
    if (cli->input_pos == 0){
        cli_print(cli, CLI_BELL);
        return;
    }
    cli->input_buffer[cli->input_pos] = '\0';

    CliCommand *completions[CLI_MAX_COMPLETIONS];

    int count = cli_find_completions(cli, cli->input_buffer, completions, CLI_MAX_COMPLETIONS);

    if (count == 0) {
        cli_print(cli, CLI_BELL);
        return;
    }
    else if (count == 1){
        const char *cmd_name = completions[0]->name;
        size_t prefix_len = strlen(cli->input_buffer);
        size_t  cmd_len = strlen(cmd_name);

        for (size_t i = prefix_len; i < cmd_len; i++) {
            if (cli->input_pos < (CLI_INPUT_BUFFER_SIZE -1)){
                cli->input_buffer[cli->input_pos++] = cmd_name[i];
                cli_write_char(cli, cmd_name[i]);
            }
        }

        if (cli->input_pos < (CLI_INPUT_BUFFER_SIZE - 1)){
            cli->input_buffer[cli->input_pos++] = ' ';
            cli_write_char(cli, ' ');
        }
    }
     else {
        // Несколько совпадений
        if (cli->last_completion == NULL) {
            // Первое нажатие Tab - показываем все варианты
            cli_println(cli, "");
            cli_show_completions(cli, completions, count);
            cli_show_prompt(cli);
            cli_print(cli, cli->input_buffer); // Восстанавливаем введенный текст
        }
        else {
            // Последующие нажатия Tab - циклическое переключение
            cli_complete_command(cli);
        }
    }
}


static int cli_find_completions(Cli *cli, const char *prefix, CliCommand *results[], int max_results){
    int count = 0;
    CliCommand *cmd = cli->commands;
    size_t prefix_len = strlen(prefix);

    while (cmd != NULL && count < max_results){
        if (strncmp(cmd->name, prefix, prefix_len) == 0) {
            results[count++] = cmd;
        }
        cmd = cmd->next;
    }
    return count;
}

static void cli_show_completions(Cli *cli, CliCommand *completions[], int count){
    cli_println(cli, "Possible");
    for (int i = 0; i < count; i++){
        cli_print(cli, " ");
        cli_print(cli, completions[i]->name);
        cli_print(cli, "-");
        cli_println(cli, completions[i]->help);
    }
}

void cli_complete_command(Cli *cli){
    CliCommand *completions[CLI_MAX_COMPLETIONS];
    int count = cli_find_completions(cli, cli->input_buffer, completions, CLI_MAX_COMPLETIONS);
    if (count == 0) return;

    int current_index = -1;
    for (int i = 0; i < count; i++){
        if (completions == cli->last_completion){
            current_index = i;
            break;
        }
    }

    int next_index = (current_index + 1) % count;
    cli->last_completion = completions[next_index];
    const char *cmd_name = completions[next_index]->name;
    size_t cmd_len = strlen(cmd_name);

    for (size_t i = 0; i < cli->input_pos; i++) {
        cli_print(cli, "\b \b");
    }

    cli->input_pos = 0;

    for (size_t i = 0;i < cmd_len && cli->input_pos < (CLI_INPUT_BUFFER_SIZE - 1); i++){
        cli->input_buffer[cli->input_pos++] = cmd_name[i];
        cli_write_char(cli,cmd_name[i]);
    }

    if (cli->input_pos < (CLI_INPUT_BUFFER_SIZE-1)){
        cli->input_buffer[cli->input_pos++] = ' ';
        cli_write_char(cli, ' ');
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
    
    cli_println(cli, "\n\rCommand not found. Type 'help' for available commands.\n\r");
}

static void cli_add_to_history(Cli *cli, const char *command) {
    // Не добавляем пустые команды или повторяющиеся подряд
    if (strlen(command) == 0 || 
        (cli->history_count > 0 && strcmp(cli->history[cli->history_count - 1], command) == 0)) {
        return;
    }
    
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
    cli->history_pos = cli->history_count; // Сбрасываем позицию в конец
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

void cli_clear_line(Cli *cli) {
    cli_print(cli, CLI_CLEAR_LINE);
}

void cli_show_prompt(Cli *cli) {
    cli_print(cli, CLI_PROMPT);
}
