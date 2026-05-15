# ==============================================================================
#   CITY REPORT 2026
# ==============================================================================

CC        := gcc
CFLAGS    := -Wall -Wextra -std=c99 -Iinclude

TARGET    := city_report

SRC_DIR   := src
OBJ_DIR   := obj
BIN_DIR   := .

# 1. AGGIUNTO main.c che si trova nella root
# 2. Individuazione automatica nelle sottocartelle
SRCS      := main.c \
             $(wildcard $(SRC_DIR)/*.c) \
             $(wildcard $(SRC_DIR)/adt/*.c) \
             $(wildcard $(SRC_DIR)/models/*.c) \
             $(wildcard $(SRC_DIR)/server/*.c) \
             $(wildcard $(SRC_DIR)/tests/*.c) \
             $(wildcard $(SRC_DIR)/utils/*.c)

# Generazione dei file oggetto
OBJS      := $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS))

# Rilevamento OS (Mantenuta la tua ottima logica di portabilità)
ifeq ($(OS),Windows_NT)
    RM        := del /Q /F
    FIX_BLDR  = if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
    CLEAN_ALL := rmdir /S /Q $(OBJ_DIR) 2>NUL
    EXE_EXT   := .exe
else
    RM        := rm -f
    FIX_BLDR  = mkdir -p $(dir $@)
    CLEAN_ALL := rm -rf $(OBJ_DIR) $(TARGET)
    EXE_EXT   :=
endif

FINAL_TARGET := $(TARGET)$(EXE_EXT)

all: $(FINAL_TARGET)

$(FINAL_TARGET): $(OBJS)
	@echo [LINKING] Generazione eseguibile finale: $@
	$(CC) $(CFLAGS) $^ -o $@
	@echo [OK] Compilazione completata con successo!

# Regola di compilazione modificata per gestire sia main.c che src/
$(OBJ_DIR)/%.o: %.c
	@$(FIX_BLDR)
	@echo [CC] Compilazione modulo: $<
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean fclean re

clean:
	@echo [CLEAN] Rimozione file oggetto...
	$(CLEAN_ALL)

fclean: clean
	@echo [FCLEAN] Rimozione eseguibile...
ifeq ($(OS),Windows_NT)
	-@del /Q /F $(FINAL_TARGET) 2>NUL || exit 0
else
	-@rm -f $(FINAL_TARGET)
endif

re: fclean all