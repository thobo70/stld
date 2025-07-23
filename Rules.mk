# Common build rules and utilities

# Automatic dependency generation
%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# Colored output
NO_COLOR := \033[0m
RED := \033[0;31m
GREEN := \033[0;32m
YELLOW := \033[0;33m
BLUE := \033[0;34m

define print_info
	@printf "$(BLUE)[INFO]$(NO_COLOR) %s\n" "$(1)"
endef

define print_success
	@printf "$(GREEN)[SUCCESS]$(NO_COLOR) %s\n" "$(1)"
endef

define print_warning
	@printf "$(YELLOW)[WARNING]$(NO_COLOR) %s\n" "$(1)"
endef

define print_error
	@printf "$(RED)[ERROR]$(NO_COLOR) %s\n" "$(1)"
endef

# Build information
build-info:
	$(call print_info,Building $(PROJECT_NAME) v$(VERSION))
	$(call print_info,Build type: $(BUILD_TYPE))
	$(call print_info,Compiler: $(CC))
	$(call print_info,Flags: $(CFLAGS))

# Verbose output control
ifeq ($(V),1)
    Q :=
else
    Q := @
endif

# Create build directories
$(shell mkdir -p $(BUILD_DIR)/src/common $(BUILD_DIR)/src/stld $(BUILD_DIR)/src/star)
$(shell mkdir -p $(BUILD_DIR)/tests/unit/common $(BUILD_DIR)/tests/unit/stld $(BUILD_DIR)/tests/unit/star)
$(shell mkdir -p $(BUILD_DIR)/tests/integration $(BUILD_DIR)/tests/emulation $(BUILD_DIR)/tests/performance)
$(shell mkdir -p $(BUILD_DIR)/tests/unity)
