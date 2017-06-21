/**********************************************************************
 "Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
 */

#define CHAT_MODE 0
#include "cu.h"

#if !defined(UNX)
#define _main gps_main
#define call gps_call
#define getflag gps_getflag
#define init gps_init
#define usage gps_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern struct tier *defheadtier;
extern char OverWriteFile;

static char gps_ftime, isConvert, isRouteFtime;
static char line[BUFSIZ], GPSValue[UTTLINELEN], *routeName;

void usage() {
	printf("Usage: gps [cN cS nS %s] filename(s)\n", mainflgs());
	puts("-c0: convert .gpx file to .csv file");
	puts("-c1: convert waypoints in .gpx file to .txt format");
	puts("-c : convert all routes in .gpx file to waypoints in .txt");
	puts("-cS: convert the route S in .gpx file to waypoints in .txt");
	puts("-nS: convert GPS coordinates S to: dd\302\260 mm\" ss'");
	mainusage(FALSE);
	puts("Example:");
	puts("   Clean up .CSV file and add \"(GPS coordinates)\"");
	puts("       gps *.csv");
	puts("   Clean up .GPX file and add \"(GPS coordinates)\"");
	puts("       gps lxs-east.GPX");
	puts("   Convert .GPX file to .CSV file");
	puts("       gps -c0 lxs-east.GPX");
	puts("   Create .TXT file from .GPX file");
	puts("       gps -c1 lxs-east.GPX");
	puts("   Create .TXT file of waypoints from named route");
	puts("       gps -c\"2012-09-H-West-FULL\" lxs-east.GPX");
	puts("   To convert GPS coordinates to: dd\302\260 mm\" ss'");
	puts("       gps -n\"N41 13.459\" -w\"W81 10.069\"");
	puts("       gps -n41.224444 -w-81.167778");
	puts("       gps -n41.224444");
	cutt_exit(0);
}

void init(char f) {
	if (f) {
		routeName = NULL;
		templineC[0] = EOS;
		templineC1[0] = EOS;
		gps_ftime = TRUE;
		isRouteFtime = TRUE;
		isConvert = 0;
		stout = FALSE;
		onlydata = 1;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	} else {
		if (gps_ftime) {
			gps_ftime = FALSE;
			if (isConvert == 2 || isConvert == 3)
				AddCEXExtension = ".txt";
		}
	}
}

static void convertCoordinates(char des, char *st) {
	int i, gpsnum;
	int num_degs, num_min, num_sec;
	double num, mult;
	char isFoundSep;

	if (strchr(st, '.') == NULL) {
		fprintf(stdout, "%c%s ", des, st);
		return;
	}
	isFoundSep = FALSE;
	if (strchr(st, ' ') != NULL)
		isFoundSep = TRUE;
	if (strchr(st, '_') != NULL)
		isFoundSep = TRUE;
	if (isFoundSep) {
		fputc(des, stdout);
		for (i=0; st[i] != EOS && st[i] != '.'; i++)
			fputc(st[i], stdout);
		if (st[i] == '.') {
			i++;
			num = (double)atoi(st+i);
			num = num * 0.06;
			gpsnum = num;
			num = num - gpsnum;
			if (num > 0.5)
				gpsnum++;
			fprintf(stdout, " %d ", gpsnum);
		}
	} else {
		for (i=0; st[i] != EOS && !isdigit(st[i]); i++) ;
		if (st[i] == EOS)
			return;
		else if (i > 0)
			strcpy(st, st+i);
		mult = 60.0000;
		num = atof(st);
		num_degs = num;
		num = num - num_degs;
		num = num * mult;
		num_min = num;
		num = num - num_min;
		num = num * mult;
		num_sec = num;
		num = num - num_sec;
		if (num > 0.5)
			num_sec++;
//		fprintf(stdout, "%c%d%c%c %d' %d\" ", des, num_degs, 0xC2, 0xB0, num_min, num_sec);
		fprintf(stdout, "%c%d %d %d ", des, num_degs, num_min, num_sec);
	}
}

