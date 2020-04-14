#******************************************************************************
#       ____/    /       / __   __/    ____/                                  *
#      /        /       /      /      /          /       /                    *
#     ___/     /       /      /      ___/   __  __/ __  __/                   *
#    /        /       /      /      /        __/     __/                      *
# _______/ ______/ __/    __/    _______/                                     *
# File: Makefile                                               Part of Elite  *
# Elite Makefile                                                              *
# Copyright (C) 1998 Petya Vachranukunkiet                                    *
#*************************************************************************** */

#compiler name
CC = gcc-3.4
LIBS = #-lcrypt

#compiler flags you want to use (other than profiling, such as -Wall)
MYFLAGS = -g -Wall -Wno-char-subscripts

#flags for profiling (see hacker.doc for more information)
PROFILE =

#Directory to put all the object files
OBJDIR = .

OSFLAGS =

#############################################################################

CFLAGS = $(MYFLAGS) $(PROFILE) $(OSFLAGS)

OBJFILES = comm.o act.comm.o act.informative.o act.movement.o act.obj1.o\
	act.obj2.o act.offensive.o act.other.o act.social.o act.wizard.o\
	handler.o db.o interpreter.o utility.o spec_assign.o shop.o limits.o\
	mobact.o fight.o modify.o weather.o spellconst.o spell_parser.o\
	constants.o spec_procs.o objsave.o boards.o magic.o mail.o history.o\
	castle.o ban.o graph.o config.o gen_cards.o casino.o ocs.o random.o\
	mobcmd.o ptriggers.o parser.o pfunctions.o fparser.o scrcol.o\
	paintball.o ident.o rprogs.o ignore.o

../bin/elite : $(OBJFILES)


utils:
	$(MAKE) ../bin/autowiz
	$(MAKE) ../bin/delobjs
	$(MAKE) ../bin/hasmail
	$(MAKE) ../bin/listrent
	$(MAKE) ../bin/mailindex
	$(MAKE) ../bin/mudpasswd
	$(MAKE) ../bin/purgeplay
	$(MAKE) ../bin/readmaiL
	$(MAKE) ../bin/showplay
	$(MAKE) ../bin/sign
	$(MAKE) ../bin/split
	$(MAKE) ../bin/syntax_check
	$(MAKE) ../bin/changefile
	$(MAKE) ../bin/changeobjfile

all:
	$(MAKE) ../bin/elite
	$(MAKE) utils

elite:
	$(MAKE) ../bin/elite
autowiz:
	$(MAKE) ../bin/autowiz
delobjs:
	$(MAKE) ../bin/delobjs
hasmail:
	$(MAKE) ../bin/hasmail
listrent:
	$(MAKE) ../bin/listrent
mailindex:
	$(MAKE) ../bin/mailindex
mudpasswd:
	$(MAKE) ../bin/mudpasswd
purgeplay:
	$(MAKE) ../bin/purgeplay
readmail:
	$(MAKE) ../bin/readmail
showplay:
	$(MAKE) ../bin/showplay
sign:
	$(MAKE) ../bin/sign
split:
	$(MAKE) ../bin/split
syntax_check:
	$(MAKE) ../bin/syntax_check
changefile:
	$(MAKE) ../bin/changefile
changeobjfile:
	$(MAKE) ../bin/changeobjfile
changewld:
	$(MAKE) ../bin/changewld


../bin/changewld : util/dutil.h
	$(CC) $(CFLAGS) -o ../bin/changewld util/changewld.c util/dutil.c
../bin/autowiz : util/autowiz.c structs.h db.h utils.h
	$(CC) $(CFLAGS) -o ../bin/autowiz util/autowiz.c
../bin/delobjs : util/delobjs.c structs.h
	$(CC) $(CFLAGS) -o ../bin/delobjs util/delobjs.c
../bin/hasmail : util/hasmail.c mail.h
	$(CC) $(CFLAGS) -o ../bin/hasmail util/hasmail.c
../bin/listrent : util/listrent.c structs.h
	$(CC) $(CFLAGS) -o ../bin/listrent util/listrent.c
