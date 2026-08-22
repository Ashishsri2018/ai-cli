CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 -Iinclude
LDFLAGS ?= -lcurl

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

SRCS = src/utils.cpp \
       src/json_dump.cpp \
       src/json_parse.cpp \
       src/config.cpp \
       src/session.cpp \
       src/http_client.cpp \
       src/provider.cpp \
       src/provider_gemini.cpp \
       src/provider_openai.cpp \
       src/provider_anthropic.cpp \
       src/terminal.cpp \
       src/cli.cpp \
       src/repl.cpp \
       src/query_runner.cpp \
       src/config_cmd.cpp \
       src/models_cmd.cpp \
       src/crypto.cpp \
       src/crypto_cipher.cpp

MAIN_SRC = src/main.cpp

TEST_SRCS = tests/test_main.cpp \
            tests/test_json.cpp \
            tests/test_utils.cpp \
            tests/test_config.cpp \
            tests/test_session.cpp \
            tests/test_providers.cpp \
            tests/test_cli.cpp \
            tests/test_crypto.cpp

OBJS = $(SRCS:.cpp=.o)
MAIN_OBJ = $(MAIN_SRC:.cpp=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

TARGET = ai
TEST_TARGET = run_tests

.PHONY: all clean test install

all: $(TARGET) install
all: $(TARGET)

$(TARGET): $(OBJS) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

install: $(TARGET)
	@mkdir -p $(BINDIR)
	@cp -f $(TARGET) $(BINDIR)/$(TARGET)
	@chmod 755 $(BINDIR)/$(TARGET)
	@if [ -d "/root/.local/bin" ]; then cp -f $(TARGET) /root/.local/bin/$(TARGET) && chmod 755 /root/.local/bin/$(TARGET); fi
	@echo "Installed $(TARGET) to $(BINDIR)/$(TARGET) for global access."
	@if [ -w "$(BINDIR)" ]; then \
		mkdir -p $(BINDIR) && cp -f $(TARGET) $(BINDIR)/$(TARGET) && chmod 755 $(BINDIR)/$(TARGET) && echo "Installed $(TARGET) to $(BINDIR)/$(TARGET) for global access."; \
	else \
		mkdir -p $(HOME)/.local/bin && cp -f $(TARGET) $(HOME)/.local/bin/$(TARGET) && chmod 755 $(HOME)/.local/bin/$(TARGET) && echo "Installed $(TARGET) to $(HOME)/.local/bin/$(TARGET)."; \
	fi

$(TEST_TARGET): $(OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -Itests -o $@ $^ $(LDFLAGS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(OBJS) $(MAIN_OBJ) $(TEST_OBJS) $(TARGET) $(TEST_TARGET)
