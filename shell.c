#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define TOKEN_DELIM " \t\r\n\a"

// Function declarations
void shell_loop();
char *read_input();
char **parse_input(char *input);
int execute_command(char **args);
void print_help();
void run_llama_model(const char *prompt);

void shell_loop() {
    char *input;
    char **args;
    int status;

    do {
        printf("mysh> ");
        input = read_input();
        args = parse_input(input);
        status = execute_command(args);

        free(input);
        free(args);
    } while (status);
}

char *read_input() {
    char *line = NULL;
    size_t bufsize = 0;
    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            // EOF (Ctrl+D) exits shell
            exit(EXIT_SUCCESS);
        } else {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

char **parse_input(char *input) {
    int bufsize = 64, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    token = strtok(input, TOKEN_DELIM);
    while (token != NULL) {
        tokens[position++] = token;
        if (position >= bufsize) {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char*));
        }
        token = strtok(NULL, TOKEN_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int execute_command(char **args) {
    if (args[0] == NULL) return 1;

    if (strcmp(args[0], "exit") == 0) {
        return 0;
    }

    if (strcmp(args[0], "help") == 0) {
        print_help();
        return 1;
    }

    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: expected argument\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
        return 1;
    }

    if (strncmp(args[0], "ai:", 3) == 0) {
        // Compose prompt from everything after "ai:"
        char prompt[1024] = {0};
        strcpy(prompt, args[0] + 3);
        for (int i = 1; args[i] != NULL; i++) {
            strcat(prompt, " ");
            strcat(prompt, args[i]);
        }
        run_llama_model(prompt);
        return 1;
    }

    // Otherwise run external command
    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            fprintf(stderr, "Error: '%s' is not a recognized command.\n", args[0]);
            fprintf(stderr, "Tip: Type 'help' to see available commands.\n");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork");
    } else {
        int status;
        waitpid(pid, &status, 0);
    }

    return 1;
}

void print_help() {
    printf("Available commands:\n");
    printf("cd [dir]     - Change directory\n");
    printf("exit         - Exit the shell\n");
    printf("help         - Show this help message\n");
    printf("ai: <prompt> - Run AI prompt using llama model\n");
    printf("Other commands run as normal shell commands\n");
}

void run_llama_model(const char *prompt) {
    char command[4096];

    // Run llama-run exactly like you run manually for best speed & reliability
    snprintf(command, sizeof(command),
        "/home/khushi/llama.cpp/build/bin/llama-run "
        "/home/khushi/llama.cpp/models/mistral-7b-instruct-v0.1.Q4_K_M.gguf "
        "\"%s\" --n_predict 50 --ctx-size 2048 --temp 0.7",
        prompt);

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run llama-run");
        return;
    }

    char output[4096];
    while (fgets(output, sizeof(output), fp) != NULL) {
        printf("%s", output);
    }

    pclose(fp);
}

int main(int argc, char **argv) {
    shell_loop();
    return EXIT_SUCCESS;
}
