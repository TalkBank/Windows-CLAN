# CLAN has to be 32bit app. Please use -m32 option when compiling CLAN.

DD  = ../unix/bin
OBJ = ../unix/obj

# for most OS:
	CC = g++

# for Apple MacOS 11 Big Sur and later, uncomment next one
#	CFLAGS = -O -DUNX -DAPPLEUNX -fwritable-strings -fshort-wchar -Wcomment -Wno-deprecated-declarations -Wempty-body

# for Apple prior to MacOS 11 Big Sur, uncomment next one
#	CFLAGS = -O -DUNX -DAPPLEUNX -fwritable-strings -fshort-wchar -Wcomment -Wno-deprecated-declarations -Wempty-body -arch i386

# for Ubuntu 20.04.6, uncomment next one
#	CFLAGS = -O -DUNX -Wno-deprecated -Wno-deprecated-declarations -Wno-narrowing

# for Ubuntu 20.04.6 Browser Server, uncomment next one
#	CFLAGS = -O -DCLAN_SRV -DUNX -Wno-deprecated -Wno-deprecated-declarations -Wno-narrowing

# for Apple MacOS 11 Big Sur and later Browser Server, uncomment next one
#	CFLAGS = -O -DCLAN_SRV -DUNX -DAPPLEUNX -fwritable-strings -fshort-wchar -Wcomment -Wno-deprecated-declarations -Wempty-body


# do not uncomment this line: -m32

# for SUN SPARC station:
#	CFLAGS = -O2 -DUNX -fwritable-strings
# for SunOS sparc SUNW, Ultra-5_10
#	CFLAGS = -O2 -DUNX -DERRNO -fwritable-strings
# Linux on Intel and most other machines:
# 	CFLAGS = -O -DUNX -fwritable-strings -Wall
# for Linux g++ 4.4.1-4ubuntu9 compiler:
#	CFLAGS = -DUNX -Wno-deprecated -Wno-deprecated-declarations -Wno-narrowing
# if previous doesn't work try:
#	CFLAGS = -DUNX -fwritable-strings
# OR
#	CFLAGS = -O -DUNX -DERRNO -fwritable-strings
#	CFLAGS = -DUNX -DERRNO -fwritable-strings

LIB = -lcurses -ltermcap

MORFILES = mor/arules.cpp mor/crules.cpp mor/drules.cpp mor/globals.cpp \
		   mor/lex-trie.cpp mor/morlib.cpp mor/mor.cpp

VOCD = vocd/args.cpp vocd/speaker.cpp vocd/tokens.cpp vocd/filter.cpp \
	   vocd/dcompute.cpp vocd/vocd.cpp 

POSTC = post/anaalgo.cpp post/atom.cpp post/brillanx.cpp \
		post/brilltagger.cpp post/compoundlib.cpp post/database.cpp \
		post/hashbsc.cpp post/hashc.cpp post/input.cpp \
		post/mortags.cpp post/msg.cpp post/rules.cpp post/splitchat.cpp \
		post/storage.cpp post/tags.cpp post/workspace.cpp
POSTH = post/atom.hpp post/compound.hpp post/database.hpp post/hashbsc.hpp \
		post/hashc.hpp post/input.hpp post/mortags.hpp post/msg.hpp \
		post/storage.hpp post/tags.hpp post/workspace.hpp \
		post/sysdep.hpp post/system.hpp post/database.hpp
MEGRASPC = megrasp/megrasp.cpp megrasp/maxent.cpp megrasp/blmvm.cpp
MEGRASPH = megrasp/ksutil.h megrasp/maxent.h megrasp/blmvm.h

all:	anvil2chat chains chat2anvil chat2ca chat2elan chat2praat chat2xmar check \
	chip chstring codes combo combtier compound cooccur  dataclean dates delim dist \
	dss elan2chat eval eval-d fixbullets fixit flo flucalc freq freqpos gem gemfreq \
	gemlist indent ipsyn keymap kideval kwal lines lena2chat lipp2chat longtier \
	lowcase makemod maxwd megrasp mlt mlu modrep mor mortable ort play2chat phonfreq \
	post postlist postmodrules postmortem praat2chat posttrain \
	quotes rely repeat retrace salt2chat script srt2chat sugar text2chat timedur \
	tierorder trnfix uniq validateMFA vocd wdlen test

