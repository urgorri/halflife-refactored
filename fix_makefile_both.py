with open('linux/Makefile.hldll', 'r') as f:
    content = f.read()

target1 = r"""$(HLDLL_OBJ_DIR)/weapons/%.o: $(HLDLL_SRC_DIR)/weapons/%.cpp $(filter-out $(wildcard  $(HLDLL_OBJ_DIR)/weapons),  $(HLDLL_OBJ_DIR)/weapons)
	$(DO_HLDLL_CC)"""
# wait, Makefiles must use tabs.
content = content.replace('$(HLDLL_OBJ_DIR)/weapons/%.o : $(HLDLL_SRC_DIR)/weapons/%.cpp $(filter-out $(wildcard  $(HLDLL_OBJ_DIR)/weapons),  $(HLDLL_OBJ_DIR)/weapons)\n\t$(DO_HLDLL_CC)',
                          '$(HLDLL_OBJ_DIR)/weapons/%.o : $(HLDLL_SRC_DIR)/weapons/%.cpp $(filter-out $(wildcard  $(HLDLL_OBJ_DIR)/weapons),  $(HLDLL_OBJ_DIR)/weapons)\n\t$(DO_HLDLL_CC)\n')

with open('linux/Makefile.hldll', 'w') as f:
    f.write(content)
