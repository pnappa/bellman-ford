
.PHONY: clean

bellmanford: bellmanford.cpp
	g++ -std=c++11 -O3 bellmanford.cpp -o bellmanford

clean: 
	rm -vf bellmanford
