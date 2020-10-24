libraries += libutil

libutil_NAME = libnixutil

libutil_DIR := $(d)

libutil_SOURCES := $(wildcard $(d)/*.cc)

libutil_LDFLAGS = $(LIBLZMA_LIBS) -lbz2 -pthread $(OPENSSL_LIBS) $(LIBBROTLI_LIBS) $(LIBARCHIVE_LIBS) $(BOOST_LDFLAGS)

ifeq (1,$(HAVE_BOOST_CONTEXT))
    libutil_LDFLAGS += -lboost_context
endif
