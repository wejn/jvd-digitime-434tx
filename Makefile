all: run

run:
	pio run

flash: upload

upload:
	pio run -t upload

watch:
	pio device monitor

clean:
	pio run -t clean
