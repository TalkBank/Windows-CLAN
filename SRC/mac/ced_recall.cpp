#include "ced.h"
#include "c_clan.h"

#define NUM_COMMANDS 50
#define MAX_COM_LEN  512


static char  clan_commands[NUM_COMMANDS][MAX_COM_LEN+1];
static short curCommand;
short lastCommand;

void init_commands(void) {
	register int i;

	curCommand = 0;
	lastCommand = 0;
	for (i=0; i < NUM_COMMANDS; i++) {
		clan_commands[i][0] = EOS;
	}
}

void set_commands(char *text) {
	register int i;

	for (i=0; i < NUM_COMMANDS; i++) {
		if (clan_commands[i][0] == EOS) {
			strncpy(clan_commands[i], text, MAX_COM_LEN);
			clan_commands[i][MAX_COM_LEN] = EOS;
			break;
		}
	}
}

void set_lastCommand(short num) {
	lastCommand = num;
	if (lastCommand < 0 || lastCommand >= NUM_COMMANDS)
		lastCommand = 0;
	curCommand  = lastCommand;
}

NSDictionary *get_next_obj_command(NSUInteger *i) {
	NSDictionary *addedObject;
	NSString *str;

	while (*i < NUM_COMMANDS) {
		if (clan_commands[*i][0] == EOS) {
			*i = *i + 1;
		} else {
			str = [NSString stringWithUTF8String:clan_commands[*i]];
			addedObject = [NSDictionary dictionaryWithObjectsAndKeys:str, @"lines", nil];
			return(addedObject);
		}
	}
	return(nil);
}

int getRecall_curCommand(void) {
	return(curCommand);
}

void write_commands1977(FILE *fp) {
	register int i;

	for (i=0; i < NUM_COMMANDS; i++) {
		if (clan_commands[i][0] != EOS)
			fprintf(fp, "%d=%s\n", 1977, clan_commands[i]);
	}
}

void delFromRecall(int index) {
	int i;

	if (index < 0 || index >= NUM_COMMANDS)
		return;
	for (i=index; i < NUM_COMMANDS-1; i++) {
		strcpy(clan_commands[i], clan_commands[i+1]);
	}
	clan_commands[i][0] = EOS;
	if (curCommand > index)
		curCommand--;
	lastCommand--;
	WriteClanPreference();
}

void AddToClan_commands(char *st) {
	register int i;

	if (strlen(st) >= MAX_COM_LEN)
		return;

	i = lastCommand - 1;
	if (i < 0)
		i = NUM_COMMANDS - 1;
	if (!strcmp(clan_commands[i], st)) {
		curCommand = lastCommand;
		return;
	}

	strncpy(clan_commands[lastCommand], st, MAX_COM_LEN);
	clan_commands[lastCommand][MAX_COM_LEN] = EOS;
//	if (RecallWin != NULL) {
//		UpdateRecall(RecallWin);
//	}
	lastCommand++;
	if (lastCommand >= NUM_COMMANDS)
		lastCommand = 0;
	curCommand = lastCommand;
	WriteClanPreference();
}

void RecallCommand(short type) {
	NSInteger comLen;

	if (type == up_arrow) {
		type = curCommand;
		if (--curCommand < 0)
			curCommand = NUM_COMMANDS - 1;
		while (clan_commands[curCommand][0] == EOS && curCommand != type) {
			if (--curCommand < 0)
				curCommand = NUM_COMMANDS - 1;
		}

		AddComStringToComWin(clan_commands[curCommand]);
	} else if (type == down_arrow) {
		type = curCommand;
		if (curCommand == lastCommand) {
			comLen = command_length();
			if (comLen == 0)
				curCommand--;
		}
		if (++curCommand >= NUM_COMMANDS)
			curCommand = 0;
		while (clan_commands[curCommand][0] == EOS && curCommand != type) {
			if (++curCommand >= NUM_COMMANDS)
				curCommand = 0;
		}

		AddComStringToComWin(clan_commands[curCommand]);
	}
}

char *getCommandAtIndex(long clickedRow) {
	if (clickedRow >= 0 && clickedRow < NUM_COMMANDS) {
		curCommand = clickedRow;
		return(clan_commands[curCommand]);
	}
	return(nil);
}
