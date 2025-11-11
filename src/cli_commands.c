#include <string.h>
#include "embedded_cli/cli.h"
#include "embedded_cli/cli_commands.h"

void cmd_help(Cli *cli, int argc, char **argv) {
    CliCommand *cmd = cli->commands;
    
    cli_println(cli, "Available commands:");
    while (cmd != NULL) {
        cli_print(cli, "  ");
        cli_print(cli, cmd->name);
        cli_print(cli, " - ");
        cli_println(cli, cmd->help);
        cmd = cmd->next;
    }
}

void cmd_echo(Cli *cli, int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        cli_print(cli, argv[i]);
        if (i < argc - 1) {
            cli_print(cli, " ");
        }
    }
    cli_println(cli, "");
}

void cmd_clear(Cli *cli, int argc, char **argv) {
    cli_clear_screen(cli);
}

void cmd_info(Cli *cli, int argc, char **argv) {
    cli_println(cli, "Embedded CLI v1.0.0");
    cli_println(cli, "Cross-platform embedded command line interface");
}

// Регистрация стандартных команд
static CliCommand help_cmd = {"help", "Show this help message", cmd_help, NULL};
static CliCommand echo_cmd = {"echo", "Echo arguments", cmd_echo, NULL};
static CliCommand clear_cmd = {"clear", "Clear screen", cmd_clear, NULL};
static CliCommand info_cmd = {"info", "Show system information", cmd_info, NULL};

void cli_register_default_commands(Cli *cli) {
    cli_add_command(cli, &help_cmd);
    cli_add_command(cli, &echo_cmd);
    cli_add_command(cli, &clear_cmd);
    cli_add_command(cli, &info_cmd);
}
