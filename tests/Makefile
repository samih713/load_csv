NAME:= run_tests

CXXFLAGS:= -std=c++20 -ggdb3 -fsanitize=address -O1 \
           -DFIXTURE_DIR='"$(CURDIR)/data/"'

all:
	c++ $(CXXFLAGS) test_cxv.cpp -o $(NAME) -lCatch2Main -lCatch2

run: all
	./$(NAME)

# known-bug tests, expected to crash (see test_cxv.cpp)
bugs: all
	./$(NAME) "[bug]"

clean:
	rm -f $(NAME)

re: clean all
