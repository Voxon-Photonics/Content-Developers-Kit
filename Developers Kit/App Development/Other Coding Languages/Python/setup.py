from setuptools import setup, find_packages
import codecs
import os

VERSION = '0.0.1'
DESCRIPTION = "Python wrapper for developing applications for Voxon's volumetric display"
LONG_DESCRIPTION = 'A package that allows to developers to build Voxon applications in Python.'
 
 # Setting up
setup(
     name="voxiePy",
     version=VERSION,
     author="Matthew Vecchio (ReadyWolf)",
     author_email="<matt@voxon.co>",
     description=DESCRIPTION,
     long_description_content_type="text/markdown",
     long_description=LONG_DESCRIPTION,
     packages=find_packages(),
     install_requires=[],
     keywords=['python', 'voxon', 'volumetric', 'holographic', 'graphic', 'vx1'],
     classifiers=[
         "Development Status :: 3 - Alpha",
         "Intended Audience :: Developers",
         "Programming Language :: Python :: 3",
         "Operating System :: Microsoft :: Windows",
     ]
)