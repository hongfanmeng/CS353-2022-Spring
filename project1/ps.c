#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#define MAX 1024
int main(void)
{
    DIR* procDir;
    struct dirent* procEntry;
    char buff[MAX], d_name[MAX], path[MAX];
    char statusCode, cmd[MAX];
    int pid;
    FILE* file;

    procDir = opendir("/proc");
    if (procDir == NULL) {
        perror("Couldn't open /proc");
    }

    printf("%5s %s %s\n", "PID", "S", "CMD");
    while ((procEntry = readdir(procDir))) {
        strcpy(d_name, procEntry->d_name);

        // check is it a number
        int i;
        for (i = 0; isdigit(d_name[i]); i++);
        if (i == 0 || d_name[i] != '\0') {
            continue;
        }

        sscanf(d_name, "%d", &pid);

        sprintf(path, "/%s/%d/%s", "proc", pid, "stat");
        // print /proc/pid/stat
        file = fopen(path, "r");
        if (file == NULL) continue;
        fgets(buff, MAX, file);
        for (i = 0; !(buff[i] == ')' && buff[i + 1] == ' ' && buff[i + 2] != '\0')
            && buff[i] != '\0';i++);
        if (buff[i] != '\0')
            statusCode = buff[i + 2];
        else continue;
        fclose(file);

        // print /proc/pid/cmdline or /proc/pid/comm
        sprintf(path, "/%s/%d/%s", "proc", pid, "cmdline");
        file = fopen(path, "r");
        if (file == NULL) continue;
        int c = fgetc(file);
        // empty cmdline
        if (c == EOF) {
            fclose(file);
            sprintf(path, "/%s/%d/%s", "proc", pid, "comm");
            file = fopen(path, "r");
            if (file == NULL) continue;
            fgets(buff, MAX - 2, file);
            int len = strlen(buff);
            if (buff[len - 1] == '\n') buff[len - 1] = '\0';
            sprintf(cmd, "[%s]", buff);
        }
        else {
            ungetc(c, file);
            fgets(cmd, MAX, file);
        }
        fclose(file);
        int len = strlen(cmd);
        if (cmd[len - 1] == '\n') cmd[len - 1] = '\0';
        printf("%5d %c %s\n", pid, statusCode, cmd);
    }

    closedir(procDir);
    return 0;
}