#The following capitalized variables store the command-line commands to perform various functions
#These can be edited to customize how/which commands are called.

CC=$(CROSS_COMPILE)gcc $(CFLAGS)
CXX=$(CROSS_COMPILE)g++ $(CXXFLAGS)
LD=$(CROSS_COMPILE)g++
AR=$(CROSS_COMPILE)ar $(ARFLAGS)
HEX=$(CROSS_COMPILE)objcopy $(HEXFLAGS)
AVRDUDE=avrdude
RM=rm -rf
CP=cp -r
MV=mv
MKDIR=mkdir -p

#These lower cased commands wrap the above calls to control verbosity of the
#output. In general, none of the lower cased variables or commands should be
#changed.

rm=$(quiet)$(RM)
cp=$(quiet)$(CP)
mv=$(quiet)$(MV)
mkdir=$(quiet)$(MKDIR)

makeopts=
make=@$(MAKE) $(makeopts)

log=$(if $(quiet),,$(warning $1))
local=$(subst $(CURDIR)/./,,$(subst ./,,$(subst $(CURDIR)/,,$1)))

#These functions provide pretty output for build steps and optionally display
#the full command used in the build step.

define cc
	@echo "    CC $(notdir $@)"
	@mkdir -p $(BUILD_DIR)
	$(quiet)  $(CC) -o $(BUILD_DIR)/$(subst .c,.o,$(notdir $^)) -c $(call local,$^)
	@echo "    DEP $@"
	$(quiet)$(CC) -MM $(call local,$^) -MF $(BUILD_DIR)/$*.d
	$(quiet) buildtools/bin/fixdeps $(BUILD_DIR)/$*.d.tmp $(BUILD_DIR)/$*.d
	$(rm) $(BUILD_DIR)/$*.d.tmp
endef

define cxx
	@echo "    CXX $(notdir $@)"
	@mkdir -p $(BUILD_DIR)
	$(quiet)  $(CXX) -o $(BUILD_DIR)/$(subst .cpp,.o,$(filter-out %.h,$(notdir $^))) -c $(filter-out %.a %.h,$(call local,$^))
	@echo "    DEP $@"
	$(quiet)$(CXX) -MM $(call local,$^) -MF $(BUILD_DIR)/$*.d
	$(quiet) buildtools/bin/fixdeps $(BUILD_DIR)/$*.d.tmp $(BUILD_DIR)/$*.d
	$(rm) $(BUILD_DIR)/$*.d.tmp
endef

define ld
	@echo "    LD $@"
	$(quiet)  $(LD) -o $(BUILD_DIR)/$@ $(foreach file,$(call local,$^),$(BUILD_DIR)/$(notdir $(file))) $(LDFLAGS)
	@mkdir -p $(BIN_DIR)
	$(mv) $(BUILD_DIR)/$@ $(BIN_DIR)
endef

define ar
	@echo "    AR $@"
	$(quiet)  $(AR) $(BUILD_DIR)/$@ $(foreach file,$(call local,$^),$(BUILD_DIR)/$(notdir $(file)))
	@mkdir -p $(LIB_DIR)
	$(cp) $(BUILD_DIR)/$@ $(LIB_DIR)
endef

#define depc
#	@echo "    DEP $@"
#	$(eval $@_D := $(BUILD_DIR)/$(subst .d,.c,$(^F)))
#	$(eval $@_TMP := $(BUILD_DIR)/$(subst .d.tmp,.c,$(^F)))
#	$(quiet)$(CC) -MM $(call local,$^) > $(eval $@_D)
#	$(cp) -f $(eval $@_D) $(eval $@_TMP)
#	$(quiet)sed -e 's/.*://' -e 's/\\$$//' < $(eval $@_TMP) | fmt -1 | \
#	  sed -e 's/^ *//' -e 's/$$/:/' >> $(eval $@_D)
#	$(rm) -f $(eval $@_TMP)
#endef