static void gpsSingleCall(int argc, char *argv[]) {
	int i;

	mmaininit();
	init(TRUE);
	fpout = stdout;
	for (i=1; i < argc; i++) {
		if (*argv[i] == '+'  || *argv[i] == '-') 
			getflag(argv[i],argv[i+1],&i);
	}		

	if (templineC[0] != EOS && templineC1[0] != EOS) {
		if (templineC[0] == '-') {
			strcpy(templineC, templineC+1);
			convertCoordinates('S', templineC);
		} else if (templineC[0] == 'S' || templineC[0] == 's') {
			strcpy(templineC, templineC+1);
			convertCoordinates('S', templineC);
		} else if (templineC[0] == 'N' || templineC[0] == 'n') {
			strcpy(templineC, templineC+1);
			convertCoordinates('N', templineC);
		} else
			convertCoordinates('N', templineC);
		fputc(' ', stdout);
		if (templineC1[0] == '-') {
			strcpy(templineC1, templineC1+1);
			convertCoordinates('W', templineC1);
		} else if (templineC1[0] == 'W' || templineC1[0] == 'w') {
			strcpy(templineC1, templineC1+1);
			convertCoordinates('W', templineC1);
		} else if (templineC1[0] == 'E' || templineC1[0] == 'e') {
			strcpy(templineC1, templineC1+1);
			convertCoordinates('E', templineC1);
		} else
			convertCoordinates('E', templineC1);
		fputc('\n', stdout);
	} else if (templineC[0] != EOS) {
		if (templineC[0] == '-') {
			strcpy(templineC, templineC+1);
		}
		convertCoordinates(' ', templineC);
		fputc('\n', stdout);
	} else if (templineC1[0] != EOS) {
		if (templineC1[0] == '-') {
			strcpy(templineC1, templineC1+1);
		}
		convertCoordinates(' ', templineC1);
		fputc('\n', stdout);
	} 
	main_cleanup();
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int   i;
	char isFileFound;
	extern void VersionNumber(char isshortfrmt, FILE *fp);

	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = GPS;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	if (argc < 2)
		usage();
	if (argc > 1) {
		if (argv[1][0] == '-' || argv[1][0] == '+') {
			if (argv[1][1] == 'v') {
				if (argv[1][2] == EOS) {
					VersionNumber(FALSE, stdout);
					cutt_exit(0);
				}
			}
		}
	}
	isFileFound = FALSE;
	for (i=1; i < argc; i++) {
		if (*argv[i] == '+'  || *argv[i] == '-')  {
		} else
			isFileFound = TRUE;
	}		
	if (isFileFound)
		bmain(argc,argv,NULL);
	else
		gpsSingleCall(argc, argv);
}
		
void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'n':
			strcpy(templineC, getfarg(f,f1,i));
			break;
		case 'w':
			strcpy(templineC1, getfarg(f,f1,i));
			break;
		case 'c':
			if (*f == '0')
				isConvert = 1;
			else if (*f == '1')
				isConvert = 2;
			else {
				isConvert = 3;
				routeName = f;
			}
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void cleanPreviousGPS(char *line) {
	int b, e, numsCnt;
	char isLatFound, isLotFound, isSpaceFound;

	for (b=0; line[b] != EOS; b++) {
		if (line[b] == '(') {
			isSpaceFound = TRUE;
			isLatFound = FALSE;
			isLotFound = FALSE;
			numsCnt = 0;
			for (e=b+1; line[e] != EOS && line[e] != ')'; e++) {
				if (line[e] == 'N' || line[e] == 'S')
					isLatFound = TRUE;
				else if (line[e] == 'W' || line[e] == 'E')
					isLotFound = TRUE;
				else if (line[e] == '.')
					;
				else if (isdigit(line[e])) {
					if (isSpaceFound)
						numsCnt++;
					isSpaceFound = FALSE;
				} else if (isSpace(line[e]))
					isSpaceFound = TRUE;
				else
					break;
			}
			if (line[e] == ')' && isLatFound && isLotFound && numsCnt == 6) {
				if ((isSpace(line[b]) || b == 0) && isSpace(line[e+1]))
					e++;
				strcpy(line+b, line+e+1);
				return;
			}
		}
	}
}

