# 项目名与输出目录
PROJECT			= MyPlayer
OUTPUT_DIR		= ./build/Debug
BUILD_DIR 		= ./build/make

# 定义文件格式和文件名
TARGET_ELF              := $(OUTPUT_DIR)/$(PROJECT).elf
TARGET_BIN              := $(OUTPUT_DIR)/$(PROJECT).bin
TARGET_HEX              := $(OUTPUT_DIR)/$(PROJECT).hex
OBJCPFLAGS_ELF_TO_BIN    = -Obinary
OBJCPFLAGS_ELF_TO_HEX    = -Oihex

# GNU工具链路径
TOOLCHAIN_DIR= D:/dev/Embedded/arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-arm-none-eabi

# 头文件搜索路径
INC_DIR     := -I./src -I./lib/cmsis -I./hal/STM32F10x_StdPeriph_Driver -Ihal/STM32F10x_StdPeriph_Driver/inc \
			   -I$(TOOLCHAIN_DIR)/arm-none-eabi/include

# 链接脚本文件
LDSCRIPT    := ./stm32f1x_64KB_flash.ld

# 工具链可执行文件
CC          = arm-none-eabi-gcc
OBJCP       = arm-none-eabi-objcopy

# 编译附加参数
CCFLAGS     += -c -xc -mthumb -std=c11 -mcpu=cortex-m3 -Og -Wall -g -ffunction-sections -fdata-sections --specs=nosys.specs --specs=nano.specs
ASFLAGS     += -c -x assembler-with-cpp -mthumb -mcpu=cortex-m3 -g -ffunction-sections -fdata-sections --specs=nosys.specs --specs=nano.specs
LDFLAGS		+= -mthumb -mcpu=cortex-m3 -T$(LDSCRIPT) -Wl,--print-memory-usage -Wl,--gc-sections --specs=nosys.specs --specs=nano.specs

# 添加头文件搜索路径
CCFLAGS     += $(INC_DIR)
ASFLAGS		+= $(INC_DIR)

# 编译时预定义宏
CCFLAGS     += -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER

# C源文件与启动文件
SRC_DIRS	:= src src/states src/FATFS src/sdcard src/oled \
			   hal/STM32F10x_StdPeriph_Driver hal/STM32F10x_StdPeriph_Driver/src
SOURCE		:= $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
SOURCE_ASM	+= ./src/startup_stm32f10x_md.s

# 指定中间产物
C_OBJS		:= $(patsubst %.c,$(BUILD_DIR)/%.o,$(SOURCE))
ASM_OBJS	:= $(patsubst %.s,$(BUILD_DIR)/%.o,$(SOURCE_ASM))

# 指令
COMPILE     = $(CC) $(CCFLAGS) -c $< -o $@
ASSEMBLE    = $(CC) $(ASFLAGS) -c $< -o $@
LINK        = $(CC) $(C_OBJS) $(ASM_OBJS) $(LDFLAGS) -o $@
ELF_TO_BIN  = $(OBJCP) $(OBJCPFLAGS_ELF_TO_BIN) $< $@
ELF_TO_HEX	= $(OBJCP) $(OBJCPFLAGS_ELF_TO_HEX) $< $@

# 构建目标
.PHONY: all clean cleanAll

all: $(TARGET_HEX) $(TARGET_BIN)
	@echo "Build done"

$(TARGET_HEX): $(TARGET_ELF)
	$(ELF_TO_HEX)

$(TARGET_BIN): $(TARGET_ELF)
	$(ELF_TO_BIN)

$(TARGET_ELF): $(C_OBJS) $(ASM_OBJS)
	$(LINK)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(COMPILE)

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(ASSEMBLE)

# 清理项
clean:
	rm -rf $(BUILD_DIR)
	@echo "Clean done"

cleanAll:
	rm -f $(TARGET_HEX) $(TARGET_ELF) $(TARGET_BIN)
	rm -rf $(BUILD_DIR)
	@echo "Cleaned All"