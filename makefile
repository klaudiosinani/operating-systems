PREFIX = ./src/
SUBDIRS = $(PREFIX)assignment-1/part-b $(PREFIX)assignment-2 \
			$(PREFIX)assignment-3 $(PREFIX)assignment-4

.PHONY: subdirs $(SUBDIRS) test

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

$(PREFIX)assignment-4: $(PREFIX)assignment-3

assignment-1:
	$(MAKE) -C $(PREFIX)assignment-1/part-b

assignment-2:
	$(MAKE) -C $(PREFIX)assignment-2

assignment-3:
	$(MAKE) -C $(PREFIX)assignment-3

assignment-4:
	$(MAKE) -C $(PREFIX)assignment-4

test:
	$(MAKE) test -C $(PREFIX)assignment-1/part-b

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) clean -C $$dir; \
	done
