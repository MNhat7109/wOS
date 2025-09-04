SHARED_COLORSTART = \e[36m
SHARED_COLOREND = \e[0m

SRC_DIR = $(shell pwd)
BUILD_DIR = $(shell echo $(subst src,build, $(SRC_DIR)) | sed 's/^[ ]*//')

LIB_DIRS = $(filter %/,$(wildcard $(SRC_DIR)/*/))
LIBS = $(patsubst $(SRC_DIR)/%, $(SHARED_DIR)/%.a, $(LIB_DIRS))

.PHONY: all

all: $(LIBS)

$(SHARED_DIR)/%.a: $(SRC_DIR)/%
	@printf "$(SHARED_COLORSTART)[SHARED]$(SHARED_COLOREND): Entering $^\n"
	$(MAKE) -C $^