static void doGPX(void) {
	int i, j, ln, lenGPSstr, ist;
	int la_degs, la_min, la_sec, lo_degs, lo_min, lo_sec;
	char isN, isW, isWPT, isRTEPT, isCMTFound, isAddedCMT;
	double lat, lot, mult;

	mult = 60.0000;
	GPSValue[0] = EOS;
	ln = 0;
	isWPT = FALSE;
	isAddedCMT = FALSE;
	isCMTFound = TRUE;
	while (!feof(fpin)) {
		ln++;
		if (fgets_cr(line, BUFSIZ, fpin) == NULL)
			break;
		for (i=0; line[i] != '\0'; i++) {
			if (!strncmp(line+i, "<wpt ", 5) || !strncmp(line+i, "<rtept ", 7)) {
				if (!strncmp(line+i, "<rtept ", 7))
					isRTEPT = TRUE;
				else
					isRTEPT = FALSE;
				isCMTFound = FALSE;
				isAddedCMT = FALSE;
// find lat
				for (; line[i] != '"' && line[i] != EOS; i++) ;
				if (line[i] == EOS) {
					fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
					fprintf(stderr, "Error on line1 %d\n", ln);
					cutt_exit(0);
				}
				i++;
				for (j=i; line[j] != '"' && line[j] != EOS; j++) ;
				if (line[j] == EOS) {
					fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
					fprintf(stderr, "Error on line2 %d\n", ln);
					cutt_exit(0);
				}
				line[j] = EOS;
				if (line[i] == '-') {
					i++;
					isN = FALSE;
				} else
					isN = TRUE;
				lat = atof(line+i);
				line[j] = '"';
				la_degs = lat;
				lat = lat - la_degs;
				lat = lat * mult;
				la_min = lat;
				lat = lat - la_min;
				lat = lat * mult;
				la_sec = lat;
				lat = lat - la_sec;
				if (lat >= 0.5)
					la_sec++;
// find lot
				for (i=j+1; line[i] != '"' && line[i] != EOS; i++) ;
				if (line[i] == EOS) {
					fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
					fprintf(stderr, "Error on line3 %d\n", ln);
					cutt_exit(0);
				}
				i++;
				for (j=i; line[j] != '"' && line[j] != EOS; j++) ;
				if (line[j] == EOS) {
					fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
					fprintf(stderr, "Error on line4 %d\n", ln);
					cutt_exit(0);
				}
				line[j] = EOS;
				if (line[i] == '-') {
					i++;
					isW = TRUE;
				} else
					isW = FALSE;
				lot = atof(line+i);
				line[j] = '"';
				lo_degs = lot;
				lot = lot - lo_degs;
				lot = lot * mult;
				lo_min = lot;
				lot = lot - lo_min;
				lot = lot * mult;
				lo_sec = lot;
				lot = lot - lo_sec;
				if (lot >= 0.5)
					lo_sec++;

				ist = 0;
				GPSValue[ist++] = '(';
				if (isN)
					GPSValue[ist++] = 'N';
				else
					GPSValue[ist++] = 'S';
				if (la_degs < 10)
					GPSValue[ist++] = '0';
				sprintf(GPSValue+ist, "%d ", la_degs);
				ist = strlen(GPSValue);
				if (la_min < 10)
					GPSValue[ist++] = '0';
				sprintf(GPSValue+ist, "%d ", la_min);
				ist = strlen(GPSValue);
				if (la_sec < 10)
					GPSValue[ist++] = '0';
				sprintf(GPSValue+ist, "%d ", la_sec);
				ist = strlen(GPSValue);
				if (isW)
					GPSValue[ist++] = 'W';
				else
					GPSValue[ist++] = 'E';
				if (lo_degs < 10)
					GPSValue[ist++] = '0';
				sprintf(GPSValue+ist, "%d ", lo_degs);
				ist = strlen(GPSValue);
				if (lo_min < 10)
					GPSValue[ist++] = '0';
				sprintf(GPSValue+ist, "%d ", lo_min);
				ist = strlen(GPSValue);
				if (lo_sec < 10)
					GPSValue[ist++] = '0';
				sprintf(GPSValue+ist, "%d", lo_sec);
				ist = strlen(GPSValue);
				GPSValue[ist++] = ')';
				GPSValue[ist++] = ' ';
				GPSValue[ist] = EOS;

//				sprintf(GPSValue, "(%c%d %d %d %c%d %d %d) ", (isN ? 'N' : 'S'), la_degs, la_min, la_sec, (isW ? 'W' : 'E'), lo_degs, lo_min, lo_sec);
				isWPT = TRUE;
				break;
			} else if (!strncmp(line+i, "<sym>", 5)) {
				if (isCMTFound == FALSE) {
//					fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
//					fprintf(stderr, "Did not find either <cmt> or <desc> for this waypoint\n");
					lenGPSstr = strlen(GPSValue) - 1;
					if (isSpace(GPSValue[lenGPSstr]))
						GPSValue[lenGPSstr] = EOS;
					if (isRTEPT) {
						fprintf(fpout, "            <cmt>%s</cmt>\n", GPSValue);
						fprintf(fpout, "            <desc>%s</desc>\n", GPSValue);
					} else {
						fprintf(fpout, "        <cmt>%s</cmt>\n", GPSValue);
						fprintf(fpout, "        <desc>%s</desc>\n", GPSValue);
					}
					isAddedCMT = TRUE;
				}
				break;
			} else if (!strncmp(line+i, "</wpt>", 6) || !strncmp(line+i, "</rtept>", 8)) {
				isWPT = FALSE;
				GPSValue[0] = EOS;
				break;
			} else if (isWPT && (!strncmp(line+i, "<cmt>", 5) || !strncmp(line+i, "<desc>", 6))) {
				if (isAddedCMT) {
					fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
					fprintf(stderr, "Already added <cmt> or <desc> for this waypoint\n");
				}
				isCMTFound = TRUE;
				for (; line[i] != EOS && line[i] != '>'; i++) ;
				if (line[i] == EOS)
					break;
				i++;
				cleanPreviousGPS(line+i);
				lenGPSstr = strlen(GPSValue);
				if ((isSpace(line[i]) || line[i] == '<') && GPSValue[lenGPSstr-1] != ')') {
					lenGPSstr--;
					GPSValue[lenGPSstr] = EOS;
				}
				uS.shiftright(line+i, lenGPSstr);
				for (j=0; GPSValue[j] != EOS; j++) {
					line[i++] = GPSValue[j];
				}
				break;
			}
		}
		fputs(line, fpout);
	}
}

