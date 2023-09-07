# doc/Makefile

DOCS :=
DOCS += ASTsForTemplates.rst
DOCS += InternalsManual.rst
DOCS += IntroductionToTheClangAST.rst
DOCS += index.rst

html/index.html: $(DOCS) conf.py
	mkdir -p html
	sphinx-build . html

clean:
	$(RM) -r html

# EOF
