.PHONY: all clean

-include $(KCONFIG_AUTOCONFIG)

TARGET_CFLAGS:= -I../libs-user-ucore/ -I../libs-user-ucore/common -I../libs-user-ucore/arch/$(ARCH) -nostdinc -nostdlib -fno-builtin -fno-stack-protector -gstabs -ggdb --std=gnu99 

#TARGET_CC_SYSTEM_LIB ?=  -L/opt/FriendlyARM/toolschain/4.4.3/lib/gcc/arm-none-linux-gnueabi/4.4.3/

USER_OBJ_ROOT := $(OBJPATH_ROOT)/user-ucore

USER_LIB_SRCDIR := ../libs-user-ucore
USER_LIB_OBJDIR := $(OBJPATH_ROOT)/libs-user-ucore
USER_LIB := $(USER_LIB_OBJDIR)/ulib.a
BIN := $(USER_OBJ_ROOT)/bin
TESTBIN := $(USER_OBJ_ROOT)/testbin
INITIAL_DIR := _initial

SYZKALLER_BUGS_TEST_LIST := test_mmap_fail test_mmap_fail2 test_unlink_fail test_dup_fail test_sigaction_fail test_write_fail
USER_APPLIST:= cat pwd cat sh ls cp echo link mkdir rename unlink lsmod insmod rmmod mount umount halt udb program_fpga  $(SYZKALLER_BUGS_TEST_LIST)
#hypercalltest only for amd64

ifneq ($(UCORE_TEST),)
USER_TESTLIST := $(basename $(wildcard tests/*.c))
USER_TESTLIST += $(basename $(wildcard tests/arch/$(ARCH)/*.c))
endif

USER_APP_BINS:= $(addprefix $(BIN)/, $(USER_APPLIST))
USER_TEST_BINS := $(addprefix $(TESTBIN)/, $(notdir $(USER_TESTLIST)))

include $(USER_LIB_SRCDIR)/arch/$(ARCH)/include.mk
INITCODE_OBJ := $(USER_LIB_OBJDIR)/arch/$(ARCH)/$(ARCH_INITCODE_OBJ)
TARGET_CFLAGS += $(ARCH_CFLAGS)
TARGET_LDFLAGS += $(ARCH_LDFLAGS)

all:  $(BIN) $(TESTBIN) $(USER_LIB) $(USER_APP_BINS) $(USER_TEST_BINS) $(BIN)/SKItest 
#$(BIN)/syz-executor only for amd64

#user applications
define make-user-app
$2/$(notdir $1): $(USER_OBJ_ROOT)/$(addsuffix .o,$1) $(INITCODE_OBJ) $(USER_LIB)
	@echo LINK $$@
	$(TARGET_LD) $(TARGET_LDFLAGS) -static -T $(USER_LIB_SRCDIR)/arch/$(ARCH)/user.ld $(USER_OBJ_ROOT)/$(addsuffix .o,$1) $(INITCODE_OBJ) $(USER_LIB) -o $$@
endef

$(foreach bdir,$(USER_APPLIST),$(eval $(call make-user-app,$(bdir),$(BIN))))

$(foreach bdir,$(USER_TESTLIST),$(eval $(call make-user-app,$(bdir),$(TESTBIN))))

SKI_FILES_LIST:= ski-main ski-barriers ski-hyper ski-params ski-test
#ski 
define make-ski-app
$(USER_OBJ_ROOT)/$(addsuffix .o,$(notdir $1)): SKItest/$(addsuffix .c,$1)
	@echo COMPILE $$@ 
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $$@ $$<
endef

$(foreach bdir,$(SKI_FILES_LIST),$(eval $(call make-ski-app,$(bdir))))

$(USER_OBJ_ROOT)/%.o: %.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<

GITREV=$(shell git rev-parse HEAD)
ifeq ("$(shell git diff --shortstat)", "")
	REV=$(GITREV)
	REV=$(GITREV)+
else
endif

$(BIN)/syz-executor: executor/executor_ucore.c executor/executor_ucore.h executor/executor.h executor/common.h executor/common_ucore.h executor/syscalls_ucore.h
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $(USER_OBJ_ROOT)/$(addsuffix .o,syz-executor) executor/executor_ucore.c 
	$(TARGET_LD) $(TARGET_LDFLAGS) -static -T $(USER_LIB_SRCDIR)/arch/$(ARCH)/user.ld $(USER_OBJ_ROOT)/$(addsuffix .o,syz-executor) $(INITCODE_OBJ) $(USER_LIB) -o $@

$(BIN)/SKItest: $(USER_OBJ_ROOT)/ski-main.o $(USER_OBJ_ROOT)/ski-barriers.o $(USER_OBJ_ROOT)/ski-hyper.o $(USER_OBJ_ROOT)/ski-params.o $(USER_OBJ_ROOT)/ski-test.o
	$(TARGET_LD) $(TARGET_LDFLAGS) -static -T $(USER_LIB_SRCDIR)/arch/$(ARCH)/user.ld $^ $(INITCODE_OBJ) $(USER_LIB) -o $@

$(BIN):
	-mkdir -p $(BIN)

$(TESTBIN):
	-mkdir -p $(USER_OBJ_ROOT)/tests
	-mkdir -p $(USER_OBJ_ROOT)/tests/arch/$(ARCH)
	-mkdir -p $(TESTBIN)

ifeq ($(UCONFIG_SFS_MINIMAL_CONTENT),y)
initial_dir:
	ln -snf _initial.small $(INITIAL_DIR)
else
initial_dir:
	ln -snf _initial.big $(INITIAL_DIR)
endif

clean:
	find . -name \*.o -exec rm -f {} \;
	rm -rf $(BIN) $(TESTBIN)
	rm -f $(INITIAL_DIR)
