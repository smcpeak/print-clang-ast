# conf.py
# Configuration file for the Sphinx documentation builder.

# Call it "clang" in the upper left navigation area.  (Well, I've now
# disabled the sidebar, so maybe this does not appear anywhere.)
project = 'clang'

# Match the style of InternalsManual.html.
#
# but it is so ugly...
#html_theme = "haiku"

# Overriding some things in the Alabaster theme.
html_static_path = ["custom.css"]

# This affects the syntax highlighting.
pygments_style = "friendly"

# Don't append anything to the page title.
html_title = ''

# Get rid of the sidebar.
html_sidebars = {
    '**': []
}

# Get rid of the footer.
html_show_copyright = False
html_show_sourcelink = False

# EOF
