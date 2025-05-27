#ifndef SHELL_H
#define SHELL_H

void shell_loop();
char *read_input();
char **parse_input(char *input);
int execute_command(char **args);
void run_llama_model(const char *prompt);

#endif
