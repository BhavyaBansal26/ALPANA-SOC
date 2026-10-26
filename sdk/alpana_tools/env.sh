# ALPANA SDK environment. Source it from anywhere (or from ~/.bashrc):
#   source ~/alpana-sdk/alpana_tools/env.sh
export ALPANASDK="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
export SHAKTISDK="$ALPANASDK"
alias alpana-ocd='openocd -c "debug_level 0" -f $ALPANASDK/alpana_tools/alpana_openocd.cfg'
alias alpana-run='riscv32-unknown-elf-gdb -q -x $ALPANASDK/alpana_tools/run.gdb'
alias alpana-uart='picocom -b 115200 --imap lfcrlf /dev/ttyUSB1'
case $- in *i*) echo "ALPANA SDK at $ALPANASDK  (alpana-ocd | alpana-run <file> | alpana-uart)";; esac
