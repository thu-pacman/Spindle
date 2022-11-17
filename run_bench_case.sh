USER_NAME=yhy

STRACER_DIR=/home/${USER_NAME}/instruction_lite/Spindle/build

LD_LIBRARY_PATH=${STRACER_DIR}:$LD_LIBRARY_PATH ./NPB-CPP/NPB-SER/bin/ep.C
