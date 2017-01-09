from distutils.core import setup

setup(name='harry',
	  description="""Harry - A Tool for Measuring String Similarity""",
	  version='0.1',
	  url='https://github.com/rieck/harry',
	  packages=['harry'],
	  package_dir={'': 'bindings/python'},
)
