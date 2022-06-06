#include <cstdio>
#include <vector>

extern "C" {

FILE *fp;
std::vector<bool> brResult;

void __spindle_init_main() {
    fp = fopen("dtrace.log", "w");
}

void __spindle_fini_main() {
    fputs("br result:\n", fp);
    for (auto b : brResult) {
        fputc(b + '0', fp);
    }
    fputc('\n', fp);
    fclose(fp);
    puts("[STracer] Dynamic trace has been collected to dtrace.log.");
}

void __spindle_record_br(bool cond) {
    brResult.push_back(cond);
}

void __spindle_record_value(unsigned long long value) {
    fprintf(fp, "%llu\n", value);
}
}