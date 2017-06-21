// recursiver directories listing
#define VERSION "(22-06-97)"

#include "cu.h"

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define CHAT_MODE 0

int depth_lim;
/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	if (f) {
	} else {
	}
}

void usage() {
//	printf("Usage: ab [%s] filename(s)\n",mainflgs());
//	mainusage();
	printf("Usage: temp <number>\n");
}


static void CloseDir(int ref) {
	WDPBRec myWDPB;

	myWDPB.ioCompletion = NULL;
	myWDPB.ioVRefNum = ref;
	PBCloseWD(&myWDPB,false);
}

static int Get_Dir(char *dirname,int index) {
	HParamBlockRec	myCPB;
	short nowref;
	
	dirname[0] = '\0';		
	CtoPstr(dirname);
	GetVol(nil, (short *)&nowref);
	myCPB.fileParam.ioFDirIndex = index;
	myCPB.fileParam.ioDirID = 0L;
	myCPB.fileParam.ioVRefNum = nowref;
	myCPB.fileParam.ioNamePtr = (StringPtr)dirname;

	while(PBGetCatInfo((CInfoPBPtr)&myCPB,0) == noErr) {
		if (((myCPB.fileParam.ioFlAttrib >> 4) & 0x01) == 1) {
 			PtoCstr((unsigned char *)dirname);
			return(++index);
		} else {
			myCPB.fileParam.ioFDirIndex = ++index;
			myCPB.fileParam.ioDirID = 0L;
			myCPB.fileParam.ioVRefNum = nowref;
			dirname[0] = '\0';
			CtoPstr(dirname);
		}	
	}	 
	PtoCstr((unsigned char *)dirname);
	return(0);
}

static int Get_fTEXT(char *filename, int index) {
	FileParam pb2;

	CtoPstr(filename);
	pb2.ioFDirIndex = index;
	pb2.ioNamePtr	= (StringPtr)filename;
	pb2.ioVRefNum	= 0;
	pb2.ioFVersNum	= 0;
	while(PBGetFInfo((ParmBlkPtr)&pb2,false) != fnfErr) {
 		if (pb2.ioFlFndrInfo.fdType == 'TEXT') {
 			PtoCstr((unsigned char *)filename);
			return(++index);
		} else pb2.ioFDirIndex = ++index;
	}
	PtoCstr((unsigned char *)filename);
	return(0);
}

static void show_contents(void) {
	char fdr[256];
	int index;

	index = 1;
	while(index = Get_Dir(fdr,index))
		fprintf(stderr,":%s\n",fdr);

	index = 1;
	while(index = Get_fTEXT(fdr,index))
		fprintf(stderr,"%s\n",fdr);
}

static void Show_Cat(long dirID,HParamBlockRec	*myCPB,char *dirname,int depth) {
	int index = 1;
	short now_ref, tempref;
	long tempioDirID = 0L;
	Str255 dummy;
		
	dummy[0] = '\0';		
	CtoPstr((char *)dummy);

	dirname[0] = '\0';
	pathname(dirname,dirID);
	if (!depth_lim || depth >= depth_lim)
		fprintf(stderr,"%d: %s\n", depth, dirname);

	GetVol(nil, (short *)&now_ref);
 	tempioDirID = dirID;
 	tempref = 0;
 	tempref = (short)mysetdir(&tempioDirID,(int)tempref,dirname);
	if (tempref != now_ref)
		SetVol(0L, tempref);

//	show_contents();

	myCPB->fileParam.ioFDirIndex = index;
	myCPB->fileParam.ioDirID = dirID;
	myCPB->fileParam.ioVRefNum = 0;
	myCPB->fileParam.ioNamePtr = (StringPtr)dummy;

	while(PBGetCatInfo((CInfoPBPtr)myCPB,0) == noErr) {
		if (((myCPB->fileParam.ioFlAttrib >> 4) & 0x01) == 1) {
 		 	Show_Cat(myCPB->fileParam.ioDirID,myCPB,dirname,depth+1);
 		}
		myCPB->fileParam.ioFDirIndex = ++index;
		myCPB->fileParam.ioDirID = dirID;
		myCPB->fileParam.ioVRefNum = 0;
		dummy[0] = '\0';		
		CtoPstr((char *)dummy);			
	}
	PtoCstr((unsigned char *)dummy);
}

static void show_contents_recursively(void) {
	int depth = 0;
	char wdir[256];	
	short tempref, oldref;
	long int tempioDirID;
	HParamBlockRec	myCPB;

	wdir[0] = '\0';
	GetVol(nil, (short *)&oldref);

	tempioDirID = 0L;
	tempref = (short)mysetdir(&tempioDirID,(int)oldref,wdir);
	if(tempref == 0)
		return;

	if(tempref != oldref)
		SetVol(0L, tempref);

	Show_Cat(tempioDirID,&myCPB,wdir,depth);

//	if(tempref != oldref) {
//		CloseDir((int)tempref);	
//	}
}

void call() {
	SetVol(0L, wd_ref);
	show_contents_recursively();
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	lversion = VERSION;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;

//	bmain(argc,argv,NULL);

	depth_lim = 0;
	if (argc == 1) {
		usage();
		return;
	}
	depth_lim = atoi(argv[1]);
	call();
}
	
void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

