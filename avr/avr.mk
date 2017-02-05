#AVR_MCU is used for specifying the controller for avr-gcc calls
AVR_MCU= atmega328p
AVR_FCPU= 16000000ul

#The AVRDUDE_* variables configure how avrdude is run
AVRDUDE_PART= m328
AVRDUDE_BITC= 10
AVRDUDE_BAUD= 115200

#Following AVRDUDE_XXXX values would be typical for using the bootloader
#AVRDUDE_PORT= /dev/ttyACM0
#AVRDUDE_PRG= stk500v2

#Parameters for using the AVR Dragon
AVRDUDE_PRG= dragon_isp
AVRDUDE_PORT= usb

#These flags override the normal flags and ensure a properly compile AVR hex
CXXFLAGS= -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=$(AVR_MCU) -D F_CPU=$(AVR_FCPU)
CFLAGS= -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=$(AVR_MCU) -D F_CPU=$(AVR_FCPU)
LDFLAGS= -Os -Wl,--gc-sections,--relax -mmcu=$(AVR_MCU) -lm
HEXFLAGS= -R .eeprom -O ihex


#Lines below this point should not be edited and ensure hex compiling and
#uploading work appropriately.

CLEANFILES+= *.elf 

#AVR project, establish CROSS_COMPILE variable for AVR-*
CROSS_COMPILE= avr-

define hex
	@echo "    HEX $@"
	$(quiet)  $(LD) -mmcu=$(AVR_MCU) -o $(BUILD_DIR)/$(subst .hex,.elf,$@) $(foreach file,$(call local,$^),$(BUILD_DIR)/$(notdir $(file)))
	$(quiet)  $(HEX) $(BUILD_DIR)/$(subst .hex,.elf,$@) $(BUILD_DIR)/$@
	@mkdir -p $(BIN_DIR)
	$(mv) $(BUILD_DIR)/$@ $(BIN_DIR)
endef

define avrdude
	@echo "    UPLOAD $(notdir $^)"
	$(quiet) $(AVRDUDE) -P $(AVRDUDE_PORT) -B $(AVRDUDE_BITC) -b $(AVRDUDE_BAUD) -c $(AVRDUDE_PRG) -p $(AVRDUDE_PART) -U flash:w:$(BIN_DIR)/$(notdir $^)
endef

%.hex: %.o
	$(hex)

up-%: %
	$(avrdude)
