# Flags
CFLAGS = -std=c++17  -Wno-unused-but-set-variable -fdata-sections -ffunction-sections
LDFLAGS = -Wl,--gc-sections
OPTI = 0

# Target
TARGET_DIR = baker/dist
TARGET = $(TARGET_DIR)/baker

# Colors
RED = \033[31m
GREEN = \033[32m
BLUE = \033[34m

RESET = \033[0m
BOLD = \033[1m
DIM = \033[2m

# Compilation
$(TARGET): baker/main.cpp
	@echo "$(BLUE)$(BOLD)ⓘ$(RESET)  Compiling main.cpp"
	g++ -O$(OPTI) $(CFLAGS) -o $(TARGET) baker/main.cpp $(LDFLAGS)
	@if [ $$? -eq 0 ]; then \
	    echo "$(GREEN)✓$(RESET)  Sources compiled"; \
	    echo "$(BLUE)$(BOLD)ⓘ$(RESET)  Size of binary: $$(stat -c%s $(TARGET)) bytes"; \
	else \
	    echo "$(RED)✗$(RESET)  Source compilation failed"; \
	fi

# Utils
.PHONY: clean run

clean:
	rm -rf $(TARGET_DIR)

run:
	$(TARGET)
