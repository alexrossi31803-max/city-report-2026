# ==============================================================================
#  MAKEFILE (v5.1)
# ==============================================================================

# Compilatore e flag di sistema conformi allo standard C99
CC       := gcc
CFLAGS   := -Wall -Wextra -pedantic -std=c99 -Iinclude

# Nome dell'eseguibile finale allineato alla documentazione
TARGET   := segnalazioni_municipali

# Directory del progetto
SRC_DIR  := src
OBJ_DIR  := obj
BIN_DIR  := .

# Individuazione automatica di tutti i file sorgente .c
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
    EXE_EXT  := .exe
else
    RM       := rm -f
    FIX_BLDR := mkdir -p $(dir $@)
    EXE_EXT  :=
endif

FINAL_TARGET := $(TARGET)$(EXE_EXT)

# Regola principale: esegue il linking dei moduli oggetti generati
all: $(FINAL_TARGET)

$(FINAL_TARGET): $(OBJS)
	@echo [LINKING] Generazione eseguibile finale: $@
	$(CC) $(CFLAGS) $^ -o $@
	@echo [OK] Compilazione AVL completata con successo!

# Compilazione separata con creazione dinamica dell'albero delle cartelle obj/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@$(FIX_BLDR)
	@echo [CC] Compilazione modulo: $<
	$(CC) $(CFLAGS) -c $< -o $@

# Regole di pulizia del workspace (Sanificate per Windows e POSIX)
.PHONY: clean fclean re

clean:
	@echo [CLEAN] Rimozione file oggetto temporanei...
ifeq ($(OS),Windows_NT)
	-@rmdir /S /Q $(OBJ_DIR) 2>NUL || exit 0
else
	-@rm -rf $(OBJ_DIR)
endif

fclean: clean
	@echo [FCLEAN] Rimozione eseguibile e indici temporanei...
ifeq ($(OS),Windows_NT)
	-@del /Q /F $(FINAL_TARGET) 2>NUL || exit 0
else
	-@rm -f $(FINAL_TARGET)
endif

# Forza la ricompilazione totale pulita del server municipale
re: fclean all

