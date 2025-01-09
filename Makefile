LIBNAME:=a64jit
BIN:=bin
BUILD:=build


CFLAGS:= -std=c2x -pedantic -Wall -Werror -Wno-newline-eof -Wno-gnu-binary-literal
LIB_CFLAGS:=-O3 -flto
TEST_CFLAGS:=-g

TEST_BIN:=$(BIN)/test
LIB_TARGET:=$(BIN)/$(LIBNAME)

HEADERS:=$(wildcard headers/*.h)
LIBSRC:=$(wildcard lib/*.c)
TEST_SRC:=$(wildcard tests/*.c)

LIB_OBJ:=$(patsubst lib/%.c, $(BUILD)/%.o, $(LIBSRC))
TESTS:=$(patsubst tests/%.c, $(TEST_BIN)/%, $(TEST_SRC))

UNAME:=$(shell uname -s)

RE_JIT_LIB:=../jit-regex/lib/$(UNAME)
RE_JIT_INCLUDE:=../jit-regex/include

all: $(LIB_TARGET) Makefile

.PHONY: clean


OBJ_MSG_PRINTED:=1
TEST_MSG_PRINTED:=1
LIB_MSG_PRINTED:=1

$(BUILD)/%.o: lib/%.c $(HEADERS)
	@mkdir -p $(dir $@)

	$(if $(filter 0,$(MAKELEVEL)), $(if $(filter 0,$(OBJ_MSG_PRINTED)),, \
	$(eval OBJ_MSG_PRINTED:=0) \
	@echo "\nCompiling object files"))

	@$(CC) $(CFLAGS) $(LIB_CFLAGS) -c $< -o $@
	@printf " - %-25s <- %s\n" "$@" " $<"

$(LIB_TARGET): $(LIB_OBJ)
	@mkdir -p $(dir $@)

	$(if $(filter 0,$(MAKELEVEL)), $(if $(filter 0,$(LIB_MSG_PRINTED)),, \
	$(eval LIB_MSG_PRINTED:=0) \
	@echo "\nBuilding library"))

	@$(CC) $(CFLAGS) $(LIB_CFLAGS) -shared -o $@ $(LIB_OBJ)
	@printf " - %-25s <- %s\n" "$@" " $(LIB_OBJ)"

$(TEST_BIN)/%: tests/%.c $(LIB_TARGET)
	@mkdir -p $(dir $@)

	$(if $(filter 0,$(MAKELEVEL)), $(if $(filter 0,$(TEST_MSG_PRINTED)),, \
	$(eval TEST_MSG_PRINTED:=0) \
	@echo "\nCompiling tests"))

	@$(CC) $(CFLAGS) $(TEST_CFLAGS) -o $@ $< -L$(BIN) $(LIB_TARGET)
	@printf " - %-25s <- %s\n" "$@" " $<"

run-tests: $(TESTS)
	@mkdir -p files
	@echo "\nRunning tests\n"
	@pass=0; fail=0; \
	green='\033[32m'; red='\033[31m'; reset='\033[0m'; \
	for test in $(TESTS); do \
		if $$test; then \
			printf "%-50s %bPASSED%b\n" "Test $$(basename $$test):" "$${green}" "$${reset}"; \
			pass=$$((pass+1)); \
		else \
			printf "%-50s %bFAILED%b\n" "Test $$(basename $$test):" "$${red}" "$${reset}"; \
			fail=$$((fail+1)); \
		fi; \
	done; \
	echo "\nSummary: $${green}$$pass passed$${reset}, $${red}$$fail failed$${reset}";

$(RE_JIT_LIB)/a64jit: $(LIB_OBJ)
	@mkdir -p $(dir $@)

	$(if $(filter 0,$(MAKELEVEL)), $(if $(filter 0,$(LIB_MSG_PRINTED)),, \
	$(eval LIB_MSG_PRINTED:=0) \
	@echo "\nInstalling library"))

	@$(CC) $(CFLAGS) $(LIB_CFLAGS) -shared -o $@ $(LIB_OBJ)
	@printf " - %-25s <- %s\n" "$@" " $(LIB_OBJ)"

install: $(RE_JIT_LIB)/a64jit $(HEADERS)
	@cp	$(HEADERS) $(RE_JIT_INCLUDE)

clean:
	rm -rf $(BIN)
	rm -rf $(BUILD)
