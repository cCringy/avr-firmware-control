MCU   = atmega328p
F_CPU = 16000000
BAUD_SERIAL = 250000

# EDIT! If on Linux, use these (according to your device address):
PROGRAMMER_ARGS = -P COM9
SERIAL_PORT_DEBUG = COM9

# EDIT! If on Windows, use these (according to your COM port):
#PROGRAMMER_ARGS = -P COM3
#SERIAL_PORT_DEBUG = COM3

BAUD = $(BAUD_SERIAL)UL

# Directories
PROJECT_PREFIX 		:= project
BUILD_DIR			:= buildDir
BUILD_DIR_LOCAL		:= buildDirLocal
BUILD_DIR_DEBUG		:= buildDirDebug
CURRENT_DIR_PATH	:= $(CURDIR)
CURRENT_DIR_NAME	:= $(notdir $(CURRENT_DIR_PATH))
FOLDER_NAME			:= $(PROJECT_PREFIX)_$(CURRENT_DIR_NAME)
SRC_DIR 			 = $(FOLDER_NAME)/src
LIB_DIR 			 = $(FOLDER_NAME)/lib
INCLUDE_DIR 		 = $(FOLDER_NAME)/include

#################################################
# Standard compilation options "Target"
#################################################
CPPFLAGS = -DF_CPU=$(F_CPU) -DBAUD=$(BAUD)

# EDIT! If any of these options are not available on your system, you may remove them (typically MacOS):
CFLAGS = -O2 -g2 -gstabs -std=c99 -Wall	# on MacOS, remove -g2 -gstabs
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -I$(INCLUDE_DIR)	# on MacOS, remove -funsigned-bitfields

CFLAGS += -MMD -MP
CFLAGS += -ffunction-sections -fdata-sections -MT"$@" -MF"$(@:.o=.d)"

#################################################
# Standard compilation options "Local"
#################################################

# EDIT! If any of these options are not available on your system, you may remove them (typically MacOS):
CFLAGS_LOCAL = -O2 -g2 -gstabs -std=c99 -Wall	# on MacOS, remove -g2 -gstabs
CFLAGS_LOCAL += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -I$(INCLUDE_DIR)	# on MacOS, remove -funsigned-bitfields

CFLAGS_LOCAL += -MMD -MP
CFLAGS_LOCAL += -ffunction-sections -fdata-sections -MT"$@" -MF"$(@:.o=.d)"

###############################################
###############################################
###############################################

# Linker flags "Target"
# EDIT! If any of these options are not available on your system, you may replace or remove them (typically MacOS):
LDFLAGS = -Wl,-Map,$(TARGET).map	# on MacOS, replace -Map with -map
LDFLAGS += -Wl,-u,vfprintf -lprintf_flt -lm	# on MacOS, remove -u
TARGET_ARCH = -mmcu=$(MCU)

# Linker flags "Local"
# EDIT! If any of these options are not available on your system, you may replace or remove them (typically MacOS):
LDFLAGS_LOCAL = -Wl,-Map,$(TARGET).map	# on MacOS, replace -Map with -map:
LDFLAGS_LOCAL += -Wl,-u,-lm	# on MacOS, remove -u

#################################################
# Platform compilers Target 
#################################################
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRSIZE = avr-size
AVRDUDE = avrdude

#################################################
# Local compilers
#################################################
CC_LOCAL = gcc

#################################################
# Build a list of directories under src
# finds all source folders starting at src
#################################################
SOURCE_DIRS := $(shell find $(SRC_DIR) -type d)

