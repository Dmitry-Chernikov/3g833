doc: clean-doc
	doxygen
	cd docs/html/ && python -m http.server 8000

clean-doc:
	-rm -rf docs/html/*