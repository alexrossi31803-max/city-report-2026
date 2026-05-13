# ====================================================================
#  MAKEFILE CENTRALIZZATO - SISTEMA SEGNALAZIONI MUNICIPALI
# ====================================================================

# Compilatore e flag di compilazione
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -Iinclude

# Nome dell'eseguibile finale
TARGET = segnalazioni_comune

# Cartelle del progetto
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Individuazione automatica di tutti i file sorgente .c nelle sottocartelle
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/models/user.c \
       $(SRC_DIR)/models/report.c \
       $(SRC_DIR)/adt/report_list.c \
       $(SRC_DIR)/adt/report_stack.c \
       $(SRC_DIR)/adt/report_bst.c \
       $(SRC_DIR)/adt/priority_queue.c \
       $(SRC_DIR)/utils/validators.c \
       $(SRC_DIR)/utils/parser.c \
       $(SRC_DIR)/server/user_manager.c \
       $(SRC_DIR)/server/report_manager.c \
       $(SRC_DIR)/tests/test_suite.c

# Mappatura dei file sorgente nei rispettivi file oggetto (.o) dentro la cartella obj/
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Regola principale: compila tutto e genera l'eseguibile
all: $(TARGET)
	@echo "===================================================="
	@echo " [OK] Compilazione completata con successo!"
	@echo " Per avviare il programma digita: ./$(TARGET)"
	@echo "===================================================="

# Regola per il linking dell'eseguibile finale
$(TARGET): $(OBJS)
	@echo "--> Generazione eseguibile finale: $(TARGET)"
	@$(CC) $(CFLAGS) $^ -o $@

# Regola generica per compilare i file .c in file .o mantenendo le sottocartelle
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@# Crea la sottocartella corrispondente dentro obj/ se non esiste
	@mkdir -p $(dir $@)
	@echo "--> Compilazione modulo: $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Regola per pulire i file temporanei di compilazione e l'eseguibile
clean:
	@echo "--> Rimozione file oggetto e pulizia ambiente..."
	@rm -rf $(OBJ_DIR) $(TARGET)
	@echo " [OK] Pulizia completata."

# Regola per pulire e ricompilare tutto da zero
re: clean all

# Dichiara le regole come fittizie (non associate a file reali)
.PHONY: all clean re
