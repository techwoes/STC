#include <stc/cstr.h>
#include <stc/algo/coroutine.h>
#include <errno.h>

// Read file line by line using coroutines:

struct file_read {
    const char* filename;
    int cco_state;
    FILE* fp;
    cstr line;
};

void file_read(struct file_read* g)
{
    cco(g) {
        g->fp = fopen(g->filename, "r");
        g->line = cstr_init();

        cco_await(!cstr_getline(&g->line, g->fp));

    cco_final:
        printf("finish\n");
        cstr_drop(&g->line);
        fclose(g->fp);
    }
    return;
}

int main(void)
{
    struct file_read g = {__FILE__};
    int n = 0;
    cco_run(&g, file_read(&g))
    {
        printf("%3d %s\n", ++n, cstr_str(&g.line));
        //if (n == 10) cco_stop(&g);
    }
}
