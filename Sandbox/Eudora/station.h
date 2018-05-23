// Station.h
//
// Handles Stationery list
//

extern CStringList g_StationeryList;

extern void InitStationery();
extern void AddStationeryItem(const char * fileName);
extern BOOL GetStatItemPath(const char *item, char *buffer, int bufsize);
extern BOOL GetStatItemName(const char *path, char *buffer, int bufsize);
