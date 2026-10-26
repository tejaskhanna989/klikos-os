/* runtime.c — KlikOS app runtime.
 *
 * Runs applications (compiled to aarch64-klikos-elf ELF). Not ART/Dalvik.
 */

#include <klikos/stdio.h>

int runtime_main(int argc, char **argv) {
    (void)argc; (void)argv;
    puts("[runtime] KlikOS app runtime: stub");
    return 0;
}
