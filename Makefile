# ==============================================================================
#  MAKEFILE AUTOMATIZZATO PER SISTEMA SEGNALAZIONI MUNICIPALI
# ==============================================================================

# Compilatore e flag di sistema
CC       := gcc
CFLAGS   := -Wall -Wextra -std=c99 -Iinclude

# Nome dell'eseguibile finale
TARGET   := municipal_system

# Directory del progetto
SRC_DIR  := src
OBJ_DIR  := obj
BIN_DIR  := .

# Individuazione automatica di tutti i file sorgente .c nelle sottocartelle
SRCS     := $(wildcard $(SRC_DIR)/*.c) \
            $(wildcard $(SRC_DIR)/adt/*.c) \
            $(wildcard $(SRC_DIR)/models/*.c) \
            $(wildcard $(SRC_DIR)/server/*.c) \
            $(wildcard $(SRC_DIR)/tests/*.c) \
            $(wildcard $(SRC_DIR)/utils/*.c)

# Generazione speculare dei file oggetto .o nella cartella obj/
OBJS     := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Rilevamento del sistema operativo per comandi di pulizia nativi (Portabilità)
ifeq ($(OS),Windows_NT)
    RM       := del /Q /F
    FIX_BLDR := if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
    CLEAN_ALL:= rmdir /S /Q $(OBJ_DIR) 2>NUL
    EXE_EXT  := .exe
else
    RM       := rm -f
    FIX_BLDR := mkdir -p $(dir $@)
    CLEAN_ALL:= rm -rf $(OBJ_DIR) $(TARGET)
    EXE_EXT  :=
endif

FINAL_TARGET := $(TARGET)$(EXE_EXT)

# ------------------------------------------------------------------------------
# REGOLA PRINCIPALE: Compila ed esegue il linking finale
# ------------------------------------------------------------------------------
all: $(FINAL_TARGET)

$(FINAL_TARGET): $(OBJS)
	@echo [LINKING] Generazione eseguibile finale: $@
	$(CC) $(CFLAGS) $^ -o $@
	@echo [OK] Compilazione completata con successo!

# ------------------------------------------------------------------------------
# COMPILAZIONE MODULARE: Genera i file .o mantenendo l'albero delle cartelle
# ------------------------------------------------------------------------------
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@$(FIX_BLDR)
	@echo [CC] Compilazione modulo: $<
	$(CC) $(CFLAGS) -c $< -o $@

# ------------------------------------------------------------------------------
# UTILITY: Regole di pulizia del workspace
# ------------------------------------------------------------------------------
.PHONY: clean fclean re

clean:
	@echo [CLEAN] Rimozione file oggetto temporanei...
ifeq ($(OS),Windows_NT)
	-@rmdir /S /Q $(OBJ_DIR) 2>NUL || exit 0
else
	-@rm -rf $(OBJ_DIR)
endif

fclean: clean
	@echo [FCLEAN] Rimozione eseguibile di sistema...
ifeq ($(OS),Windows_NT)
	-@del /Q /F $(FINAL_TARGET) 2>NUL || exit 0
else
	-@rm -f $(FINAL_TARGET)
endif

# Ricompila l'intero progetto da zero
re: fclean all
