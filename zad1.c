#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
int main()
{
	int potoki[2];
	int surowiec =0;
	int towar = 0;
	pid_t pid;
	int pip;
	int i;
	FILE *filw;
	FILE *filr;
	pip=pipe(potoki);

		if(pip==0)
		{
		wait(0);

		pid = fork();
		if(pid==-1) {exit(0);}
		if(pid==0)
		{ 
			printf("Konsument \n");
			filw=fopen("schowek.txt","a");
			if(!filw)
			{
				printf("Nie można otworzyc pliku\n");
				exit(0);
			}

			close(potoki[1]);
				while(read(potoki[0], &towar, sizeof(int)) > 0)
				{
				printf("Odebrano towar: %d", towar);
				fprintf(filw, "%d\n",towar);
				sleep(rand()%4);
				}
				close(potoki[0]);
				fclose(filw);
			}
			else
			{
				printf("Producent \n");

				filr=fopen("magazyn.txt","r");
				if(!filr)
				{
					printf("Nie można otworzyc pliku\n");
					exit(0);
				}

				close(potoki[0]);
				for(i=0;i<10;i++)
				{
					fscanf(filr,"%d",&surowiec);
					dane_przetworzone = write(potoki[1], &surowiec, sizeof(int));
					printf("Wyslano surowiec: %d \n",surowiec);
					sleep(rand()%2);
				}
				close(potoki[1]);
				fclose(filr);
			}
		}
		else
		{
			printf("Blad przy tworzeniu potoku\n");
			exit(0);
		}
		return 0;
}