static char isCoordinates(char *line) {
	int cntComas;

	cntComas = 0;
	for (; *line != EOS; line++) {
		if (!isdigit(*line) && *line != '-' && *line != ',' && *line != '.')
			break;
		if (*line == ',')
			cntComas++;
	}
	if (cntComas == 2)
		return(TRUE);
	else
		return(FALSE);
}

static void doCSV(void) {
	double lat, lot, mult;
	int la_degs, la_min, la_sec, lo_degs, lo_min, lo_sec;
	char isN, isW, isQF;
	int i, j, ln, lenGPSstr;
	
	mult = 60.0000;
	GPSValue[0] = EOS;
	ln = 0;
	while (!feof(fpin)) {
		ln++;
		if (fgets_cr(line, BUFSIZ, fpin) == NULL)
			break;
		if (isCoordinates(line)) {
			cleanPreviousGPS(line);
// find lot
			i = 0;
			for (j=0; line[j] != ',' && line[j] != EOS; j++) ;
			if (line[j] != ',') {
				fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
				fprintf(stderr, "Error on line1 %d\n", ln);
				cutt_exit(0);
			}
			line[j] = EOS;
			if (line[i] == '-') {
				i++;
				isW = TRUE;
			} else
				isW = FALSE;
			lot = atof(line+i);
			line[j] = ',';
			lo_degs = lot;
			lot = lot - lo_degs;
			lot = lot * mult;
			lo_min = lot;
			lot = lot - lo_min;
			lot = lot * mult;
			lo_sec = lot;
			lot = lot - lo_sec;
			if (lot > 0.5)
				lo_sec++;
// find lat
			i= j + 1;
			for (j=i; line[j] != ',' && line[j] != EOS; j++) ;
			if (line[j] != ',') {
				fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
				fprintf(stderr, "Error on line2 %d\n", ln);
				cutt_exit(0);
			}
			line[j] = EOS;
			if (line[i] == '-') {
				i++;
				isN = FALSE;
			} else
				isN = TRUE;
			lat = atof(line+i);
			line[j] = ',';
			la_degs = lat;
			lat = lat - la_degs;
			lat = lat * mult;
			la_min = lat;
			lat = lat - la_min;
			lat = lat * mult;
			la_sec = lat;
			lat = lat - la_sec;
			if (lat > 0.5)
				la_sec++;
			sprintf(GPSValue, "(%c%d %d %d %c%d %d %d) ", (isN ? 'N' : 'S'), la_degs, la_min, la_sec, (isW ? 'W' : 'E'), lo_degs, lo_min, lo_sec);
			i = j + 1;
			isQF = FALSE;
			for (; (line[i] != ',' || isQF) && line[i] != EOS && line[i] != '\n'; i++) {
				if (line[i] == '"')
					isQF = !isQF;
			}
			if (line[i] == EOS) {
				fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
				fprintf(stderr, "Error on line3 %d\n", ln);
				cutt_exit(0);
			}
			if (line[i] == ',') {
				i++;
				if (line[i] == '"')
					i++;
				lenGPSstr = strlen(GPSValue);
				if (isSpace(line[i])) {
					lenGPSstr--;
					GPSValue[lenGPSstr] = EOS;
				}
				uS.shiftright(line+i, lenGPSstr);
				for (j=0; GPSValue[j] != EOS; j++) {
					line[i++] = GPSValue[j];
				}
			} else {
				lenGPSstr = strlen(GPSValue);
				if (isSpace(line[i])) {
					lenGPSstr--;
					GPSValue[lenGPSstr] = EOS;
				}
				uS.shiftright(line+i, lenGPSstr+1);
				line[i++] = ',';
				for (j=0; GPSValue[j] != EOS; j++) {
					line[i++] = GPSValue[j];
				}
			}
		}
		fputs(line, fpout);
	}
}