#used to create the list of .o files that is required to define as dependencies for any link target
#the sources in this list have their relative directory path as prefix, i.e. src/subdir/myFile.c
#This automatically adds all .c files up to the specified depth of subfolders. To increase, add another /* part
#  to the list after the wildcard, e.g. for three layers deep: $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c $(SRC_DIR)/*/*/*.c)
SOURCES = $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c $(SRC_DIR)/*/*/*.c $(SRC_DIR)/*/*/*/*.c)

#this creates a list of .o files with the same path prefix as the SOURCES, i.e. src/subdir/myFile.o
RAW_OBJECT_FILES_WITH_DIRS = $(SOURCES:.c=.o)

#this cuts off the directory prefix and leaves only the filename, i.e. myFile.o
RAW_OBJECT_FILES_WITHOUT_DIR = $(notdir $(RAW_OBJECT_FILES_WITH_DIRS))

#This makes a list of all .o files that should be in the BUILD_DIR, i.e. buildDir/myFile.o
#This is used to create the dependencies for a link target, but not to actually compile them, as you can't pattern match .c to .o file.
OBJECT_FILES = $(addprefix $(BUILD_DIR)/, $(RAW_OBJECT_FILES_WITHOUT_DIR))

# For local runs
OBJECT_FILES_LOCAL = $(addprefix $(BUILD_DIR_LOCAL)/, $(RAW_OBJECT_FILES_WITHOUT_DIR))

#Use the list in $(SOURCE_DIRS) as a search path for .c files.
#This adds all .c files to that %.c thing from the list of all folders that are done by the SOURCE_DIRS
#With this you can simply do the %.o : %.c thing, as all necessary sources are well known to the %.c "variable (?)" <- not sure how much that makes sense
vpath %.c $(SOURCE_DIRS)

.PHONY: clean all

all : clean release


# Target release build
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)/$(SOURCES_DIRS) 
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
# $(OBJCOPY) -j .text -j .data -O ihex $< $@
	$(OBJCOPY) -R .eeprom -R .fuse -R .lock -R .signature -O ihex $< $@

$(BUILD_DIR)/%.eeprom: $(BUILD_DIR)/%.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@

$(BUILD_DIR)/%.lst: $(BUILD_DIR)/%.elf
	$(OBJDUMP) -h -S $< > $@

$(BUILD_DIR)/deployment.elf : $(OBJECT_FILES)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ -o $@


# Debug UART release build
$(BUILD_DIR_DEBUG)/%.hex: $(BUILD_DIR_DEBUG)/%.elf
# $(OBJCOPY) -j .text -j .data -O ihex $< $@
	$(OBJCOPY) -R .eeprom -R .fuse -R .lock -R .signature -O ihex $< $@

$(BUILD_DIR_DEBUG)/deployment_debug.elf : $(OBJECT_FILES)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ -L$(LIB_DIR) -ldebug_uart -o $@

###############################################
# Local "release" build deps
###############################################

$(BUILD_DIR_LOCAL)/%.o: %.c | $(BUILD_DIR_LOCAL)/$(SOURCES_DIRS) 
	$(CC_LOCAL) $(CFLAGS_LOCAL) -c -o $@ $<

$(BUILD_DIR_LOCAL)/localTest : $(OBJECT_FILES_LOCAL)
	$(CC_LOCAL) $(LDFLAGS_LOCAL) $^ -o $@

makeExecutable:
	chmod +x $(BUILD_DIR_LOCAL)/localTest

###############################################
# User Make targets
###############################################

clean: 
	rm -rf $(BUILD_DIR) ; \
	rm -rf $(BUILD_DIR_LOCAL); \
	rm -rf $(BUILD_DIR_DEBUG); \
	rm .map; \

compile: $(BUILD_DIR)/$(SOURCES_DIRS) | $(BUILD_DIR)/deployment.hex

compileLocal:  $(BUILD_DIR_LOCAL)/$(SOURCES_DIRS) | $(BUILD_DIR_LOCAL)/localTest

compileDebug: $(BUILD_DIR_DEBUG)/$(SOURCES_DIRS) | $(BUILD_DIR_DEBUG)/deployment_debug.hex

runLocal: compileLocal | makeExecutable
	$(BUILD_DIR_LOCAL)/localTest

createProjectFolder:
	mkdir -p $(SRC_DIR)

size: $(BUILD_DIR)/deployment.elf
	$(AVRSIZE) -C --mcu=$(MCU) $<

programArduino : $(BUILD_DIR)/deployment.hex
	$(AVRDUDE) -c arduino -p $(MCU) $(PROGRAMMER_ARGS) -D -V -U flash:w:$<

programArduinoDebug : $(BUILD_DIR_DEBUG)/deployment_debug.hex
	$(AVRDUDE) -c arduino -p $(MCU) $(PROGRAMMER_ARGS) -D -V -U flash:w:$<

debugArduino: programArduino
	python3 serial_echo.py $(SERIAL_PORT_DEBUG) $(BAUD_SERIAL)

runSerial: 
	python3 serial_echo.py $(SERIAL_PORT_DEBUG) $(BAUD_SERIAL)

plotSerial: 
	python3 serial_plot.py $(SERIAL_PORT_DEBUG) $(BAUD_SERIAL)

$(BUILD_DIR)/$(SOURCES_DIRS) :
	mkdir -p $@

$(BUILD_DIR_LOCAL)/$(SOURCES_DIRS) :
	mkdir -p $@	

$(BUILD_DIR_DEBUG)/$(SOURCES_DIRS) :
	mkdir -p $@	
