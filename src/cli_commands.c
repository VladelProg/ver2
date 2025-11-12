#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include "embedded_cli/cli.h"
#include "embedded_cli/cli_commands.h"


void cmd_ps(Cli *cli, int argc, char **argv) {
    cli_println(cli, "PID\tCommand");
    cli_println(cli, "---\t-------");

    DIR *dir = opendir("/proc");
    if (!dir) {
        cli_println(cli, "Error: cannot access /proc");
        return;
    }
    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir)) != NULL && count < 50){
        if (entry->d_type == DT_DIR) {
            char *endptr;
            long pid = strtol(entry->d_name, &endptr, 10);
            if (*endptr == '\0' && pid > 0){
                char path[256];
                snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);
                FILE *f = fopen(path, "r");

                if (f) {
                    char comm[256];
                    if (fgets(comm, sizeof(comm), f)){
                        comm[strcspn(comm, "\n")] = '\0';
                        cli_print(cli, entry->d_name);
                        cli_print(cli, "\t");
                        cli_println(cli, comm);
                        count++;
                    }
                    fclose(f);
                }
            }
        }
    }

    closedir(dir);

    if (count == 0) {
        cli_println(cli, "No proc");
    }

    else {
        char count_str[32];
        snprintf(count_str, sizeof(count_str), "Total %d proc", count);
        cli_println(cli, count_str);
    }
}

void cmd_kill(Cli *cli, int argc, char **argv)
{
    if (argc < 2) {
        cli_println(cli, "Usage kill <pid> [signal]");
        cli_println(cli, "Signals");
    }

    char *endptr;
    long pid = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || pid <=0) {
        cli_println(cli, "Error: invalid PID");
    }

    int signal_num = SIGTERM;

    if (argc >=3){
        long sig = strtol(argv[2], &endptr, 10);
        if (*endptr == '\0' && sig > 0){
            signal_num = (int)sig;
        }

        if (kill((pid_t)pid, signal_num) == 0) {
            char msg[64];
            const char *sig_name = "UNKNOWN";
            switch (signal_num) {
                case SIGKILL:
                    sig_name = "KILL"; break;
                case SIGTERM:
                    sig_name = "TERM"; break;
                case SIGINT:
                    sig_name = "INT"; break;
            }
            snprintf(msg, sizeof(msg), "Sent");
            cli_println(cli, msg);
        }
       // else{
       //     c
       // }
    }
}


void cmd_exit(Cli *cli, int argc, char **argv) {
    cli_println(cli, "goodbye");
    exit(0);
}
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
    cli_println(cli, "\n\rEmbedded CLI v1.0.0\n\r");
    cli_println(cli, "\n\rCross-platform embedded command line interface\n\r");
}

// Регистрация стандартных команд
static CliCommand help_cmd = {"help", "Show this help message", cmd_help, NULL};
static CliCommand echo_cmd = {"echo", "Echo arguments", cmd_echo, NULL};
static CliCommand clear_cmd = {"clear", "Clear screen", cmd_clear, NULL};
static CliCommand info_cmd = {"info", "Show system information", cmd_info, NULL};
static CliCommand exit_cmd = {"exit", "Exit", cmd_exit, NULL};
static CliCommand ps_cmd = {"ps", "Proc", cmd_ps, NULL};
static CliCommand ps_kill = {"kill", "Kill", cmd_kill, NULL};

void cli_register_default_commands(Cli *cli) {
    cli_add_command(cli, &help_cmd);
    cli_add_command(cli, &echo_cmd);
    cli_add_command(cli, &clear_cmd);
    cli_add_command(cli, &info_cmd);
    cli_add_command(cli, &exit_cmd);
    cli_add_command(cli, &ps_cmd);
    cli_add_command(cli, &ps_kill);
}
