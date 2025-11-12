#ifndef CLI_COMMANDS_H
#define CLI_COMMANDS_H

#include "cli.h"

#ifdef __cplusplus
extern "C" {
#endif

// Стандартные команды
void cmd_help(Cli *cli, int argc, char **argv);
void cmd_echo(Cli *cli, int argc, char **argv);
void cmd_clear(Cli *cli, int argc, char **argv);
void cmd_info(Cli *cli, int argc, char **argv);
void cmd_exit(Cli *cli, int argc, char **argv);
// Регистрация стандартных команд
void cli_register_default_commands(Cli *cli);

#ifdef __cplusplus
}
#endif

#endif // CLI_COMMANDS_H
