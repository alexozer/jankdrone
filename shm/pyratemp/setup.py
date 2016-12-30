#!/usr/bin/env python
# -*- coding: utf-8 -*-

from distutils.core import setup

setup(name='pyratemp',
      version='0.3.2',
      description="Small, simple and powerful template-engine for Python.""",
      author="Roland Koebler",
      author_email="rk@simple-is-better.org",
      url="http://www.simple-is-better.org/template/pyratemp.html",
      download_url='http://www.simple-is-better.org/template/pyratemp-0.3.2.tgz',

      py_modules=['pyratemp'],
      scripts=['pyratemp_tool.py', 'tools.py'],

      platforms = ["any"],
      license="MIT",
      classifiers=[
          "Development Status :: 5 - Production/Stable",
          "Environment :: Console",
          "Environment :: Web Environment",
          "Intended Audience :: Developers",
          "License :: OSI Approved :: MIT License",
          "Natural Language :: English",
          "Operating System :: OS Independent",
          "Programming Language :: Python",
          "Programming Language :: Python :: 2",
          "Programming Language :: Python :: 2.6",
          "Programming Language :: Python :: 2.7",
          "Programming Language :: Python :: 3",
          "Topic :: Software Development :: Libraries :: Python Modules",
          "Topic :: Text Processing",
          "Topic :: Text Processing :: Markup",
          "Topic :: Text Processing :: Markup :: HTML",
          "Topic :: Text Processing :: Markup :: LaTeX",
          "Topic :: Internet :: WWW/HTTP :: Dynamic Content",
          ]

      )

