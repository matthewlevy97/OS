.PHONY: setup_env build debug run clean

all: setup_env build

setup_env:
	./setup_env.sh

build:
	./scripts/build.sh

debug: all
	./scripts/debug.sh

run: all
	./scripts/run_qemu.sh

clean:
	./scripts/clean.sh