../bin/mailindex : util/mailindex.c mail.h
	$(CC) $(CFLAGS) -o ../bin/mailindex util/mailindex.c
../bin/mudpasswd   : util/mudpasswd.c structs.h
	$(CC) $(CFLAGS) -o ../bin/mudpasswd util/mudpasswd.c
../bin/purgeplay : util/purgeplay.c structs.h
	$(CC) $(CFLAGS) -o ../bin/purgeplay util/purgeplay.c
../bin/readmail : util/readmail.c mail.h
	$(CC) $(CFLAGS) -o ../bin/readmail util/readmail.c
../bin/showplay : util/showplay.c structs.h
	$(CC) $(CFLAGS) -o ../bin/showplay util/showplay.c
../bin/sign: util/sign.c
	$(CC) $(CFLAGS) -o ../bin/sign $(COMMFLAGS) util/sign.c
../bin/split: util/split.c
	$(CC) $(CFLAGS) -o ../bin/split util/split.c
../bin/syntax_check: util/syntax_check.c structs.h utils.h db.h comm.h \
	handler.h limits.h
	$(CC) $(CFLAGS) -o ../bin/syntax_check util/syntax_check.c
../bin/changefile: util/changefile.c structs.h utils.h db.h comm.h \
	handler.h limits.h
	$(CC) $(CFLAGS) -o ../bin/changefile util/changefile.c
../bin/changeobjfile: util/changeobjfile.c structs.h utils.h db.h comm.h \
	handler.h limits.h
	$(CC) $(CFLAGS) -o ../bin/changeobjfile util/changeobjfile.c

