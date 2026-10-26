/* coreutils stubs — will be wired to real syscalls later.
 * These are placeholders so the build has something to link against.
 */

#include <klikos/unistd.h>
#include <klikos/stdio.h>

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    puts("[coreutils] stub — not yet implemented");
    return 0;
}
