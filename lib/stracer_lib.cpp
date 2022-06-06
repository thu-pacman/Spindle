#include <cstdio>

extern "C" {

FILE *fp;

void __spindle_init_main() {
    fp = fopen("dtrace.log", "w");
}

void __spindle_fini_main() {
    fclose(fp);
    puts("[STracer] Dynamic trace has been collected to dtrace.log.");
}
}