anvil2chat:	  	$(DD)/anvil2chat
chains:	  		$(DD)/chains
chat2anvil:		$(DD)/chat2anvil
chat2ca:		$(DD)/chat2ca
chat2elan:		$(DD)/chat2elan
chat2praat:		$(DD)/chat2praat
chat2xmar:		$(DD)/chat2xmar
check:			$(DD)/check
chip:			$(DD)/chip
chstring:		$(DD)/chstring
codes:			$(DD)/codes
combo:			$(DD)/combo
compound:		$(DD)/compound
combtier:		$(DD)/combtier
cooccur:		$(DD)/cooccur
dataclean:		$(DD)/dataclean
dates:			$(DD)/dates
delim:			$(DD)/delim
dist:			$(DD)/dist
dss:			$(DD)/dss
elan2chat:		$(DD)/elan2chat
eval:			$(DD)/eval
eval-d:			$(DD)/eval-d
fixbullets:		$(DD)/fixbullets
fixit:			$(DD)/fixit
flo:			$(DD)/flo
flucalc:		$(DD)/flucalc
freq:			$(DD)/freq
freqpos:		$(DD)/freqpos
gem:			$(DD)/gem
gemfreq:		$(DD)/gemfreq
gemlist:		$(DD)/gemlist
imdi:			$(DD)/imdi
indent:			$(DD)/indent
ipsyn:			$(DD)/ipsyn
keymap:			$(DD)/keymap
kideval:		$(DD)/kideval
kwal:			$(DD)/kwal
lines:			$(DD)/lines
lena2chat:		$(DD)/lena2chat
lipp2chat:		$(DD)/lipp2chat
longtier:		$(DD)/longtier
lowcase:		$(DD)/lowcase
makemod:		$(DD)/makemod
maxwd:			$(DD)/maxwd
megrasp:		$(DD)/megrasp
mlt:			$(DD)/mlt
mlu:			$(DD)/mlu
modrep:			$(DD)/modrep
mor:			$(DD)/mor
mortable:		$(DD)/mortable
ort:			$(DD)/ort
play2chat:		$(DD)/play2chat
phonfreq:		$(DD)/phonfreq
post:			$(DD)/post
postlist:		$(DD)/postlist
postmodrules:		$(DD)/postmodrules
postmortem:		$(DD)/postmortem
praat2chat:		$(DD)/praat2chat
posttrain:		$(DD)/posttrain
quotes:			$(DD)/quotes
rely:			$(DD)/rely
repeat:			$(DD)/repeat
retrace:		$(DD)/retrace
salt2chat:		$(DD)/salt2chat
script:			$(DD)/script
srt2chat:	  	$(DD)/srt2chat
sugar:			$(DD)/sugar
text2chat:		$(DD)/text2chat
timedur:		$(DD)/timedur
tierorder:		$(DD)/tierorder
trnfix:			$(DD)/trnfix
uniq:			$(DD)/uniq
validateMFA:		$(DD)/validateMFA
vocd:			$(DD)/vocd
wdlen:			$(DD)/wdlen

test:			$(DD)/test

$(OBJ)/c_ver.o: c_ver.cpp
	$(CC) -c $(CFLAGS) -o $(OBJ)/c_ver.o c_ver.cpp

$(OBJ)/cut.o: cutt.cpp cu.h common.h mul.h
	$(CC) -c $(CFLAGS) -o $(OBJ)/cut.o cutt.cpp

$(OBJ)/stp.o: stringparser.cpp stringparser.h fontconvert.h
	$(CC) -c $(CFLAGS) -o $(OBJ)/stp.o stringparser.cpp

$(OBJ)/fntc.o: fontconvert.cpp fontconvert.h
	$(CC) -c $(CFLAGS) -o $(OBJ)/fntc.o fontconvert.cpp

$(OBJ)/cxml.o: cutt-xml.cpp cu.h common.h mul.h
	$(CC) -c $(CFLAGS) -o $(OBJ)/cxml.o cutt-xml.cpp


$(DD)/anvil2chat: Anvil2Chat.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/cxml.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) Anvil2Chat.cpp -o $(DD)/anvil2chat $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/cxml.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/chains: chains.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) chains.cpp -o $(DD)/chains $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o -lm

$(DD)/chat2anvil: Chat2Anvil.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) Chat2Anvil.cpp -o $(DD)/chat2anvil $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o -lm

$(DD)/chat2ca: Chat2CA.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) Chat2CA.cpp -o $(DD)/chat2ca $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/chat2elan: Chat2Elan.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) Chat2Elan.cpp -o $(DD)/chat2elan $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/chat2praat: Chat2Praat.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) Chat2Praat.cpp -o $(DD)/chat2praat $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/chat2xmar: Chat2Xmar.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) Chat2Xmar.cpp -o $(DD)/chat2xmar $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/check: check.cpp cu.h common.h check.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) check.cpp -o $(DD)/check $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o -lm

