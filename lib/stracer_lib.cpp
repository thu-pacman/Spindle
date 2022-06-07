#include <cstdio>

FILE *fp;

extern "C" {

void __spindle_init_main() {
    fp = fopen("dtrace.json", "w");
    fputs("[\n", fp);
}

void __spindle_fini_main() {
    fputs("]\n", fp);
    fclose(fp);
    puts("[STracer] Dynamic trace has been collected to `dtrace.json`.");
}

void __spindle_record_br(bool cond) {
    fprintf(fp, "{\"type\": \"br\", \"value\": %d},\n", cond);
}

void __spindle_record_value(unsigned long long value) {
    fprintf(fp, "{\"type\": \"value\", \"value\": %llu},\n", value);
}
}