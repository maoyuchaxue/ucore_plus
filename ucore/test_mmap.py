
import mmap
import os
import sys
import random
import subprocess

BINARY_FILE_NAME = "/dev/shm/kafl_qemu_binary_0"
PAYLOAD_FILE_NAME = "/dev/shm/kafl_qemu_payload_0"
COVERAGE_FILE_NAME = "/dev/shm/kafl_qemu_coverage_0"
BITMAP_FILE_NAME = "/dev/shm/kafl_bitmap_0"


bin_f = os.open(BINARY_FILE_NAME, os.O_RDWR | os.O_SYNC | os.O_CREAT)
pay_f = os.open(PAYLOAD_FILE_NAME, os.O_RDWR | os.O_SYNC | os.O_CREAT)
cov_f = os.open(COVERAGE_FILE_NAME, os.O_RDWR | os.O_SYNC | os.O_CREAT)
bit_f = os.open(BITMAP_FILE_NAME, os.O_RDWR | os.O_SYNC | os.O_CREAT)


os.ftruncate(bin_f, 100)
os.ftruncate(pay_f, 100)
os.ftruncate(cov_f, 100)
bin_mem = mmap.mmap(bin_f, 100, mmap.MAP_SHARED, mmap.PROT_WRITE | mmap.PROT_READ)
pay_mem = mmap.mmap(pay_f, 100, mmap.MAP_SHARED, mmap.PROT_WRITE | mmap.PROT_READ)
cov_mem = mmap.mmap(cov_f, 100, mmap.MAP_SHARED, mmap.PROT_WRITE | mmap.PROT_READ)

prc = subprocess.Popen("bash kvm_uCore_run.sh", shell=True)
    
hello_world = "hello world with QEMU-PT!\n"
pay_mem.write(hello_world)
    
cov_out = cov_mem.read(100)

print(cov_out)

prc.wait()