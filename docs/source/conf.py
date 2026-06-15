# Configuration file for the Sphinx documentation builder.

# -- Project information -----------------------------------------------------
import sphinx_rtd_theme
import os

project = 'DevelDeck-API'
copyright = '2025, DevelTeam'
author = 'DevelTeam'

# -- General configuration ---------------------------------------------------

extensions = [
    "breathe",
    "sphinx_copybutton",
    "sphinx_togglebutton"
]

breathe_projects = {
    "DevelDeck-docs": os.path.abspath('../doxygen/xml'),
    "TFT_eSPI-docs": os.path.abspath('../doxygen/TFT_eSPI/xml')
}
breathe_default_project = "DevelDeck-docs"

templates_path = ['_templates']
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
html_theme = "sphinx_rtd_theme"
html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]
html_static_path = ["_static"]
html_css_files = ["css/custom.css", "pygments.css"]
templates_path = ["_templates"]

html_logo = "_static/images/DevelDeck.png"


html_theme_options = {
    "navigation_depth": 4,
    "collapse_navigation": False,
    "style_external_links": True,
    'titles_only': False,
    'logo_only': False,
    'display_version': True,
    'prev_next_buttons_location': 'bottom',
}

master_doc = "index"

html_static_path = ['_static']