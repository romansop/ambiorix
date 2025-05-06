VALGRIND = valgrind --leak-check=full --exit-on-first-error=yes --error-exitcode=1 --gen-suppressions=all --suppressions=../valgrind.supp --keep-debuginfo=yes

all: $(TARGET)

run: $(TARGET)
	set -o pipefail; $(VALGRIND) ./$< 2>&1 | tee -a $(OBJDIR)/unit_test_results.txt;

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

-include $(OBJECTS:.o=.d)

$(OBJDIR)/%.o: ./%.c | $(OBJDIR)/
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $(<)

$(OBJDIR)/%.o: ../common/src/%.c | $(OBJDIR)/
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $(<)

$(OBJDIR)/:
	mkdir -p $@

clean:
	! rm -f $(TARGET) vgcore.* $(OBJDIR)

.PHONY: clean $(OBJDIR)/
