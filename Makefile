LLVM_VERSION:=-10
CXX := clang++$(LLVM_VERSION)
LLVM_CONFIG := llvm-config$(LLVM_VERSION)
CXXFLAGS := `$(LLVM_CONFIG) --cxxflags` -Wall -g -std=c++17
LDFLAGS := `$(LLVM_CONFIG) --ldflags --libs core`
STYLE := "{BasedOnStyle: llvm, AllowShortFunctionsOnASingleLine: false, IndentWidth: 4, ColumnLimit: 150, BreakBeforeBraces: Custom, BraceWrapping: {BeforeElse: true}}"
PROJECT := codegen

$(PROJECT): $(PROJECT).cpp x86.cpp x86.hpp .format_$(PROJECT).cpp .format_x86.cpp .format_x86.hpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PROJECT).cpp x86.cpp -o $@

.format_%: %
	clang-format -i -style=$(STYLE) $^ && touch $@ || echo "clang-format isn't installed..."

.PHONY: clean
clean:
	rm -f a.out *.o $(PROJECT) .format_*