$(DD)/chip: chip.cpp chip_utl.cpp chip.h cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) chip.cpp chip_utl.cpp -o $(DD)/chip $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o -lm

$(DD)/chstring: chstring.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) chstring.cpp -o $(DD)/chstring $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/codes: codes-p.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) codes-p.cpp -o $(DD)/codes $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/combo: combo.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) combo.cpp -o $(DD)/combo $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/compound: post/compound.cpp $(POSTH) $(POSTC) 
	$(CC) -Ipost -I. $(CFLAGS) $(POSTC) post/compound.cpp -o $(DD)/compound

$(DD)/combtier: combtier.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) combtier.cpp -o $(DD)/combtier $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/cooccur: cooccur.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) cooccur.cpp -o $(DD)/cooccur $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/dataclean: DataCleanUp.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) DataCleanUp.cpp -o $(DD)/dataclean $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/dates: dates.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) dates.cpp -o $(DD)/dates $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/delim: delim.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) delim.cpp -o $(DD)/delim $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/dist: dist.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) dist.cpp -o $(DD)/dist $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/dss: dss.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) dss.cpp -o $(DD)/dss $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/elan2chat: Elan2Chat.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/cxml.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) Elan2Chat.cpp -o $(DD)/elan2chat $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/cxml.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/eval: eval.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) eval.cpp -o $(DD)/eval $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/eval-d: eval-d.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) eval-d.cpp -o $(DD)/eval-d $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/fixbullets: FixBullets.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) FixBullets.cpp -o $(DD)/fixbullets $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/fixit: fixit.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) fixit.cpp -o $(DD)/fixit $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/flo: flo.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) flo.cpp -o $(DD)/flo $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/flucalc: flucalc.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) flucalc.cpp -o $(DD)/flucalc $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/freq: freq.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) freq.cpp -o $(DD)/freq $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/freqpos: freqpos.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) freqpos.cpp -o $(DD)/freqpos $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/gem: gem.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) gem.cpp -o $(DD)/gem $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/gemfreq: gemfreq.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) gemfreq.cpp -o $(DD)/gemfreq $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/gemlist: gemlist.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) gemlist.cpp -o $(DD)/gemlist $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/imdi: imdi.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) imdi.cpp -o $(DD)/imdi $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/indent: indent.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) indent.cpp -o $(DD)/indent $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/ipsyn: ipsyn.cpp ipsyn.h cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) ipsyn.cpp -o $(DD)/ipsyn $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/keymap: keymap.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) keymap.cpp -o $(DD)/keymap $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/kideval: kideval.cpp dss.cpp dss.h ipsyn.h $(VOCD) cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) -Ivocd -I. $(CFLAGS) -DKIDEVAL_LIB kideval.cpp dss.cpp ipsyn.cpp $(VOCD) -o $(DD)/kideval $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/kwal: kwal.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) kwal.cpp -o $(DD)/kwal $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/lines: lines.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) lines.cpp -o $(DD)/lines $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/lena2chat: Lena2Chat.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/cxml.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) Lena2Chat.cpp -o $(DD)/lena2chat $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/cxml.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/lipp2chat: lippUTF2Chat.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) lippUTF2Chat.cpp -o $(DD)/lipp2chat $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/longtier: longtier.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) longtier.cpp -o $(DD)/longtier $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/lowcase: lowcase.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) lowcase.cpp -o $(DD)/lowcase $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/makemod: makemod.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) makemod.cpp -o $(DD)/makemod $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/maxwd: maxwd.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) maxwd.cpp -o $(DD)/maxwd $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/megrasp: $(MEGRASPC) $(MEGRASPH) cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) $(MEGRASPC) -o $(DD)/megrasp $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/mlt: mlt.cpp mllib.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) mlt.cpp mllib.cpp -o $(DD)/mlt $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/mlu: mlu.cpp mllib.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) mlu.cpp mllib.cpp -o $(DD)/mlu $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o -lm

$(DD)/modrep: modrep.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) modrep.cpp -o $(DD)/modrep $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/ort: ort.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) ort.cpp -o $(DD)/ort $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/mor: $(MORFILES) mor/mor.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) -Imor -I. $(CFLAGS) $(MORFILES) -o $(DD)/mor $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/mortable: mortable.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) mortable.cpp -o $(DD)/mortable $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/play2chat: Play2Chat.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) Play2Chat.cpp -o $(DD)/play2chat $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/phonfreq: phonfreq.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) phonfreq.cpp -o $(DD)/phonfreq $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/post: post/post.cpp $(POSTH) $(POSTC) post/postana.cpp
	$(CC) -Ipost -I. $(CFLAGS) $(POSTC) post/postana.cpp post/post.cpp -o $(DD)/post

