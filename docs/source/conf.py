# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information
import sphinx_rtd_theme
import os

project = 'gamepadAPI'
copyright = '2025, DevelTeam'
author = 'DevelTeam'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "breathe",
    "sphinx_copybutton"
]

breathe_projects = {
    "DevelDeck-docs": os.path.abspath('../doxygen/xml'),
}

breathe_default_project = "DevelDeck-docs"

templates_path = ['_templates']
exclude_patterns = []

pygments_style = "pastie"



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output
html_theme = "sphinx_rtd_theme"
html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]
html_static_path = ["_static"]
html_css_files = ["css/extra.css"]
templates_path = ["_templates"]

html_logo = "_static/images/DevelDeck.png"
html_theme_options = {
    'logo_only': False, # Set to True if you don't want the project name text
    'display_version': True,
}

# Optional: set theme options for nicer defaults
html_theme_options = {
    "navigation_depth": 4,
    "collapse_navigation": False,
    "style_external_links": True,
    'titles_only': False
}

master_doc = "index"

html_static_path = ['_static']