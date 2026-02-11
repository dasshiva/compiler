OBJDIR := objdir

SRC_FILES := $(shell find src -name "*.c")
OBJ_FILES := $(patsubst src/%.c, objdir/%.o, $(SRC_FILES))

$(OBJDIR)/%.o: src/%.c
	cc $(CFLAGS) -Iinclude -c $< -o $@

all: $(OBJ_FILES)
	cc $^ -o lang

$(OBJ_FILES): | $(OBJDIR)
$(OBJDIR): 
	mkdir -p objdir
	mkdir -p objdir/parser

clean:
	rm -f objdir/*.o
	rm -f objdir/parser/*.o
	rm -f lang