$(DD)/postlist: post/postlist.cpp $(POSTH) $(POSTC) post/trainproc.cpp
	$(CC) -Ipost -I. $(CFLAGS) $(POSTC) post/trainproc.cpp post/postlist.cpp -o $(DD)/postlist

$(DD)/postmodrules: post/postmodrules.cpp $(POSTH) $(POSTC)
	$(CC) -Ipost -I. $(CFLAGS)  $(POSTC) post/postmodrules.cpp -o $(DD)/postmodrules

$(DD)/postmortem: postmortem.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) postmortem.cpp -o $(DD)/postmortem $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/praat2chat: Praat2Chat.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) Praat2Chat.cpp -o $(DD)/praat2chat $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/posttrain: post/posttrain.cpp $(POSTH) $(POSTC) post/trainproc.cpp post/trainbrill.cpp post/contextrl.cpp post/darray.cpp post/darray.hpp post/registry.cpp post/registry.hpp post/registryP.hpp post/memory.cpp post/memory.hpp
	$(CC) -Ipost -I. $(CFLAGS)  $(POSTC) post/trainproc.cpp post/trainbrill.cpp post/contextrl.cpp post/darray.cpp post/registry.cpp post/memory.cpp post/posttrain.cpp -o $(DD)/posttrain

$(DD)/quotes: Quotes.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) Quotes.cpp -o $(DD)/quotes $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/rely: rely.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) rely.cpp -o $(DD)/rely $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/repeat: repeat.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) repeat.cpp -o $(DD)/repeat $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/retrace: retrace.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) retrace.cpp -o $(DD)/retrace $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/salt2chat: salt2chat.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) salt2chat.cpp -o $(DD)/salt2chat $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/script: script.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) script.cpp -o $(DD)/script $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/srt2chat: Srt2Chat.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/cxml.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) Srt2Chat.cpp -o $(DD)/srt2chat $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/cxml.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/sugar: sugar.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) sugar.cpp -o $(DD)/sugar $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/text2chat: text2chat.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) text2chat.cpp -o $(DD)/text2chat $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/timedur: timedur.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) timedur.cpp -o $(DD)/timedur $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/tierorder: tierorder.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) tierorder.cpp -o $(DD)/tierorder $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/trnfix: TrnFix.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) TrnFix.cpp -o $(DD)/trnfix $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/validateMFA: validateMFA.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) validateMFA.cpp -o $(DD)/validateMFA $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/uniq: uniq.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) uniq.cpp -o $(DD)/uniq $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/vocd: $(VOCD) cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) -Ivocd -I. $(CFLAGS) $(VOCD) -o $(DD)/vocd $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/wdlen: wdlen.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) wdlen.cpp -o $(DD)/wdlen $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

$(DD)/test: test.cpp cu.h common.h $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o
	$(CC) $(CFLAGS) test.cpp -o $(DD)/test $(OBJ)/c_ver.o $(OBJ)/cut.o $(OBJ)/stp.o $(OBJ)/fntc.o

clean:
	-rm -f $(DD)/* $(OBJ)/*

.PHONY: all clean \
	anvil2chat \
	chains \
	chat2anvil \
	chat2ca \
	chat2elan \
	chat2praat \
	chat2xmar \
	check \
	chip \
	chstring \
	codes \
	combo \
	compound \
	combtier \
	cooccur \
	dataclean \
	dates \
	delim \
	dist \
	dss \
	elan2chat \
	eval \
	eval-d \
	fixbullets \
	fixit \
	flo \
	flucalc \
	freq \
	freqpos \
	gem \
	gemfreq \
	gemlist \
	imdi \
	indent \
	ipsyn \
	keymap \
	kideval \
	kwal \
	lines \
	lena2chat \
	lipp2chat \
	longtier \
	lowcase \
	makemod \
	maxwd \
	megrasp \
	mlt \
	mlu \
	modrep \
	mor \
	mortable \
	ort \
	play2chat \
	phonfreq \
	post \
	postlist \
	postmodrules \
	postmortem \
	praat2chat \
	posttrain \
	quotes \
	rely \
	repeat \
	retrace \
	salt2chat \
	script \
	srt2chat \
	sugar \
	text2chat \
	timedur \
	tierorder \
	trnfix \
	uniq \
	validateMFA \
	vocd \
	wdlen \
	test
