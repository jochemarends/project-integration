# author: Jochem Arends
# date: 03-04-2024
# 
# Makefile for building LaTeX documents
# for project integration

BUILD_DIR := build
TEX_FILES := $(wildcard *.tex)

SOURCE_FILE := research.tex
OUTPUT_FILE := $(patsubst %.tex, $(BUILD_DIR)/%.pdf, $(SOURCE_FILE))

LATEX := pdflatex
LATEX_FLAGS := -output-directory=$(BUILD_DIR) -aux-directory=$(BUILD_DIR) -shell-escape

.PHONY: build
build: $(OUTPUT_FILE)

$(OUTPUT_FILE): $(TEX_FILES)
	@mkdir -p $(BUILD_DIR)
	@$(LATEX) $(LATEX_FLAGS) $(SOURCE_FILE)

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)

