#include <stdio.h>
#include <string.h>
#include <x68k/dos.h>

int main(int argc, char **argv, char **envp)
{
    if (argc < 2) {
        printf("Usage: %s <directory>\n", argv[0]);
        return 1;
    }

    const char *match = argv[1];

    char path[256];

    if (_dos_getenv("path", NULL, path) < 0) {
        printf("Error: no path environment variable\n");
        return 1;
    }

    char *delim = ";";
    char *p;
    char str[4096];

    strcpy(str, "");
    p = strtok(path, delim);
    while (p) {
        char *s1 = p;
        const char *s2 = match;
        char c;
        while ((c = *s2++) != '\0') {
            if (*s1++ != c && c != '.')
                break;
        }
        if (c != '\0') {
            strcat(str, p);
            strcat(str, ";");
        }
        p = strtok(NULL, delim);
    }

    if (strlen(str) > 0) {
        str[strlen(str) - 1] = '\0';
    }

    _dos_setenv("path", NULL, str);

    return 0;
}
