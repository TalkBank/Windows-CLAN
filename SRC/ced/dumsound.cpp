#include "ced.h"
#include "MMedia.h"

int play_sound(char *file, long int begin, long int end, int cont) {
    strcpy(global_df->err_message, "NOT IMPLEMENTED YET !");
    return(1);
}

void show_wave(char *file, long int begin, long int end, int add) {
    strcpy(global_df->err_message, "NOT IMPLEMENTED YET !");
    return;
}

int soundwindow(int TurnOn) {
    strcpy(global_df->err_message, "NOT IMPLEMENTED YET !");
    return(1);
}

int SelectSoundFile(int i) {
    IllegalFunction(-1); strcpy(global_df->err_message, "NOT IMPLEMENTED YET !");
    return(66);
}

int SoundWinSync(int i) {
    IllegalFunction(-1); strcpy(global_df->err_message, "NOT IMPLEMENTED YET !");
    return(78);
}

int ContPlayPause(void) {
    IllegalFunction(-1); strcpy(global_df->err_message, "NOT IMPLEMENTED YET !");
    return(TRUE);
}

void PrintSoundWin(int col, int cm, Size cur) {
}

void PutSoundStats(Size ws) {
}

char AdjustSound(int row, int col, int ext, Size right_lim) {
	return(1);
}

char SetCurrSoundTier(void) {
	return(FALSE);
}

void delay_mach(Size num) {
}

void DisplayEndF(char all) {
}
