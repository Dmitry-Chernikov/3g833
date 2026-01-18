doc:
	-doc --rf docs/*
	doxygen
	cd docs/html/ && python -m http.server 8000