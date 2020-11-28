add-auto-load-safe-path /home/tig3r/esp/rpc/.gdbinit
target remote :3333
set remote hardware-watchpoint-limit 2
mon reset halt
flushregs
thb app_main
c
