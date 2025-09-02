# Lets just hope this doesn't get evaluated every time..
CC 	= g++

#UnComment this line if your getting errors about deflate and such
MCCPLIB = -lz

#UnComment this line if your getting errors about crypt and such
CRYPT = -lcrypt

OS=$(shell uname | cut -d _ -f 1)
#Uncomment this line if your compiling on a windows system
ifeq (${OS},CYGWIN)
WIN=1
endif

ifndef WIN 
DYNLIB = "-ldl"
endif

#uncomment this line if you want webpage export
WEB = -DWEB

#IMC2 - Comment out to disable IMC2 support
IMC = 1
IMCFLAGS = -DIMC -DIMCSMAUG

#Shouldn't need to touch this...
DLLEXP = -export-dynamic -rdynamic

INDENT  = indent
INDENT_FLAGS = -ts4 -nut -bap -bli0 -di10 -l78 -lp -nbc -i8 -sc -cdb -c1 -cd1 -nbfda -npcs -bad -hnl -nce -npsl -bl

#Uncomment the next line if you want request support
#DBUGFLG = -DREQUESTS
DEFINES = -DMCCP -DMXP -DACCOUNT -DOLC_HOMES -DRESTORE -DOLC_SHUTTLE $(WEB)
DEBUG   = -g3

#Miscellaneous compiler options.
ifndef DEBUG
OPT_FLAG = -pipe -Os
endif

C_FLAGS = $(DEFINES) $(IMCFLAGS) $(DBUGFLG) $(DEBUG) $(DLLEXP) -fpermissive -Wno-write-strings
L_FLAGS = $(DYNLIB) $(MCCPLIB) $(CRYPT) -largon2 -lm

ifndef DEBUG
W_FLAGS = -pedantic -W -Wall -Wformat-security -Winline -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-align -Wcast-qual -Wredundant-decls -Wformat-security
endif

CPP_FILES = body.cpp account.cpp act_comm.cpp act_info.cpp act_move.cpp act_obj.cpp \
            act_wiz.cpp alias.cpp arena.cpp autobuild.cpp ban.cpp bank.cpp bet.cpp \
            boards.cpp bounty.cpp build.cpp changes.cpp channels.cpp clans.cpp cleanup.cpp color.cpp combat.cpp \
            comm.cpp comments.cpp const.cpp db.cpp delivery.cpp designship.cpp dns.cpp editor.cpp \
            fight.cpp finger.cpp grid_c.cpp handler.cpp hashstr.cpp homes.cpp hotboot.cpp immcomm.cpp \
            implants.cpp installations.cpp interp.cpp magic.cpp makeobjs.cpp mccp.cpp \
            medic.cpp misc.cpp msp.cpp mud_comm.cpp mud_prog.cpp mxp.cpp occupations.cpp olc_bounty.cpp\
            olc-shuttle.cpp olc.cpp password.cpp pfiles.cpp pilot.cpp player.cpp quest.cpp races.cpp raceskills.cpp \
            renumber.cpp reset.cpp restore.cpp save.cpp shell.cpp shops.cpp  \
            skills.cpp smuggling.cpp space.cpp space2.cpp special.cpp starsystem.cpp swskills.cpp \
            tables.cpp track.cpp update.cpp vendor.cpp wedding.cpp grid.cpp greet.cpp \
			imccustom.cpp \

O_FILES = $(patsubst %.cpp,o/%.o,$(CPP_FILES))

H_FILES = $(wildcard *.h) 

ifdef IMC
   CPP_FILES :=  $(CPP_FILES) imc.cpp md5.cpp
endif

ifdef WEB
   CPP_FILES := $(CPP_FILES) web-server.cpp 
endif

ifdef WIN
   SWR = swr.exe
   RESOLVER = resolver.exe
else
   SWR = swr
   RESOLVER = resolver
endif

#all: .depend
all:
	@make -s swr

dns:
	make -s resolver

swr: $(O_FILES)
	@echo "Making Executable |$(OS)|$(WIN)|"
	@rm -f swr
ifdef WIN
	@dlltool --export-all --output-def swr.def $(O_FILES)
	@dlltool --dllname $(SWR) --output-exp swr.exp --def swr.def
	@$(CC)  $(W_FLAGS) -o $(SWR) $(O_FILES) swr.exp $(L_FLAGS)
else
	@$(CC)  $(DLLEXP) $(W_FLAGS) -o $(SWR) $(O_FILES) $(L_FLAGS)
endif
	@[ -f /usr/bin/bf ] && /usr/bin/bf -s swr && /usr/bin/bf -m swr || true
	@chmod g+w $(SWR)
	@chmod g+w $(O_FILES)

cvs:
	@echo "Updating from CVS"
	@cvs update

svn:
	@echo "Updating from SVN"
	@~/bin/svn update
	
clean:
	@echo Cleaning Files
	@rm -f $(O_FILES)
	@rm -f o/resolver.o


resolver: o/resolver.o
	@rm -f $(RESOLVER)
	@echo Building Resolver
	@$(CC) $(DEFINES) -o $(RESOLVER) o/resolver.o
	@echo Done compiling resolver
	@chmod g+w $(RESOLVER)
	@chmod a+x $(RESOLVER)
	@chmod g+w o/resolver.o

neat:
	$(INDENT) $(INDENT_FLAGS) $(C_FILES) $(H_FILES)

indentclean:
	rm *.cpp~ *.h~

o/%.o: %.cpp mud.h
	@echo -n "  Compiling $@...";
	$(CC) -c $(C_FLAGS) $< -o $@
	@echo "Done"

# Modern pattern rule instead of old-style suffix rule
%.o: %.cpp mud.h
	@$(CC) -c $(C_FLAGS) $< -o $@
	
.depend:  $(CPP_FILES)
	@echo Making Depend
	@rm -f .depend
# For each source, let the compiler run the preprocessor with the -M and -MM
# options, which causes it to output a dependency suitable for make.
	@for source in $(CPP_FILES); do \
		$(CC) $(DEFINES) $(INCLUDEDIR) $(C_FLAGS) -M -MM -MT o/`basename $$source .cpp`.o $$source >> .depend ; \
	done

DEPENDEXISTS=$(shell if [ -e .depend ]; then echo -n "1"; fi )
ifdef (DEPENDEXISTS) 
include .depend
endif
