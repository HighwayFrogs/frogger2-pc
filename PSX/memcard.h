#ifndef _MEMCARD_H_INCLUDE
#define _MEMCARD_H_INCLUDE


typedef struct
{
	int saveStage;
	int saveFrame;
	int saveChosen;
	int load;
}SAVE_INFO;

extern SAVE_INFO saveInfo;

int gameSaveGetCardStatus();
void SaveGame(void);
void LoadGame(void);

#endif