static void fromXML(char *line) {
	while (*line != EOS) {
		if (!strncmp(line, "&amp;", 5)) {
			*line = '&';
			line++;
			strcpy(line, line+4);
		} else if (!strncmp(line, "&quot;", 6)) {
			*line = '\'';
			line++;
			strcpy(line, line+5);
		} else if (!strncmp(line, "&apos;", 6)) {
			*line = '\'';
			line++;
			strcpy(line, line+5);
		} else if (!strncmp(line, "&lt;", 4)) {
			*line = '<';
			line++;
			strcpy(line, line+3);
		} else if (!strncmp(line, "&gt;", 4)) {
			*line = '>';
			line++;
			strcpy(line, line+3);
		} else
			line++;
	}
}

static void gpxRoute2txt(void) {
	int i, j, ln;
	char isRTE, isWPT, isGPSNumFound, isEndFound, name[BUFSIZ];

	if (routeName == NULL)
		return;
	isRouteFtime = TRUE;
	ln = 0;
	isRTE = FALSE;
	isWPT = FALSE;
	name[0] = EOS;
	GPSValue[0] = EOS;
	while (!feof(fpin)) {
		ln++;
		if (fgets_cr(line, BUFSIZ, fpin) == NULL)
			break;
		for (i=0; line[i] != '\0'; i++) {
			if (!strncmp(line+i, "<rte>", 5)) {
				isRTE = TRUE;
				break;
			} else if (!strncmp(line+i, "</rte>", 6)) {
				isRTE = FALSE;
				isWPT = FALSE;
				name[0] = EOS;
				GPSValue[0] = EOS;
				break;
			} else if (!strncmp(line+i, "<rtept ", 7)) {
				if (isRTE)
					isWPT = TRUE;
				break;
			} else if (!strncmp(line+i, "</rtept>", 8)) {
				if (isRTE) {
					isWPT = FALSE;
					name[0] = EOS;
					GPSValue[0] = EOS;
				}
				break;
			} else if (!isWPT && isRTE && !strncmp(line+i, "<name>", 6)) {
				for (; line[i] != EOS && line[i] != '>'; i++) ;
				name[0] = EOS;
				if (line[i] == EOS)
					break;
				for (i++, j=0; line[i] != EOS; i++) {
					if (!strncmp(line+i, "</name>", 7))
						break;
					name[j++] = line[i];
				}
				name[j] = EOS;
				if (uS.mStricmp(routeName, name) != 0 && routeName[0] != EOS)
					isRTE = FALSE;
				else {
					if (isRouteFtime)
						isRouteFtime = FALSE;
					else
						fprintf(fpout, "\n");
					fprintf(fpout, "***** Route:    %s\n", name);
				}
				break;
			} else if (isWPT && !strncmp(line+i, "<name>", 6)) {
				for (; line[i] != EOS && line[i] != '>'; i++) ;
				name[0] = EOS;
				if (line[i] == EOS)
					break;
				for (i++, j=0; line[i] != EOS; i++) {
					if (!strncmp(line+i, "</name>", 7))
						break;
					name[j++] = line[i];
				}
				name[j] = EOS;
				break;
			} else if (isWPT && !strncmp(line+i, "<cmt>", 5)) {
				for (; line[i] != EOS && line[i] != '>'; i++) ;
				GPSValue[0] = EOS;
				if (line[i] == EOS)
					break;
				isEndFound = FALSE;
				isGPSNumFound = 0;
				for (i++, j=0; line[i] != EOS; i++) {
					if (!strncmp(line+i, "</cmt>", 6)) {
						isEndFound = TRUE;
						break;
					}
					if (isGPSNumFound == 2) {
						isGPSNumFound = -1;
						GPSValue[j++] = '\n';
						GPSValue[j++] = '%';
						GPSValue[j++] = 'c';
						GPSValue[j++] = ':';
						if (line[i] != ' ' && line[i] != '\t')
							GPSValue[j++] = ' ';
					}
					if (isGPSNumFound == 0 && line[i] == '(')
						isGPSNumFound = 1;
					else if (isGPSNumFound == 1 && line[i] == ')')
						isGPSNumFound = 2;
					if (line[i] == '\n')
						GPSValue[j++] = ' ';
					else
						GPSValue[j++] = line[i];
				}
				GPSValue[j] = EOS;
				if (!isEndFound) {
					fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
					fprintf(stderr, "Found unexpected newline on line: %s", line);
				}
				fromXML(name);
				fromXML(GPSValue);
				fprintf(fpout, "%s %s\n", name, GPSValue);
				isWPT = FALSE;
				break;
			} else if (isWPT && !strncmp(line+i, "<desc>", 6)) {
				for (; line[i] != EOS && line[i] != '>'; i++) ;
				GPSValue[0] = EOS;
				if (line[i] == EOS)
					break;
				isEndFound = FALSE;
				isGPSNumFound = 0;
				for (i++, j=0; line[i] != EOS; i++) {
					if (!strncmp(line+i, "</desc>", 7)) {
						isEndFound = TRUE;
						break;
					}
					if (isGPSNumFound == 2) {
						isGPSNumFound = -1;
						GPSValue[j++] = '\n';
						GPSValue[j++] = '%';
						GPSValue[j++] = 'c';
						GPSValue[j++] = ':';
						if (line[i] != ' ' && line[i] != '\t')
							GPSValue[j++] = ' ';
					}
					if (isGPSNumFound == 0 && line[i] == '(')
						isGPSNumFound = 1;
					else if (isGPSNumFound == 1 && line[i] == ')')
						isGPSNumFound = 2;
					if (line[i] == '\n')
						GPSValue[j++] = ' ';
					else
						GPSValue[j++] = line[i];
				}
				GPSValue[j] = EOS;
				if (!isEndFound) {
					fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
					fprintf(stderr, "Found unexpected newline on line: %s", line);
				}
				fromXML(name);
				fromXML(GPSValue);
				fprintf(fpout, "%s %s\n", name, GPSValue);
				isWPT = FALSE;
				break;
			}
		}
	}
}

