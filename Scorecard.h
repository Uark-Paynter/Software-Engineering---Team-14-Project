#pragma once
#include <semaphore.h>
struct Scorecard {
	int activeIDS[30];
	int scores[30];
	int playercount;
	sem_t mutex;
	char events[100][16];
	int loggedevents;
};

Scorecard* initSharedMemory(bool create = true);
void addID(Scorecard* scorecard, int ID);
void removeID(Scorecard* scorecard, int ID);
void clearIDS(Scorecard* scorecard);
void updateScore(Scorecard* scorecard, int ID, int Score);
void clearScores(Scorecard* scorecard);
void fetchScore(Scorecard* scorecard, int ID);
void printScores(Scorecard* scorecard);
void logEvent(Scorecard* scorecard, const char* event);
bool pullEvent(Scorecard* scorecard, char* buffer, size_t size);