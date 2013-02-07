UNIT_TEST_OBJS := \
	src/unit_test/ares/bin_util.o \
	src/unit_test/ares/bytes.o \
	src/unit_test/ares/date.o \
	src/unit_test/ares/date_util.o \
	src/unit_test/ares/hashtable.o \
	src/unit_test/ares/main.o \
	src/unit_test/ares/message_reader.o \
	src/unit_test/ares/message_writer.o \
	src/unit_test/ares/queue_sink.o \
	src/unit_test/ares/string_tokenizer.o \
	src/unit_test/ares/string_util.o \
	src/unit_test/ares/sync_queue.o \
	src/unit_test/ares/test_sink.o

test: bin/run_unit_tests
	./bin/run_unit_tests

bin/run_unit_tests: CPPFLAGS += -I$(CPPUNIT)/include
bin/run_unit_tests: $(UNIT_TEST_OBJS) $(LIB_NAME)
	$(CC) -o $@ $(UNIT_TEST_OBJS) \
		-lares -lcppunit -ldl $(LIBS) \
		-L$(CPPUNIT)/lib -Llib

clean: clean_unit_test

clean_unit_test:
	-rm -f $(UNIT_TEST_OBJS) bin/run_unit_tests

depend: depend_unit_test

depend_unit_test:
	$(CC) $(CPPFLAGS) -MM `find src/unit_test/ares -name \*.cpp` \
		2>/dev/null | cat - > build/deps_unit_test.mk

include build/deps_unit_test.mk
