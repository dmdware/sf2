

#include "thread.h"

Thread g_thread[THREADS];

int ThreadFun(void* param)
{
	Thread* t = (Thread*)param;
	const int tin = t - g_thread;

#if 0
	char file[32];
	sprintf(file, "thread%d.txt", tin);
	FILE* fp = fopen(file, "w");

	fprintf(fp, "start ");
#endif

	ecbool quit = ecfalse;

	while(ectrue)
	{
		SDL_mutexP(t->mutex);
		quit = t->quit;
		SDL_mutexV(t->mutex);

		if(quit)
			break;

		//work
	}
	
#if 0
	fprintf(fp, "end ");
	fclose(fp);
#endif

	return 0;
}