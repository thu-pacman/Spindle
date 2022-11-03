cd NPB-CPP/NPB-SER
# 修改位于 config/make.def 的配置文件
# 在 C_LIB 行末添加 -L<构建结果中libstracer.so所在的目录> -lstracer
# 在 CFLAGS 行末尾添加 -fpass-plugin=<构建结果中SpindlePass.so所在的完整路径> -fno-unroll-loops -fno-vectorize

# for workload in BT CG EP FT IS LU MG SP; do
for workload in EP; do

  make "$workload" CLASS=C
done
# bin 目录下即是构建出的二进制
cd ../..
