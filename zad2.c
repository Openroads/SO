#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <memory.h>

#define N 6 //rozmiar bufora

//**** Funkcje semafora *****//
int utworzSem(int key)
{
	int idSem = semget(key,1,IPC_CREAT | 0660);
	return idSem;
}

void inicjalizujSem(int idSem,int beg_val)
{
		int s = semctl(idSem,0,SETVAL,(int)beg_val);
		if(s==-1)
			printf("blad inicjalizacji semafora\n");
}

void otworzSem(int idSem)
{
	struct sembuf u;
	u.sem_num = 0;
	u.sem_flg = 0;
	u.sem_op = 1;
	int s = semop(idSem,&u,1);
	if(s==-1)
			printf("bladd otwierania semafora\n");
}

void zamknijSem(int idSem)
{	
	struct sembuf u;
	u.sem_num = 0;
	u.sem_flg = 0;
	u.sem_op = -1;
	int s = semop(idSem,&u,1);
	if(s==-1)
			printf("blad zamkniecia semafora\n");

}
void usunSem(int idSem)
{
	int s = semctl(idSem,0,IPC_RMID);
	if(s==-1)
	{
			printf("blad usuwania semafora\n");
	}
}

//**** Funkcje pamieci wspoldzielonej *****//
int Shm_Create(int idKey,int r)
{	
	int x = shmget(idKey,r,IPC_CREAT | 0666 | IPC_EXCL);
	if (x==-1)
	{
		printf("Blad przy tworzeniu pamieci  .\n");
		exit(0);
	}
	return x;
}
int Shm_Open(int idKey)
{
	int x = shmget(idKey,0,0666);
	if(x == -1)
	{
		printf("Blad przy tworzeniu pamieci  .\n");
		exit(0);
	}
	return x;
}
void* Shm_Att(int id)
{
	return shmat(id,NULL,0);
}
void Shm_Dt(void* r)
{
   shmdt(r);
}
int Shm_Delete(int id)
{
	int x = shmctl(id,IPC_RMID,0);
		if(x == -1)
	{
		printf("Blad przy tworzeniu pamieci  .\n");
		exit(0);
	}
	return x;
}

int ilosc_linii(void)
{
	FILE *fil;
	int linie=0;
	int znak;
	fil=fopen("magazyn2.txt","r");
		if(!fil)
		{
			printf("Nie można otworzyc pliku\n");
			exit(0);
		}
		while(znak!=EOF)
			{
				znak=fgetc(fil);
				if(znak == ' ' || znak=='\n')
					linie++;
			}
		fclose(fil);
	return linie;
}
struct CircularBuffer
{
int size;
int beg;
int end;
int buf[];
};

int main(void)
{
	int surowiec =0;
	int towar = 0;
	pid_t pid;
	int i;
	FILE *filw;
	FILE *filr;
	srand(time(NULL));
	int ilosc_surowca = ilosc_linii();

	int key = ftok(".",'s')+2;
	int idShm = Shm_Create(key,(N+5)*sizeof(int));

	int key1 = ftok(".",'f');
	int idSem1 = utworzSem(key1);
	inicjalizujSem(idSem1,0);

	int key2 = ftok(".",'g');
	int idSem2 = utworzSem(key2);
	inicjalizujSem(idSem2,N);

		pid = fork();
		if(pid==-1) {exit(0);}
		//****** Konsument   - proces potomny ******//
		if(pid==0)
		{ 
			filw=fopen("schowek2.txt","a");
			if(!filw)
			{
				printf("Nie można otworzyc pliku\n");
				exit(0);
			}
			
			void* wsk = Shm_Att(idShm);
			struct CircularBuffer *wskBuff = (struct CircularBuffer*)wsk;
			i=0;
			while(i<ilosc_surowca)
			{	
				zamknijSem(idSem1);	

				wskBuff->beg = i%wskBuff->size;
				printf("Odbieram towar: %d\n",wskBuff->buf[wskBuff->beg]);
				fprintf(filw, "%d\n",wskBuff->buf[wskBuff->beg]);
				i++;
				
				otworzSem(idSem2);
				sleep(rand()%4);
			}

				Shm_Dt(wsk);
				fclose(filw);
				Shm_Delete(idShm);
				//usuniecie semafora pierwszym procesem  producenta spowoduje bledy otwierania 
				usunSem(idSem2);
				usunSem(idSem1);
			}
			//****** Producent   - proces macierzysty ******//
			else
			{
				void * addr = Shm_Att(idShm);
				struct CircularBuffer *surowce;
				surowce = (struct CircularBuffer*)addr ;
				memset(surowce,0,sizeof(surowce));
				surowce->size=N;
				for (i=0;i<surowce->size;i++)
					{surowce->buf[i] = 0;}

				filr=fopen("magazyn2.txt","r");
				if(!filr)
				{
					printf("Nie można otworzyc pliku\n");
					exit(0);
				}

				i=0;
				while(fscanf(filr,"%d",&surowiec) != EOF)
				{
					zamknijSem(idSem2);

					surowce->end= i%surowce->size;
					surowce->buf[surowce->end] = surowiec;
					printf("Wysyłam surowiec: %d\n",surowce->buf[surowce->end]);
					i++;
					sleep(rand()%5);
	
					otworzSem(idSem1);
				}
				//usuwanie i zamykanie 
				Shm_Dt(addr);
				fclose(filr);
			}
		return 0;
}