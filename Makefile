CFLAGS=

all: task1 task2 task3 
	@echo "Done compiling all the tasks"

task1: main.c
	gcc  -o $@	$^ $(CFLAGS) -DTASK_1

task2: main.c
	gcc  -o $@	$^ $(CFLAGS) -DTASK_2

task3: main.c 
	gcc  -o $@	$^ $(CFLAGS) -DTASK_3

histogram: task1
	./task1 | python3 plotter.py

deliverable: clean
	echo "Zipping the current director in $(shell whoami).zip"
	zip -r $(shell whoami).zip ./*
	
clean:
	rm -f task1
	rm -f task2
	rm -f task3
