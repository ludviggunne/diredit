#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/param.h>
#include <fcntl.h>

int
main(int argc, char *argv[])
{
    char *path = NULL;
    char *editor = NULL;
    DIR *dir = NULL;
    struct dirent *ent = NULL;
    int fd;
    char fname[] = "diredit-XXXXXX";
    FILE *f = NULL;
    char **oldlist = NULL;
    char **newlist = NULL;
    size_t oldcount = 0;
    size_t newcount = 0;
    pid_t pid = -1;
    size_t linesz = 0;
    size_t linelen = 0;
    char *line = NULL;
    char *pathbuf = NULL;

    if (argc > 1 && chdir(argv[1]) == -1)
    {
        perror("chdir");
        return 1;
    }

    path = getcwd(NULL, MAXPATHLEN);
    if (path == NULL)
    {
        perror("getcwd");
        return 1;
    }

    dir = opendir(path);
    if (dir == NULL)
    {
        perror("opendir");
        return 1;
    }

    fd = mkstemp(fname);
    if (fd == -1)
    {
        perror("mkstemp");
        return 1;
    }

    f = fdopen(fd, "w");
    if (f == NULL)
    {
        perror("tmpfile");
        close(fd);
        unlink(fname);
        return 1;
    }

    editor = getenv("EDITOR");
    if (editor == NULL)
    {
        editor = strdup("nano");
    }

    while ((ent = readdir(dir)))
    {
        oldlist = realloc(oldlist, sizeof (*oldlist) * (oldcount + 1));
        if (oldlist == NULL)
        {
            perror("realloc");
            fclose(f);
            unlink(fname);
            return 1;
        }

        if (strcmp(ent->d_name, fname) == 0)
        {
            continue;
        }

        if (strcmp(ent->d_name, ".") == 0)
        {
            continue;
        }

        if (strcmp(ent->d_name, "..") == 0)
        {
            continue;
        }

        oldlist[oldcount++] = strdup(ent->d_name);
    }

    for (size_t i = 0; i < oldcount; i++)
    {
        fprintf(f, "%s\n", oldlist[i]);
    }

    fclose(f);
    f = NULL;

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        unlink(fname);
        return 1;
    }

    if (pid == 0)
    {
        char *argv[] = { editor, fname, NULL };
        if (execvp(editor, argv) < 0)
        {
            perror("execvp");
            return 1;
        }
    }

    int status;
    wait(&status);

    if (!WIFEXITED(status))
    {
        unlink(fname);
        return 1;
    }

    if (status != 0)
    {
        unlink(fname);
        return status;
    }

    f = fopen(fname, "r");
    if (f == NULL)
    {
        perror("fopen");
        unlink(fname);
        return 1;
    }

    while (getline(&line, &linesz, f) >= 0)
    {
        linelen = strlen(line);

        if (line[linelen - 1] == '\n')
        {
            line[linelen - 1] = '\0';
        }

        newlist = realloc(newlist, sizeof (*newlist) * (newcount + 1));

        if (strcmp(line, fname) == 0)
        {
            continue;
        }

        newlist[newcount++] = strdup(line);
    }

    fclose(f);
    unlink(fname);

    if (oldcount != newcount)
    {
        fprintf(stderr, "error: file count mismatch\n");
        return 1;
    }

    for (size_t i = 0; i < oldcount; i++)
    {
        if (strcmp(oldlist[i], newlist[i]) == 0)
        {
            continue;
        }

        if (renameat(AT_FDCWD, oldlist[i], AT_FDCWD, newlist[i]) < 0)
        {
            perror("renameat");
            return 1;
        }
    }

    return 0;
}