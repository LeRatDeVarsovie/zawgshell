# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <string.h>
# include <unistd.h>
# include <sys/wait.h>
# include <sys/types.h>

static const char yellow[] = "\033[33m";
static char end[] = "\033[0m";


void zawgsh_loop(void);

# define ZSH_READLINE_BUFSIZE 1024
char* zsh_read_line(void);

# define ZSH_TOKEN_BUFSIZE 64
# define ZSH_TOKEN_DELIM " \t\r\n\a"
char** zsh_split_line(char* line);

int zsh_launch(char** args);

int zsh_cd(char** args);

int zsh_help(char** args);

int zsh_exit(char** args);

int zsh_num_builtins(void);

int zsh_execute(char** args);


char* builtin_str[] = {
    "cd",
    "zawg-help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &zsh_cd,
    &zsh_help,
    &zsh_exit
};

 
int main() {
    zawgsh_loop();
    return 0;
}


int zsh_execute(char** args) {
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < zsh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return zsh_launch(args);
}

int zsh_cd(char** args) {
    if (args[1] == NULL) {
        fprintf(stderr, "zawgsh: expected argument to \"cd\"\n");
    }
    else {
        if (chdir(args[1]) != 0) {
            perror("zawgsh");
        }
    }
    return 1;
}

int zsh_help(char** args) {
    int i;
    printf("%sLou Ollivier-Hostin%s Zawg-Shell\n", yellow, end);
    printf("Type command names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i=0; i < zsh_num_builtins(); i++) {
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other commands.\n");
    return 1;
}

int zsh_exit(char** args) {
    return 0;
}

int zsh_num_builtins(void) {
    return sizeof(builtin_str) / sizeof(char *);
}

void zawgsh_loop(void) {
    char* line;
    char** args;
    int status;

    do {
        printf("%s>%s ", yellow, end);
        line = zsh_read_line();
        args = zsh_split_line(line);
        status = zsh_execute(args);

        free(line);
        free(args);
    } while (status);
}

char* zsh_read_line(void) {
    int bufsize = ZSH_READLINE_BUFSIZE;
    int position = 0;
    char* buffer = malloc(sizeof(char) * bufsize);
    int character;

    if (!buffer) {
        fprintf(stderr, "zawgsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (true) {
        character = getchar();

        if (character == EOF || character == '\n') {
            buffer[position] = '\0';
            return buffer;
        }
        else {
            buffer[position] = character;
        }
        position++;

        // If the buffer is exceeded, reallocate some memory
        if (position >= bufsize) {
            bufsize += ZSH_READLINE_BUFSIZE;
            buffer = realloc(buffer, bufsize);

            if (!buffer) {
                fprintf(stderr, "zawgsh: allocation error\n");
                exit(EXIT_FAILURE);
                
            }
        }
    }
}

char** zsh_split_line(char* line) {
    int bufsize = ZSH_TOKEN_BUFSIZE;
    int position = 0;
    char** tokens = malloc(bufsize * sizeof(char*));
    char* token;

    if (!tokens) {
        fprintf(stderr, "zawgsh: allocation error\n");
        exit(EXIT_FAILURE);
    }  

    token = strtok(line, ZSH_TOKEN_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += ZSH_TOKEN_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "zawgsh: allocation error\n");
                exit(EXIT_FAILURE);
            }     
        }
        token = strtok(NULL, ZSH_TOKEN_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int zsh_launch(char** args) {
    pid_t pid;
    pid_t wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("zawgsh");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        perror("zawgsh");
    }
    else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    /*
    This is a short description of the values that can 
    be take by the process ID :

    `` If PID is greater than 0, match any process whose process ID is PID.
    If PID is (pid_t) -1, match any process.
    If PID is (pid_t) 0, match any process with the
    same process group as the current process.
    If PID is less than -1, match any process whose
    process group is the absolute value of PID. ``
    */
    return 1;
}