CXXFLAGS += -static -Wl,--start-group -Wl,--whole-archive -lrlottie -lapngasm -lboost_filesystem -lboost_regex -Wl,--end-group -Wl,-rpath-link=/usr/local/lib
all: tgs2apng
clean:
	$(RM) tgs2apng
