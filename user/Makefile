PROGRAMS := minimum-user

.PHONY: all lib programs clean $(PROGRAMS)

all: lib programs

lib:
	$(MAKE) -C lib all

programs: $(PROGRAMS)

$(PROGRAMS):
	$(MAKE) -C prog/$@ all

clean:
	$(MAKE) -C lib clean
	@for program in $(PROGRAMS); do $(MAKE) -C prog/$$program clean; done
