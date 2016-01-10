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
	srand(time(NULL));
		if(pip==0)
		{
		pid = fork();
		if(pid==-1) {exit(0);}
		//****** Konsument   - proces potomny ******//
		if(pid==0)
		{ 
			filw=fopen("schowek.txt","a");
			if(!filw)
			{
				printf("Nie można otworzyc pliku\n");
				exit(0);
			}

			close(potoki[1]);
				while(read(potoki[0], &towar, sizeof(int)) > 0)
				{
				
				printf(" Odebrano towar: %d ", towar);
				fprintf(filw, "%d\n",towar);
				sleep(rand()%2);
				}
				close(potoki[0]);
				fclose(filw);
			}
			//****** Producent   - proces macierzysty ******//
			else
			{
				filr=fopen("magazyn.txt","r");
				if(!filr)
				{
					printf("Nie można otworzyc pliku\n");
					exit(0);
				}

				close(potoki[0]);
				while(fscanf(filr,"%d",&surowiec) != EOF)
				{
					write(potoki[1], &surowiec, sizeof(int));
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