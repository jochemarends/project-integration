# author: Jochem Arends
# date: 06-03-2024
# 
# Makefile for building LaTeX documents
# for project integration

BUILD_DIR := build
TEX_FILES := $(wildcard *.tex)

SOURCE_FILE := research.tex
OUTPUT_FILE := $(patsubst %.tex, $(BUILD_DIR)/%.pdf, $(SOURCE_FILE))

LATEX := pdflatex
LATEX_FLAGS := -output-directory=$(BUILD_DIR) -shell-escape

GLOSSARY := makeglossaries
GLOSSARY_FLAGS := -d $(BUILD_DIR)

.PHONY: build
build: $(OUTPUT_FILE)

$(OUTPUT_FILE): $(TEX_FILES)
	@mkdir -p $(BUILD_DIR)
	@$(LATEX) $(LATEX_FLAGS) $(SOURCE_FILE)

	@$(GLOSSARY) $(GLOSSARY_FLAGS) $(basename $(SOURCE_FILE))

	@TEXMFOUTPUT=$(BUILD_DIR)
	@bibtex	$(BUILD_DIR)/$(basename $(SOURCE_FILE))

	@$(LATEX) $(LATEX_FLAGS) $(SOURCE_FILE)
	@$(LATEX) $(LATEX_FLAGS) $(SOURCE_FILE)
	@cp -f $(OUTPUT_FILE) .

.PHONY: open
open:
	@wslview $(OUTPUT_FILE)

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)

