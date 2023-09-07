# doc/Makefile

DOCS :=
DOCS += ASTsForTemplates.rst
DOCS += InternalsManual.rst
DOCS += IntroductionToTheClangAST.rst
DOCS += index.rst

IMAGES := $(wildcard ASTsForTemplatesImages/*.png)

html/index.html: $(DOCS) $(IMAGES) conf.py
	mkdir -p html
	sphinx-build . html

clean:
	$(RM) -r html

# EOF
