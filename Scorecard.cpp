#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

struct Scorecard {
	int activeIDS[30];
	int scores[30];
	int playercount;
	sem_t mutex;
	char events[100][16];
	int loggedevents;
};

Scorecard* initSharedMemory(bool create = true) {
	int shm_fd;
	if(create) {
		shm_fd = shm_open("/scorecard", O_CREAT | O_RDWR, 0666);
		ftruncate(shm_fd, sizeof(Scorecard));
	}
	else {
		shm_fd = shm_open("/scorecard", O_RDWR, 0666);
	}
	
	if(shm_fd == -1) {
		std::cout << "Scorecard Shared Memory Failure" << std::endl;
		exit(-1);
	}
	
	void* ptr = mmap(NULL, sizeof(Scorecard), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if(ptr == MAP_FAILED) {
		std::cout << "Memory Mapping Failure" << std::endl;
		exit(-1);
	}
	
	Scorecard* scorecard = (Scorecard*)ptr;
	
	if (create) {
		scorecard->playercount = 0;
		memset(scorecard->activeIDS, 0, sizeof(scorecard->activeIDS));
		memset(scorecard->scores, 0, sizeof(scorecard->scores));
		sem_init(&scorecard->mutex, 1, 1);
		scorecard->loggedevents = 0;
		memset(scorecard->events, 0, sizeof(scorecard->events));
	}
	return scorecard;
}

void addID(Scorecard* scorecard, int ID) {
	sem_wait(&scorecard->mutex);
	if(scorecard->playercount < 30) {
		scorecard->activeIDS[scorecard->playercount] = ID;
		scorecard->scores[scorecard->playercount] = 0;
		scorecard->playercount += 1;
		std::cout << "ID: " << ID << " added" << std::endl;
	}
	sem_post(&scorecard->mutex);
}

void removeID(Scorecard* scorecard, int ID) {
	sem_wait(&scorecard->mutex);
	for(int i = 0; i < scorecard->playercount; i++) {
		if(scorecard->activeIDS[i] == ID) {
			for(int j = i; j < scorecard->playercount-1; j++) {
				scorecard->activeIDS[j] = scorecard->activeIDS[j + 1];
				scorecard->scores[j] = scorecard->scores[j+1];
			}
			scorecard->activeIDS[scorecard->playercount-1] = 0;
			scorecard->scores[scorecard->playercount-1]=0;
			scorecard->playercount--;
			break;
		}
	}
	sem_post(&scorecard->mutex);
}

void clearIDS(Scorecard* scorecard) {
	sem_wait(&scorecard->mutex);
	memset(scorecard->activeIDS, 0, sizeof(scorecard->activeIDS));
	memset(scorecard->scores, 0, sizeof(scorecard->scores));
	scorecard->playercount = 0;
	sem_post(&scorecard->mutex);
}

void updateScore(Scorecard* scorecard, int ID, int Score) {
	sem_wait(&scorecard->mutex);
	for(int i = 0; i < scorecard->playercount; i++) {
		if(scorecard->activeIDS[i] == ID) {
			scorecard->scores[i] += Score;
			break;
		}
	}
	sem_post(&scorecard->mutex);
}

void clearScores(Scorecard* scorecard) {
	sem_wait(&scorecard->mutex);
	for(int i = 0; i < scorecard->playercount; i++) {
		scorecard->scores[i] = 0;
	}
	sem_post(&scorecard->mutex);
}

int fetchScore(Scorecard* scorecard, int ID) {
	int result = 0;
	sem_wait(&scorecard->mutex);
	for(int i = 0; i < scorecard->playercount; i++){
		if(scorecard->activeIDS[i] == ID) {
			result = scorecard->scores[i];
			break;
		}
	}
	sem_post(&scorecard->mutex);
	return result;
}

void printScores(Scorecard* scorecard) {
	sem_wait(&scorecard->mutex);
	std::cout << "Scores:" << std::endl;
	for(int i = 0; i < scorecard->playercount; i++) {
		std::cout << "Player ID: " << scorecard->activeIDS[i] << " Score: " << scorecard->scores[i] << std::endl;
	}
	sem_post(&scorecard->mutex);
}

void logEvent(Scorecard* scorecard, const char* event) {
	sem_wait(&scorecard->mutex);
	strncpy(scorecard->events[scorecard->loggedevents], event, sizeof(scorecard->events[0]) -1);
	scorecard->events[scorecard->loggedevents][sizeof(scorecard->events[0]) -1] = '\0';
	scorecard->loggedevents += 1;
	sem_post(&scorecard->mutex);
}

bool pullEvent(Scorecard* scorecard, char* buffer, size_t size) {
	sem_wait(&scorecard->mutex);
	if(scorecard->loggedevents == 0) {sem_post(&scorecard->mutex); return false;}
	strncpy(buffer, scorecard->events[0], size-1);
	buffer[size-1] = '\0';
	for(int i = 1; i < scorecard->loggedevents; i++) {
		strcpy(scorecard->events[i-1], scorecard->events[i]);
	}
	scorecard->loggedevents -= 1;
	sem_post(&scorecard->mutex);
	return true;
}