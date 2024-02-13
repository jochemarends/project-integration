# author: Jochem Arends
# date: 13-02-2024
# 
# a Makefile for building LaTeX documents
# for project integration

BUILD_DIR := build
TEX_FILES := $(wildcard *.tex)
PDF_FILES := $(patsubst %.tex, $(BUILD_DIR)/%.pdf, $(TEX_FILES))

.PHONY: all
all: $(PDF_FILES)

$(BUILD_DIR)/%.pdf: %.tex $(wildcard %.bib)
	mkdir -p $(BUILD_DIR)
	pdflatex -output-directory=$(BUILD_DIR) $<
# check for BibTeX files
# make sure the .bib file has the same extension
# as the .tex file!
ifneq ($(wildcard %.bib), "")
	TEXMFOUTPUT=$(BUILD_DIR)
	bibtex $(BUILD_DIR)/$*
	pdflatex -output-directory=$(BUILD_DIR) $<
	pdflatex -output-directory=$(BUILD_DIR) $<
endif

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)