clean:
	rm -f *.o ../bin/*

depend:
	makedepend *.cc *.c -Y.


comm.o : comm.c
	$(CC) -c $(CFLAGS) $(COMMFLAGS) comm.c
act.comm.o : act.comm.c
	$(CC) -c $(CFLAGS) act.comm.c
act.informative.o : act.informative.c
	$(CC) -c $(CFLAGS) act.informative.c
act.movement.o : act.movement.c
	$(CC) -c $(CFLAGS) act.movement.c
act.obj1.o : act.obj1.c
	$(CC) -c $(CFLAGS) act.obj1.c
act.obj2.o : act.obj2.c
	$(CC) -c $(CFLAGS) act.obj2.c
act.offensive.o : act.offensive.c
	$(CC) -c $(CFLAGS) act.offensive.c
act.other.o : act.other.c
	$(CC) -c $(CFLAGS) act.other.c
act.social.o : act.social.c
	$(CC) -c $(CFLAGS) act.social.c
act.wizard.o : act.wizard.c
	$(CC) -c $(CFLAGS) act.wizard.c
handler.o : handler.c
	$(CC) -c $(CFLAGS) handler.c
db.o : db.c
	$(CC) -c $(CFLAGS) db.c
ban.o : ban.c
	$(CC) -c $(CFLAGS) ban.c
interpreter.o : interpreter.c
	$(CC) -c $(CFLAGS) interpreter.c 
utility.o : utility.c
	$(CC) -c $(CFLAGS) utility.c
spec_assign.o : spec_assign.c
	$(CC) -c $(CFLAGS) spec_assign.c
spec_procs.o : spec_procs.c
	$(CC) -c $(CFLAGS) spec_procs.c
castle.o : castle.c
	$(CC) -c $(CFLAGS) castle.c
limits.o : limits.c
	$(CC) -c $(CFLAGS) limits.c
fight.o : fight.c
	$(CC) -c $(CFLAGS) fight.c
weather.o : weather.c
	$(CC) -c $(CFLAGS) weather.c
shop.o : shop.c
	$(CC) -c $(CFLAGS) shop.c
magic.o : magic.c
	$(CC) -c $(CFLAGS) magic.c 
spellconst.o : spellconst.c
	$(CC) -c $(CFLAGS) spellconst.c
spell_parser.o : spell_parser.c
	$(CC) -c $(CFLAGS) spell_parser.c 
mobact.o : mobact.c
	$(CC) -c $(CFLAGS) mobact.c
modify.o : modify.c
	$(CC) -c $(CFLAGS) modify.c
constants.o : constants.c
	$(CC) -c $(CFLAGS) constants.c
objsave.o : objsave.c
	$(CC) -c $(CFLAGS) objsave.c
boards.o : boards.c
	$(CC) -c $(CFLAGS) boards.c
mail.o : mail.c
	$(CC) -c $(CFLAGS) mail.c
graph.o : graph.c
	$(CC) -c $(CFLAGS) graph.c
config.o : config.c
	$(CC) -c $(CFLAGS) config.c
gen_cards.o: gen_cards.c
	$(CC) -c $(CFLAGS) gen_cards.c
ocs.o   : ocs.c
	$(CC) -c $(CFLAGS) ocs.c
casino.o: casino.c
	$(CC) -c $(CFLAGS) casino.c
mobcmd.o: mobcmd.c
	$(CC) -c $(CFLAGS) mobcmd.c
ptriggers.o: ptriggers.c
	$(CC) -c $(CFLAGS) ptriggers.c
parser.o: parser.c
	$(CC) -c $(CFLAGS) parser.c
pfunctions.o: pfunctions.c
	$(CC) -c $(CFLAGS) pfunctions.c
fparser.o: fparser.c
	$(CC) -c $(CFLAGS) fparser.c
random.o: random.c
	$(CC) -c $(CFLAGS) random.c
history.o: history.c
	$(CC) -c $(CFLAGS) history.c
paintball.o: paintball.c
	$(CC) -c $(CFLAGS) paintball.c
scrcol.o: scrcol.c
	$(CC) -c $(CFLAGS) scrcol.c
ident.o: ident.c
	$(CC) -c $(CFLAGS) ident.c
rprogs.o: rprogs.c
	$(CC) -c $(CFLAGS) rprogs.c
ignore.o: rprogs.c
	$(CC) -c $(CFLAGS) ignore.c

../bin/elite :
#	$(CC) -o ../../DIKU/elite/bin/elite $(PROFILE) $(OBJFILES) $(LIBS)
#	$(CC) -o ../../../temp/elite $(PROFILE) $(OBJFILES) $(LIBS)
#	$(CC) -o ../../../inda/matlab $(PROFILE) $(OBJFILES) $(LIBS)
	$(CC) -o ../bin/elite $(PROFILE) $(OBJFILES) $(LIBS)

# Dependencies for the main mud
# DO NOT DELETE

act.comm.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h interpreter.h
act.comm.o: handler.h db.h screen.h functions.h history.h ignore.h
act.informative.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h
act.informative.o: interpreter.h handler.h db.h spells.h screen.h functions.h
act.informative.o: scrcol.h
act.movement.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h
act.movement.o: interpreter.h handler.h db.h spells.h functions.h
act.obj1.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h interpreter.h
act.obj1.o: handler.h db.h spells.h functions.h
act.obj2.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h interpreter.h
act.obj2.o: handler.h db.h spells.h functions.h
act.offensive.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h
act.offensive.o: interpreter.h handler.h db.h spells.h functions.h ignore.h
act.other.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h ignore.h
act.other.o: interpreter.h handler.h db.h spells.h screen.h functions.h
act.social.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h ignore.h
act.social.o: interpreter.h handler.h db.h spells.h functions.h history.h
act.wizard.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h
act.wizard.o: interpreter.h handler.h db.h spells.h screen.h functions.h
act.wizard.o: ocs.h history.h
ban.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h interpreter.h
ban.o: handler.h db.h functions.h
boards.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h db.h boards.h
boards.o: interpreter.h handler.h functions.h
casino.o: conf.h sysdep.h ./limits.h structs.h utils.h db.h gen_cards.h
casino.o: comm.h handler.h interpreter.h functions.h
castle.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h interpreter.h
castle.o: handler.h db.h spells.h functions.h
comm.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h interpreter.h
comm.o: handler.h db.h screen.h functions.h scrcol.h history.h
config.o: structs.h
constants.o: structs.h ./limits.h spells.h
cstore.o: conf.h sysdep.h ./limits.h structs.h comm.h handler.h db.h
cstore.o: interpreter.h utils.h functions.h cstore.h
db.o: conf.h sysdep.h ./limits.h structs.h utils.h db.h comm.h handler.h
db.o: spells.h mail.h interpreter.h ocs.h functions.h shop.h
fight.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h handler.h
fight.o: interpreter.h db.h spells.h screen.h functions.h
fparser.o: fparser.h conf.h sysdep.h ./limits.h
gen_cards.o: conf.h sysdep.h ./limits.h structs.h utils.h gen_cards.h comm.h
gen_cards.o: handler.h interpreter.h db.h functions.h
graph.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h interpreter.h
graph.o: handler.h db.h spells.h functions.h
handler.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h db.h handler.h
handler.o: interpreter.h functions.h
history.o: conf.h sysdep.h ./limits.h structs.h utils.h history.h
ident.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h db.h ident.h
interpreter.o: conf.h sysdep.h ./limits.h structs.h comm.h interpreter.h db.h
interpreter.o: utils.h spells.h handler.h mail.h screen.h objsave.h
interpreter.o: functions.h ignore.h
limits.o: conf.h sysdep.h ./limits.h structs.h utils.h spells.h comm.h db.h
limits.o: handler.h functions.h
magic.o: structs.h utils.h comm.h spells.h handler.h ./limits.h interpreter.h
magic.o: db.h functions.h sysdep.h conf.h
mail.o: conf.h sysdep.h ./limits.h mail.h structs.h utils.h comm.h db.h
mail.o: interpreter.h handler.h objsave.h functions.h
mobact.o: conf.h sysdep.h ./limits.h structs.h utils.h db.h comm.h
mobact.o: interpreter.h handler.h functions.h
mobcmd.o: conf.h sysdep.h ./limits.h structs.h db.h utils.h handler.h
mobcmd.o: interpreter.h comm.h functions.h
modify.o: conf.h sysdep.h ./limits.h structs.h utils.h interpreter.h scrcol.h
modify.o: handler.h db.h comm.h mail.h screen.h spells.h functions.h
objsave.o: conf.h sysdep.h ./limits.h structs.h comm.h handler.h db.h
objsave.o: interpreter.h utils.h spells.h objsave.h fparser.h functions.h
ocs.o: conf.h sysdep.h ./limits.h structs.h utils.h ocs.h db.h interpreter.h
ocs.o: functions.h
paintball.o: conf.h sysdep.h structs.h utils.h db.h
parser.o: conf.h sysdep.h ./limits.h structs.h utils.h functions.h parser.h
parser.o: db.h interpreter.h
pfunctions.o: conf.h sysdep.h ./limits.h structs.h utils.h parser.h comm.h
pfunctions.o: db.h interpreter.h functions.h
ptriggers.o: conf.h sysdep.h ./limits.h structs.h utils.h interpreter.h
ptriggers.o: handler.h db.h parser.h functions.h
random.o: conf.h sysdep.h ./limits.h
rprogs.o: conf.h sysdep.h structs.h utils.h db.h functions.h interpreter.h
rprogs.o: comm.h
scrcol.o: conf.h sysdep.h scrcol.h
shop.o: conf.h sysdep.h ./limits.h structs.h comm.h handler.h db.h
shop.o: interpreter.h utils.h functions.h shop.h
spec_assign.o: conf.h sysdep.h ./limits.h structs.h db.h interpreter.h
spec_assign.o: utils.h
spec_procs.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h
spec_procs.o: interpreter.h handler.h db.h spells.h functions.h
spell_parser.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h db.h
spell_parser.o: interpreter.h spells.h handler.h functions.h
spellconst.o: structs.h ./limits.h
utility.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h screen.h
utility.o: spells.h functions.h
weather.o: conf.h sysdep.h ./limits.h structs.h utils.h comm.h handler.h
weather.o: interpreter.h db.h functions.h