static void gpx2txt(void) {
	int i, j, ln;
	char isWPT, isGPSNumFound, isEndFound, name[BUFSIZ];

	ln = 0;
	isWPT = FALSE;
	name[0] = EOS;
	GPSValue[0] = EOS;
	while (!feof(fpin)) {
		ln++;
		if (fgets_cr(line, BUFSIZ, fpin) == NULL)
			break;
		for (i=0; line[i] != '\0'; i++) {
			if (!strncmp(line+i, "<wpt ", 5)) {
				isWPT = TRUE;
				break;
			} else if (!strncmp(line+i, "</wpt>", 6)) {
				isWPT = FALSE;
				name[0] = EOS;
				GPSValue[0] = EOS;
				break;
			} else if (isWPT && !strncmp(line+i, "<name>", 6)) {
				for (; line[i] != EOS && line[i] != '>'; i++) ;
				name[0] = EOS;
				if (line[i] == EOS)
					break;
				for (i++, j=0; line[i] != EOS; i++) {
					if (!strncmp(line+i, "</name>", 7))
						break;
					name[j++] = line[i];
				}
				name[j] = EOS;
				break;
			} else if (isWPT && !strncmp(line+i, "<cmt>", 5)) {
				for (; line[i] != EOS && line[i] != '>'; i++) ;
				GPSValue[0] = EOS;
				if (line[i] == EOS)
					break;
				isEndFound = FALSE;
				isGPSNumFound = 0;
				for (i++, j=0; line[i] != EOS; i++) {
					if (!strncmp(line+i, "</cmt>", 6)) {
						isEndFound = TRUE;
						break;
					}
					if (isGPSNumFound == 2) {
						isGPSNumFound = -1;
						GPSValue[j++] = '\n';
						GPSValue[j++] = '%';
						GPSValue[j++] = 'c';
						GPSValue[j++] = ':';
						if (line[i] != ' ' && line[i] != '\t')
							GPSValue[j++] = ' ';
					}
					if (isGPSNumFound == 0 && line[i] == '(')
						isGPSNumFound = 1;
					else if (isGPSNumFound == 1 && line[i] == ')')
						isGPSNumFound = 2;
					if (line[i] == '\n')
						GPSValue[j++] = ' ';
					else
						GPSValue[j++] = line[i];
				}
				GPSValue[j] = EOS;
				if (!isEndFound) {
					fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
					fprintf(stderr, "Found unexpected newline on line: %s", line);
				}
				fromXML(name);
				fromXML(GPSValue);
				fprintf(fpout, "%s %s\n", name, GPSValue);
				isWPT = FALSE;
				break;
			} else if (isWPT && !strncmp(line+i, "<desc>", 6)) {
				for (; line[i] != EOS && line[i] != '>'; i++) ;
				GPSValue[0] = EOS;
				if (line[i] == EOS)
					break;
				isEndFound = FALSE;
				isGPSNumFound = 0;
				for (i++, j=0; line[i] != EOS; i++) {
					if (!strncmp(line+i, "</desc>", 7)) {
						isEndFound = TRUE;
						break;
					}
					if (isGPSNumFound == 2) {
						isGPSNumFound = -1;
						GPSValue[j++] = '\n';
						GPSValue[j++] = '%';
						GPSValue[j++] = 'c';
						GPSValue[j++] = ':';
						if (line[i] != ' ' && line[i] != '\t')
							GPSValue[j++] = ' ';
					}
					if (isGPSNumFound == 0 && line[i] == '(')
						isGPSNumFound = 1;
					else if (isGPSNumFound == 1 && line[i] == ')')
						isGPSNumFound = 2;
					if (line[i] == '\n')
						GPSValue[j++] = ' ';
					else
						GPSValue[j++] = line[i];
				}
				GPSValue[j] = EOS;
				if (!isEndFound) {
					fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
					fprintf(stderr, "Found unexpected newline on line: %s", line);
				}
				fromXML(name);
				fromXML(GPSValue);
				fprintf(fpout, "%s %s\n", name, GPSValue);
				isWPT = FALSE;
				break;
			}
		}
	}
}

