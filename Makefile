.PHONY: all clean

all:
	@$(MAKE) --no-print-directory -C src

%:
	@$(MAKE) --no-print-directory -C src $@