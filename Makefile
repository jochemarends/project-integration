BUILD_DIR := build
TEX_FILES := $(wildcard *.tex)
PDF_FILES := $(patsubst %.tex, $(BUILD_DIR)/%.pdf, $(TEX_FILES))

.PHONY: all
all: $(PDF_FILES)

build/%.pdf : %.tex
	mkdir -p $(BUILD_DIR)
	pdflatex -output-directory=$(BUILD_DIR) $<
# check for BibTeX files
ifneq ($(wildcard $*.bib), "")
	TEXMFOUTPUT="$(BUILD_DIR)" bibtex $(BUILD_DIR)/$*
	pdflatex -output-directory=$(BUILD_DIR) $<
	pdflatex -output-directory=$(BUILD_DIR) $<
endif


.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)