static void gpx2csv(void) {
	int i, j, ln;
	char isWPT, name[BUFSIZ];
	double lat, lot;

	ln = 0;
	isWPT = FALSE;
	lat = 0.0;
	lot = 0.0;
	name[0] = EOS;
	GPSValue[0] = EOS;
	while (!feof(fpin)) {
		ln++;
		if (fgets_cr(line, BUFSIZ, fpin) == NULL)
			break;
		for (i=0; line[i] != '\0'; i++) {
			if (!strncmp(line+i, "<wpt ", 5)) {
// find lat
				for (; line[i] != '"' && line[i] != EOS; i++) ;
				if (line[i] == EOS) {
					fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
					fprintf(stderr, "Error on line1 %d\n", ln);
					cutt_exit(0);
				}
				i++;
				for (j=i; line[j] != '"' && line[j] != EOS; j++) ;
				if (line[j] == EOS) {
					fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
					fprintf(stderr, "Error on line2 %d\n", ln);
					cutt_exit(0);
				}
				line[j] = EOS;
				lat = atof(line+i);
// find lot
				for (i=j+1; line[i] != '"' && line[i] != EOS; i++) ;
				if (line[i] == EOS) {
					fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
					fprintf(stderr, "Error on line3 %d\n", ln);
					cutt_exit(0);
				}
				i++;
				for (j=i; line[j] != '"' && line[j] != EOS; j++) ;
				if (line[j] == EOS) {
					fprintf(stderr,"0*** File \"%s\": line %ld.\n", oldfname, ln);			
					fprintf(stderr, "Error on line4 %d\n", ln);
					cutt_exit(0);
				}
				line[j] = EOS;
				lot = atof(line+i);
				isWPT = TRUE;
				break;
			} else if (!strncmp(line+i, "</wpt>", 6)) {
				isWPT = FALSE;
				lat = 0.0;
				lot = 0.0;
				name[0] = EOS;
				GPSValue[0] = EOS;
				break;
			} else if (isWPT && !strncmp(line+i, "<name>", 6)) {
				for (; line[i] != EOS && line[i] != '>'; i++) ;
				name[0] = EOS;
				if (line[i] == EOS)
					break;
				for (i++, j=0; line[i] != EOS; i++) {
					if (!strncmp(line+i, "</name>", 7))
						break;
					name[j++] = line[i];
				}
				name[j] = EOS;
				break;
			} else if (isWPT && !strncmp(line+i, "<cmt>", 5)) {
				for (; line[i] != EOS && line[i] != '>'; i++) ;
				GPSValue[0] = EOS;
				if (line[i] == EOS)
					break;
				for (i++, j=0; line[i] != EOS; i++) {
					if (!strncmp(line+i, "</cmt>", 6))
						break;
					GPSValue[j++] = line[i];
				}
				GPSValue[j] = EOS;
				fromXML(name);
				fromXML(GPSValue);
				fprintf(fpout, "%lf,%lf,\"%s\",\"%s\"\n", lot, lat, name, GPSValue);
				isWPT = FALSE;
				break;
			} else if (isWPT && !strncmp(line+i, "<desc>", 6)) {
				for (; line[i] != EOS && line[i] != '>'; i++) ;
				GPSValue[0] = EOS;
				if (line[i] == EOS)
					break;
				for (i++, j=0; line[i] != EOS; i++) {
					if (!strncmp(line+i, "</desc>", 7))
						break;
					GPSValue[j++] = line[i];
				}
				GPSValue[j] = EOS;
				fromXML(name);
				fromXML(GPSValue);
				fprintf(fpout, "%lf,%lf,\"%s\",\"%s\"\n", lot, lat, name, GPSValue);
				isWPT = FALSE;
				break;
			}
		}
	}
}
/*
static void toXML(char *an, char *bs, char *es) {
	long i;
	
	for (; (isSpace(*bs) || *bs == '\n') && bs < es; bs++) ;
	for (es--; (isSpace(*es) || *es == '\n') && bs <= es; es--) ;
	es++;
	i = 0L;
	for (; bs < es; bs++) {
		if (*bs == '&') {
			strcpy(an+i, "&amp;");
			i = strlen(an);
		} else if (*bs == '"') {
			strcpy(an+i, "&quot;");
			i = strlen(an);
		} else if (*bs == '\'') {
			strcpy(an+i, "&apos;");
			i = strlen(an);
		} else if (*bs == '<') {
			strcpy(an+i, "&lt;");
			i = strlen(an);
		} else if (*bs == '>') {
			strcpy(an+i, "&gt;");
			i = strlen(an);
		} else if (*bs == '\n')
			an[i++] = ' ';
		else if (*bs == '\t')
			an[i++] = ' ';
		else if (*bs >= 0 && *bs < 32) {
			sprintf(an+i,"{0x%x}", *bs);
			i = strlen(an);
		} else
			an[i++] = *bs;
	}
	an[i] = EOS;
}
*/
static void csv2gpx(void) {
}

