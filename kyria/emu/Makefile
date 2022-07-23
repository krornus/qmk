include config.mk

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(DEP)

$(OBJ): config.mk

.PHONY: clean
clean:
	$(RM) -rf $(OBJ) $(DEP) $(TARGET)
