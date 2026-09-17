CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -Wpedantic -O2
SRC=src/main.c src/csv_utils.c src/sequential_search.c src/mergesort.c src/binary_search.c
TARGET=csv_unico

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

run-seq-it: $(TARGET)
	./$(TARGET) data/novas_exemplo.csv data/destino_exemplo.csv seq-it

run-seq-rec: $(TARGET)
	./$(TARGET) data/novas_exemplo.csv data/destino_exemplo.csv seq-rec

run-merge-it: $(TARGET)
	./$(TARGET) data/novas_exemplo.csv data/destino_exemplo.csv merge-it

run-merge-rec: $(TARGET)
	./$(TARGET) data/novas_exemplo.csv data/destino_exemplo.csv merge-rec
