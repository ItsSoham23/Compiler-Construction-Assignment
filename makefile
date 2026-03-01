CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lm

TARGET = stage1exe

SOURCES = driver.c lexer.c parser.c
OBJECTS = $(SOURCES:.c=.o)

HEADERS = parser.h parserDef.h lexer.h lexerDef.h

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)
	@echo "✓ Compilation successful!"
	@echo "  Executable: $(TARGET)"
	@echo "  Usage: ./$(TARGET) <testcase.txt> <parsetreeOutFile.txt>"

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "✓ Clean completed"

rebuild: clean all

.PHONY: all clean rebuild
