# Clue Makefile

PROJECT = clue
VERSION = 7
PROJECT_VERSIONED = $(PROJECT)-$(VERSION)
DISTFILE = $(PROJECT_VERSIONED).zip


check:
	$(CC) -I/usr/include/lua5.1 -o clue_test clue_test.c clue.c -llua5.1
	./clue_test

dist:
	cd .. && rm -f $(DISTFILE) && zip $(DISTFILE) -r $(PROJECT) -x "*.o" "*.zip" clue/clue_test "clue/odds/*" && mv $(DISTFILE) $(PROJECT) && cd $(PROJECT) && unzip $(DISTFILE) && mv $(PROJECT) $(PROJECT_VERSIONED) && rm -f $(DISTFILE) && zip $(DISTFILE) -r $(PROJECT_VERSIONED) && rm -rf $(PROJECT_VERSIONED)
