#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/param.h>
#include <fcntl.h>

void
usage(FILE *f, const char *name)
{
    fprintf(f, "Usage: %s [DIR] [--show-hidden|--help]\n", name);
}

char tmpname[] = ".diredit-XXXXXX";

void
cleanup(void)
{
    (void) unlink(tmpname);
}

int
main(int argc, char *argv[])
{
    /* parse command line arguments */
    int show_hidden = 0;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--show-hidden") == 0)
        {
            show_hidden = 1;
            continue;
        }

        if (strcmp(argv[i], "--help") == 0)
        {
            usage(stdout, argv[0]);
            return 0;
        }

        if (strlen(argv[i]) > 0 && argv[i][0] == '-')
        {
            usage(stderr, argv[0]);
            fprintf(stderr, "error: invalid option %s\n", argv[i]);
            return 1;
        }

        if (chdir(argv[i]) < 0)
        {
            perror("chdir");
            return 1;
        }
    }

    char *path = getcwd(NULL, MAXPATHLEN);
    if (path == NULL)
    {
        perror("getcwd");
        return 1;
    }

    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        perror("opendir");
        return 1;
    }

    /* create temporary file */
    int fd = mkstemp(tmpname);
    if (fd == -1)
    {
        perror("mkstemp");
        return 1;
    }

    atexit(cleanup);

    FILE *f = fdopen(fd, "w");
    if (f == NULL)
    {
        perror("fdopen");
        return 1;
    }

    char *editor = getenv("EDITOR");
    if (editor == NULL)
    {
        editor = strdup("nano");
    }

    /* read directoy content */
    struct dirent *ent;
    char **oldlist = NULL;
    size_t oldcount = 0;
    size_t listcap = 16;

    oldlist = malloc(sizeof (*oldlist) * listcap);
    if (oldlist == NULL)
    {
        perror("malloc");
        return 1;
    }

    while ((ent = readdir(dir)))
    {
        if (oldcount == listcap)
        {
            listcap *= 2;
            oldlist = realloc(oldlist, sizeof (*oldlist) * listcap);
            if (oldlist == NULL)
            {
                perror("realloc");
                return 1;
            }
        }

        if (strcmp(ent->d_name, tmpname) == 0)
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

        if (!show_hidden && strlen(ent->d_name) > 0 && ent->d_name[0] == '.')
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

    /* open editor */
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        char *argv[] = { editor, tmpname, NULL };
        if (execvp(editor, argv) < 0)
        {
            perror("execvp");
            return 1;
        }
    }

    int status;
    if (wait(&status) < 0)
    {
        perror("wait");
        return 1;
    }

    if (!WIFEXITED(status))
    {
        return 1;
    }

    if (WEXITSTATUS(status) != 0)
    {
        return WEXITSTATUS(status);
    }

    f = fopen(tmpname, "r");
    if (f == NULL)
    {
        perror("fopen");
        return 1;
    }

    /* read temporary file */
    char **newlist = NULL;
    size_t newcount = 0;
    char *line = NULL;
    size_t linesz = 0;

    newlist = malloc(sizeof (*newlist) * oldcount);
    if (newlist == NULL)
    {
        perror("malloc");
        return 1;
    }

    while (getline(&line, &linesz, f) >= 0)
    {
        if (newcount == oldcount)
        {
            fprintf(stderr, "error: file count mismatch\n");
            return 1;
        }

        size_t linelen = strlen(line);

        if (line[linelen - 1] == '\n')
        {
            --linelen;
            line[linelen] = '\0';
        }

        if (linelen == 0)
        {
            continue;
        }

        if (strcmp(line, tmpname) == 0)
        {
            continue;
        }

        newlist[newcount++] = strdup(line);
    }

    if (oldcount > newcount)
    {
        fprintf(stderr, "error: file count mismatch\n");
        return 1;
    }

    int ret = 0;

    /* rename files */
    for (size_t i = 0; i < oldcount; i++)
    {
        if (strcmp(oldlist[i], newlist[i]) == 0)
        {
            continue;
        }

        if (renameat(AT_FDCWD, oldlist[i], AT_FDCWD, newlist[i]) < 0)
        {
            perror("renameat");
            ret = 1;
        }
    }

    return ret;
}
