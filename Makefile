# author: Jochem Arends
# date: 13-02-2024
# 
# a Makefile for building LaTeX documents
# for project integration

BUILD_DIR := build
TEX_FILES := $(wildcard *.tex)
BIB_FILES := $(wildcard *.bib)
PDF_FILES := $(patsubst %.tex, $(BUILD_DIR)/%.pdf, $(TEX_FILES))

.PHONY: all
all: $(PDF_FILES)

# make sure a build will happen after modifying
# .bib files
$(PDF_FILES): $(BIB_FILES)

$(BUILD_DIR)/%.pdf: %.tex
	echo $^
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