void call() {
	char *ext;

	ext = strrchr(oldfname, '.');
	if (ext != NULL) {
		if (isConvert == 3) {
			if (uS.mStricmp(ext, ".gpx") == 0)
				gpxRoute2txt();
			else
				fprintf(stderr, "Error file \"%s\" needs to have extension .gpx\n", oldfname);
		} else if (isConvert == 2) {
			if (uS.mStricmp(ext, ".gpx") == 0)
				gpx2txt();
			else
				fprintf(stderr, "Error file \"%s\" needs to have extension .gpx\n", oldfname);
		} else if (isConvert == 1) {
			if (uS.mStricmp(ext, ".gpx") == 0)
				gpx2csv();
			else if (uS.mStricmp(ext, ".csv") == 0)
				csv2gpx();
			else
				fprintf(stderr, "Error file \"%s\" needs to have either extension .gpx or .csv\n", oldfname);
		} else {
			if (uS.mStricmp(ext, ".gpx") == 0)
				doGPX();
			else if (uS.mStricmp(ext, ".csv") == 0)
				doCSV();
			else
				fprintf(stderr, "Error file \"%s\" needs to have either extension .gpx or .csv\n", oldfname);
		}
	}
}

/*
 (N41 13.459 W81 10.069) 
 (N +41.224444 W -81.167778) 
 (N41 13 28  W81 10 4)
static void (void) {
	int i, gpsnum;
	float num;
	char buf[BUFSIZ];
	FILE *fp;
	fp = fopen("gps.txt", "r");
	if (fp == NULL)
		return(1);
	while (fgets(buf, BUFSIZ, fp)) {
		gpsnum = 0;
		for (i=0; buf[i] != '\0'; i++) {
			fputc(buf[i], stdout);
			if (buf[i] == '(' && buf[i+1] == 'N') {
				i++;
				gpsnum = 1;
				break;
			}
		}
		if (gpsnum == 1) {
			for (; buf[i] != '\0' && buf[i] != '.'; i++) {
				fputc(buf[i], stdout);
			}
			if (buf[i] == '.') {
				i++;
				num = atoi(buf+i);
				num = num * 0.06;
				gpsnum = num;
				num = num - gpsnum;
				if (num > 0.5)
					gpsnum++;
				fprintf(stdout, " %d", gpsnum);
				for (; buf[i] != '\0' && buf[i] != ' '; i++) ;
				for (; buf[i] != '\0' && buf[i] != '.'; i++) {
					fputc(buf[i], stdout);
				}
				if (buf[i] == '.') {
					i++;
					num = atoi(buf+i);
					num = num * 0.06;
					gpsnum = num;
					num = num - gpsnum;
					if (num > 0.5)
						gpsnum++;
					fprintf(stdout, " %d", gpsnum);
					for (; buf[i] != '\0' && buf[i] != ')'; i++) ;
				}
				for (; buf[i] != '\0'; i++) {
					fputc(buf[i], stdout);
				}
			}
		}
	}
}
*/

