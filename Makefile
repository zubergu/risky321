QEMU = qemu-system-riscv32

CC = riscv64-unknown-elf-gcc
AS = riscv64-unknown-elf-as
LD = riscv64-unknown-elf-ld
OBJCOPY = riscv64-unknown-elf-objcopy
OBJDUMP = riscv64-unknown-elf-objdump


CFLAGS = -Wall -Werror -Wno-address-of-packed-member -Wno-unknown-attributes -O0 -fno-omit-frame-pointer -fno-stack-protector -ggdb -MD
# CFLAGS = -Wall -O0 -fno-omit-frame-pointer
CFLAGS += -march=rv32im_zicsr -mabi=ilp32
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding
CFLAGS += -fno-common -nostdlib
CFLAGS += -fno-builtin-strncpy -fno-builtin-strncmp -fno-builtin-strlen -fno-builtin-memset
CFLAGS += -fno-builtin-memmove -fno-builtin-memcmp -fno-builtin-log -fno-builtin-bzero
CFLAGS += -fno-builtin-strchr -fno-builtin-exit -fno-builtin-malloc -fno-builtin-putc
CFLAGS += -fno-builtin-free
CFLAGS += -fno-builtin-memcpy -Wno-main
CFLAGS += -fno-builtin-printf -fno-builtin-fprintf -fno-builtin-vprintf
CFLAGS += -fno-pie -no-pie

kernel.elf: entry.o kernelvec.o uservec.o swtch.o
	$(CC) $(CFLAGS) -Wl,-Tkernel/kernel.ld -Wl,-Map=kernel.map -o kernel.elf \
	kernel/start.c kernel/uart.c kernel/kernel.c kernel/kprintf.c kernel/klibc.c kernel/system.c kernel/ktrap.c  kernel/kmem.c kernel/kvmem.c kernel/kproc.c \
	entry.o kernelvec.o uservec.o swtch.o

entry.o:
	$(AS) -march=rv32im_zicsr  -mabi=ilp32 kernel/entry.S -o entry.o

kernelvec.o:
	$(AS) -march=rv32im_zicsr  -mabi=ilp32 kernel/kernelvec.S -o kernelvec.o

uservec.o:
	$(AS) -march=rv32im_zicsr  -mabi=ilp32 kernel/uservec.S -o uservec.o

swtch.o:
	$(AS) -march=rv32im_zicsr  -mabi=ilp32 kernel/swtch.S -o swtch.o
	
run: kernel.elf entry.o kernelvec.o
	$(QEMU) -machine virt -smp 1 -bios none -nographic -serial mon:stdio --no-reboot \
	-kernel kernel.elf

# 	-d unimp,guest_errors,int,cpu_reset -D qemu.log \


# USER applications and linking kernel binary with user applications as binary blobs
USER_LIBRARY = usyscall.o ulibc.o uprintf.o umalloc.o

file_disk.tar: shell.elf echo.elf
	tar cf file_disk.tar -format=ustar -M hello.txt bye.txt shell.elf echo.elf

kernel_with_apps.elf: entry.o kernelvec.o uservec.o swtch.o file_disk.tar
	$(CC) $(CFLAGS) -Wl,-Tkernel/kernel.ld -Wl,-Map=kernel.map -o kernel_with_apps.elf \
	kernel/start.c kernel/uart.c kernel/kernel.c kernel/kprintf.c kernel/klibc.c kernel/system.c kernel/ktrap.c  kernel/kmem.c kernel/kvmem.c kernel/kproc.c kernel/ksyscall.c kernel/virtio.c kernel/filesystem.c kernel/elf.c kernel/kterminal.c \
	entry.o kernelvec.o uservec.o swtch.o

umalloc.o:
	$(CC) $(CFLAGS) -c user/umalloc.c -o umalloc.o

usyscall.o:
	$(AS) -march=rv32im_zicsr  -mabi=ilp32 user/usyscall.S -o usyscall.o

ulibc.o:
	$(CC) $(CFLAGS) -c user/ulibc.c -o ulibc.o

uprintf.o: usyscall.o ulibc.o
	$(CC) $(CFLAGS) -c user/uprintf.c -o uprintf.o

shell.elf: $(USER_LIBRARY)
	$(CC) $(CFLAGS) -Wl,-Tuser/user.ld -Wl,-Map=shell.map -o shell.elf user/shell.c $(USER_LIBRARY)

echo.elf: $(USER_LIBRARY)
	$(CC) $(CFLAGS) -Wl,-Tuser/user.ld -Wl,-Map=echo.map -o echo.elf user/echo.c $(USER_LIBRARY)

run_with_apps: kernel_with_apps.elf entry.o kernelvec.o
	$(QEMU) -machine virt -smp 1 -bios none -nographic -serial mon:stdio --no-reboot -global virtio-mmio.force-legacy=true \
	-drive id=drive0,file=file_disk.tar,format=raw,if=none \
    -device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.0 \
	-kernel kernel_with_apps.elf

debug_with_apps: kernel_with_apps.elf entry.o kernelvec.o
	$(QEMU) -machine virt -s -S -smp 1 -bios none -nographic -serial mon:stdio --no-reboot -global virtio-mmio.force-legacy=true \
	-drive id=drive0,file=file_disk.tar,format=raw,if=none \
    -device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.0 \
	-kernel kernel_with_apps.elf

log_with_apps: kernel_with_apps.elf entry.o kernelvec.o
	$(QEMU) -machine virt -smp 1 -bios none -nographic -serial mon:stdio --no-reboot -global virtio-mmio.force-legacy=true \
	-drive id=drive0,file=disk.tar,format=raw,if=none \
    -device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.0 \
	-d unimp,guest_errors,int,cpu_reset -D qemu.log \
	-kernel kernel_with_apps.elf

clean:
	rm *.o *.elf *.bin *.tar
