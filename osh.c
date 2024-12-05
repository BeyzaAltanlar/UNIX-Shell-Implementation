#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> //Dosya islemleri

#define MAX_LINE 80
#define HISTORY_SIZE 10

//komut gecmisi
typedef struct Command {
    char command[MAX_LINE];
    struct Command *next;
} Command;

Command *history_head = NULL;
int history_count = 0;

// Komut gecmisine ekle
void add_to_history(const char *cmd) {
    Command *new_cmd = (Command *)malloc(sizeof(Command));
    strncpy(new_cmd->command, cmd, MAX_LINE);
    new_cmd->next = history_head;
    history_head = new_cmd;
    history_count++;
}

// Komut gecmisinden alma
const char *get_history(int index) {
    Command *cmd = history_head;
    for (int i = 0; i < index && cmd != NULL; i++) {
        cmd = cmd->next;
    }
    return cmd ? cmd->command : NULL;
}

// Islem fonk
void execute_command(char **args, int background) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {  // (child process)
        // Giris ynlendirmesi
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], "<") == 0) {
                int fd = open(args[i + 1], O_RDONLY);
                if (fd < 0) {
                    perror("open failed");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
                args[i] = NULL; // < ve dosya ismini diziden cikar
                break;
            }
            // cikis yonlendirmesi
            if (strcmp(args[i], ">") == 0) {
                int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open failed");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                args[i] = NULL; // > ve dosya ismini diziden cikar
                break;
            }
        }
        execvp(args[0], args);
        perror("execvp failed");
        exit(1);
    } else {  // Ebeveyn surec
        if (!background) {
            wait(NULL);
        }
    }
}

// Pipe islemleri
void execute_piped_commands(char **cmds, int num_pipes) {
    int pipes[num_pipes][2];
    pid_t pid;

    // Pipe islemleri icin borular olustur
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("Pipe creation failed");
            exit(1);
        }
    }

    // Her komut icin process olusturulsun
    for (int i = 0; i <= num_pipes; i++) {
        pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {  // cocuk process
            if (i > 0) {  // Ilk komut degilse, girise boru ekle
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i < num_pipes) {  // Son komut degilse, cikisa boru ekle
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            for (int j = 0; j < num_pipes; j++) {  // Tum borulari kapat
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            char *args[MAX_LINE / 2 + 1];
            char *token = strtok(cmds[i], " ");
            int k = 0;
            while (token != NULL) {
                args[k++] = token;
                token = strtok(NULL, " ");
            }
            args[k] = NULL;
            execvp(args[0], args);
            perror("execvp failed");
            exit(1);
        }
    }

    // Tum borulari kapat
    for (int i = 0; i < num_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    // Ebeveyn surec, cocuk sureclerin bitmesini beklesin waiting islem
    for (int i = 0; i <= num_pipes; i++) {
        wait(NULL);
    }
}

// komutlari ayir
void parse_command(char *input, char **args) {
    char *token = strtok(input, " ");
    int i = 0;
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

// program dongusu
int main() {
    char input[MAX_LINE];
    char *args[MAX_LINE / 2 + 1];
    int should_run = 1;

    while (should_run) {
        printf("osh-> ");
        fflush(stdout);

        if (!fgets(input, MAX_LINE, stdin)) break;
        input[strcspn(input, "\n")] = 0;  // Yeni satir karakterini kaldir

        // "exit" komutunu kontrol et
        if (strcmp(input, "*exit*") == 0) {
            should_run = 0;
            break;
        }

        // Komut gecmisinden calistir
        if (strncmp(input, "!!", 2) == 0) {
            int index = 0;
            if (strlen(input) > 2 && input[2] == ' ') {
                index = atoi(&input[3]);  // "!! 1" seklinde sayi girildiyse oku
            }

            const char *history_cmd = get_history(index);
            if (history_cmd) {
                printf("Komut çalıştırılıyor: %s\n", history_cmd);
                strncpy(input, history_cmd, MAX_LINE);
            } else {
                printf("Geçersiz geçmiş indeksi\n");
                continue;
            }
        }

        // gecmise komut ekle
        add_to_history(input);

        // giris ynlendirmesi ve kontrol
        if (strchr(input, '|')) {
            char *cmds[MAX_LINE / 2 + 1];
            char *token = strtok(input, "|");
            int num_pipes = 0;

            while (token != NULL) {
                cmds[num_pipes++] = token;
                token = strtok(NULL, "|");
            }
            cmds[num_pipes] = NULL;
            execute_piped_commands(cmds, num_pipes - 1);
        } else {
            parse_command(input, args);
            execute_command(args, 0);
        }
    }

    return 0;
}

