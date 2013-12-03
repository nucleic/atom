# Note: This is meant for Atom developers, not for end users
#       To install Atom, please use setup.py

.PHONY: all clean test cover

all:  
	make clean
	pip install -e .

clean:
	rm -rf build
	find . -name "*.pyc" -o -name "*.py,cover" | xargs rm -f

test: 
	python -m py.test
	python setup.py check -r
	
cover: 
	coverage run --source atom -m py.test
	coverage report

release:
	rm -rf dist
	python setup.py register
	python setup.py sdist --formats=gztar,zip upload

gh-pages:
	#git checkout master
	#git pull origin master
	rm -rf ../atom_docs
	mkdir ../atom_docs
	rm -rf docs/build
	-make -C docs html
	cp -R docs/build/html/ ../atom_docs
	mv ../enaml_docs/html ../atom_docs/docs
	git checkout gh-pages
	rm -rf docs
	cp -R ../atom_docs/docs/ .
	git commit -a -m "rebuild docs"
	git push upstream-rw gh-pages
	rm -rf ../atom_docs
	#git checkout master
	#rm docs/.buildinfo
