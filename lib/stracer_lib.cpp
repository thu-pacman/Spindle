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

void __spindle_record_br(bool cond) {
    fprintf(fp, "br %d\n", cond);
}
}