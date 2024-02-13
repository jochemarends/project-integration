BUILD_DIR := build

$(BUILD_DIR)/*.pdf: *.tex
	mkdir -p $(BUILD_DIR)
	pdflatex -output-directory=$(BUILD_DIR) $<
	cp $< $(BUILD_DIR)
	cp *.bib $(BUILD_DIR)
	bibtex $(BUILD_DIR)/$*
	pdflatex $(BUILD_DIR)/$*
	pdflatex $(BUILD_DIR)/$*

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)

