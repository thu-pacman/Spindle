# build
rm -r ./build
mkdir -p ./build
cd build
cmake ..
make -j
cd ..
echo "build completed !!!"

USER_NAME=yhy
# PROG_NAME=GEP_nested
# PROG_NAME=nested_loop
# PROG_NAME=loop_unsafe
# PROG_NAME=nested_loop_2
PROG_NAME=nested_while_loop
# PROG_NAME=nested_do_while_loop


# STRACER_DIR=./build
# SpindlePass_DIR=./build/SpindlePass
STRACER_DIR=/home/${USER_NAME}/instruction_lite/Spindle/build
SpindlePass_DIR=/home/${USER_NAME}/instruction_lite/Spindle/build/SpindlePass


PROG_SRC=./examples/${PROG_NAME}.c
PROG_IR=./examples/${PROG_NAME}.ll
PROG_IR_OLD=./examples/${PROG_NAME}_old.ll
PROG_IR_OLD_LS=./examples/${PROG_NAME}_old_ls.ll
PROG_BIN=./build/examples/${PROG_NAME}

mkdir -p ./build/examples
# __DEBUG=1 \
clang ${PROG_SRC} -O2 -lstracer -L${STRACER_DIR} -fpass-plugin=${SpindlePass_DIR}/SpindlePass.so -fno-unroll-loops -fno-vectorize -S -emit-llvm -o ${PROG_IR} # -o ${PROG_BIN}
clang ${PROG_SRC} -O2 -lstracer -L${STRACER_DIR} -fpass-plugin=${SpindlePass_DIR}/SpindlePass.so -fno-unroll-loops -fno-vectorize -o ${PROG_BIN}
clang -S -emit-llvm ${PROG_SRC} -O2 -fno-unroll-loops -fno-vectorize -o ${PROG_IR_OLD}
# opt -passes='loop-simplify' ${PROG_IR_OLD} -S -o ${PROG_IR_OLD_LS}

LD_LIBRARY_PATH=${STRACER_DIR}:$LD_LIBRARY_PATH ${PROG_BIN}
