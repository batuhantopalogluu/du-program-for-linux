/*
+ 151044026  BATUHAN TOPALOÐLU + 
							   */
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#include <fcntl.h> 
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
extern int errno; 
#define startLenOfPathArr 256       // içinde 256'den daha fazla directory barından bir path ile çalışılacaksa bu değer arttırılmalıdır. 


int sizepathfun (char *path);
int postOrderApply (char *path, int pathfun (char *path1));



int sizeZ(char ** arr,char * path,int i);
int main(int argc, char *argv[]) {
			
	/* Hatalý parametreler ile run edilmeye çalýþýlýrsa ekrana Usage basýlýr.*/
	if ((argc < 2 || argc > 3) || (argc == 3 && (strcmp(argv[1],"-z\0") != 0))) {
		fprintf(stderr, "\nUsage:  \n \t%s directory_name  \n   -z \n\t%s -z directory_name \n\t For calculating total size\n", argv[0],argv[0]);
		return 1;
	}
	pid_t cpid; 	
	int Z ;  /*-z nin girilip girilmediğini kontrol etmek kullanılan değişken*/
	
	int fds = open("151044026sizes.txt",O_CREAT | O_TRUNC,S_IWUSR | S_IRUSR);	/*işlemler sırasında kullanılacak dosya isminde hali hazırda varsa o yok ediliğ yenisi açılır.*/		
	if (fds ==-1){ 																			
     	printf("Error Number % d\n", errno);  
    	perror("Program");                  
  	} 
	
	close(fds);

	if(argc == 3) /*- z li çağrı*/
		Z = 1;
	else 
		Z = 0;

	if((cpid = fork()) == 0){  						 /*child process'in çalýþtýracaðý kýsým */
		if(argc == 2)	                            /* -z parametresi eklenmeme durumu.*/
			postOrderApply(argv[1],sizepathfun);
	
		else                                        /* -z parametresi eklenme durumu.*/
			postOrderApply(argv[2],sizepathfun);		
	}
	
	else {                                           /*main process kendi oluþturuduðu child'ýný bekler ve o gelene kadar iþlem yapmaz*/
		
		int statu0;
		waitpid(cpid,&statu0,0);

		if (WEXITSTATUS(statu0)){ 
			//free(str);
	        int exitStatu = WEXITSTATUS(statu0);         
	        printf("Exit statu is :  %d\n",exitStatu);
	        exit(-1); 		                                     
  		} 
		
		int fd = open("151044026sizes.txt",O_RDONLY ,S_IWUSR | S_IRUSR);   /*veriler alýnmak üzere 151044026sizes.txt dosyasý açýlýr*/

		if (fd ==-1){ 												       /*dosyanýn açýlamama hata kontrolü*/
     	    printf("Error Number % d\n", errno);  
          	printf("! Kayıt dosyası bulunamadı !\n");
    	    perror("Program");                  
  	    } 

		int statu;
		char tee[1] = "0";                                           /*read return deðerini tutan deðiþken*/
		int pidCounter = 0; 								/*Child process sayýsýný tutan counter*/		
		
		int f;
		char *bufi =  malloc(264);							/*dosyadan okunan line'larý tutan deðiþken*/
		for(f = 0;f<264;f++)
				bufi[f] = '0';
		char *bufiBackUp =  malloc(264);
		for(f = 0;f<264;f++)
				bufiBackUp[f] = '0';
		char **pathsArr = malloc(startLenOfPathArr * sizeof(char*));
		int i = 0;
		int ii;
		printf("PID     SIZE    PATH \n");
	
		for(;;){											/* dosyadaki veriler satýr satýr okunur,*/
															/* satýrýn ilk elemaný o verinin bir directory'e mi yoksa special file'a mý ait olduðuna */
			statu = read(fd,bufi,264);                      /* dair flag taþýr.*/
			if(statu <= 0)        	                        
				break;                                      /*dosyanýn sonuna gelindiginde loop kýrýlýr.*/
			
			read(fd,tee,1);  
			if(Z == 1){
				
				if(bufi[0] == 'N'){
				
					pathsArr[i] = malloc(264);
	  				
	  				strcpy(pathsArr[i],bufi);  		
					strcpy(bufiBackUp,bufi);
					sizeZ(pathsArr,bufiBackUp,i);
					
					i++;
				}
				else
					printf("%s\n",bufi+1);			                              				     
   			}
			
			else printf("%s\n",bufi+1);                            /* satýr ekrana basýlýr flag atlanýr.*/

			if(bufi[0] == 'N')                              /*bu flag'i kullanarak alt directory sayýsý üzerinde oluþturulan child sayýsý hesaplanýr.*/
				pidCounter++;							  
			
		}	
		
		printf("%d child processes created. Main process is %d\n",pidCounter,getpid());
		free(bufi);
		                                          /*işlemleri tamamlandığı için alınan yer free edilir ve dosya kapatılırr.*/
		close(fd);
		for( ii = 0; ii<i; ii++) // pathArr için alýnan yerler býrakýlýr
  			free(pathsArr[ii]);
		free(pathsArr); 
		free(bufiBackUp);
		
	}
	return 0;
}


int postOrderApply (char *path, int pathfun (char *path1)){

	struct flock lock;							/*dosyayý kilitleme iþlemi için gerekli*/
	DIR *dir;
	struct dirent *dptr;
	struct stat statbuf;	
	int size = 0;
	char* str = (char *)malloc(sizeof(char) *256);
  	size = 0; /* current directory'deki Regular dosyalarýn boyutlarýnýn toplamýný tutan deðiþken*/
	
	if ((dir = opendir(path)) == NULL) { /*path i verilen directoryi açýlamazsa hata mesajý verilip -1 return edilir.*/
		perror ("Failed to open directory");
		free(str);
		exit (-1);
	}
	
	strcpy(str,path);
	
	int cpidChild;
	int isThereChild = 0;
	
	while ((dptr = readdir(dir)) != NULL){ /* directory altýndaki dosyalarýn birer birer gezildiði loop*/
		
		strcat (str,"/");			
		strcat (str,dptr->d_name); /* bulunan directory ismi / karakteri ile parametre olarak gelen path'e eklnir.*/
		stat(str, &statbuf);
		long sizeReg = pathfun(str);
		/* Bulunan eleman bir directory ise ve bu directory bir üst dizin veya bulunulan dizinin kendisi dýþýnda bir dizinse
		 depth first search yapýldýðýndan inilebildiðince alt dizine inilir.*/
		if(S_ISDIR(statbuf.st_mode) && strcmp(dptr->d_name,".\0") != 0 && strcmp(dptr->d_name,"..\0") !=0 ){
					
			isThereChild =1;
				
			if((cpidChild = fork()) == 0){  /*eğer alt dizinde directory bulunursa fork yapılır ve child proceses oradan devam eder ana
											process onun dönmesini bekler.*/
				postOrderApply(str,sizepathfun);
			}			
			else {
	
				int statu;
				waitpid(cpidChild,&statu,0); 	              

				if (WEXITSTATUS(statu)){ 
					free(str);
			        int exitStatu =WEXITSTATUS(statu);         
			        printf("Exit statu is :%d\n",exitStatu); 		                                     
		  		} 
			}
		}

		else if(sizeReg != -1){
			size += sizeReg;
		}

		if(dptr->d_type != DT_REG && dptr->d_type != DT_DIR){
	
			int SpecialFD = open("151044026sizes.txt",O_RDWR | O_CREAT | O_APPEND,S_IWUSR | S_IRUSR);    /*normalde directoryden çýkarken dosyaya yazma yaptýðým için */	
			
			if (SpecialFD ==-1){ 																			//X
   			  	printf("Error Number % d\n", errno);  
    			free(str);
    			close(SpecialFD); 
    			exit(-1);                
  			} 																							 
			
			char * forSpcl =  malloc(264 * sizeof(char));
			
			memset(&lock,0,sizeof(lock)); 				/*dosya o sýrada baþka bir iþlemin araya girmemesi için kilitlenir.*/
			lock.l_type =F_WRLCK;			            
			fcntl(SpecialFD,F_SETLKW,&lock);            

			strcpy(forSpcl,"Special file ");
			
			strcat(forSpcl,dptr->d_name);											/*dosya adý önüne ekrana basarken kullanýlacak olan string eklenir*/
			char strSpcl[264];
			int i;
			for(i = 0;i<264;i++)
				strSpcl[i] = 0;
			sprintf(strSpcl,"%c%d\t%s",'S',(int)getpid(),forSpcl);				/*dosyaya yazýlacak satýr strSpcl(string special) üzerinde birleþtirilir.*/
			
			
			write(SpecialFD,strSpcl,sizeof(strSpcl));
			write(SpecialFD,"\n",1);

			lock.l_type = F_UNLCK;						/*dosya ile iþimiz bittiði için üzerindeki kilit kaldýrýlýr.*/
			fcntl(SpecialFD,F_SETLKW,&lock);

			close(SpecialFD);				/*dosya kapatılır ve alınan yerler free edilir.*/
			free(forSpcl); 
			                  
			size -= pathfun(str);
			
		}	
		strcpy(str,path); /*üzerinde iþlem yapýlan kýsým pathten çýkarýlýr.*/
	}

	if(isThereChild == 0 || waitpid(cpidChild,NULL,0)){    /*bu processin cocuðu varsa beklenir yoksa veriler dosyaya yazýlýr.*/

		int fdr = open("151044026sizes.txt",O_RDWR | O_CREAT | O_APPEND,S_IWUSR | S_IRUSR); 	
		
		memset(&lock,0,sizeof(lock)); 				/*dosya o sýrada baþka bir iþlemin araya girmemesi için kilitlenir.*/
		lock.l_type =F_WRLCK;			            
		fcntl(fdr,F_SETLKW,&lock);   
		char strrb[264];
		int i;
		for(i = 0;i<264;i++)
			strrb[i] = 0;
		sprintf(strrb,"%c%d\t%d\t%s",'N',(int)getpid(),(int)size,str);
		write(fdr,strrb,sizeof(strrb));
		write(fdr,"\n",1);

		lock.l_type = F_UNLCK;						/*dosya ile iþimiz bittiði için üzerindeki kilit kaldýrýlýr.*/
		fcntl(fdr,F_SETLKW,&lock);
		
		close(fdr);
	}
	while (closedir(dir) == -1);
	free(str);
	exit(0);
}

int sizepathfun (char *path){

	struct stat statbuf;
	
	if(stat(path, &statbuf) != -1 && S_ISREG(statbuf.st_mode))
		return statbuf.st_size;
	else 
		return -1;
}

int sizeZ(char ** arr,char * path,int i){    /* -z parametresi girildiðinde gelen path'e sahip directory'nin size hesaplanýrken 
											alt direstorylerinin de size ý hesaba katýlýr. Bunun için üzerinde iþlem yapýlan  path
											ondan önce varsa kaydedilmiþ olan directorylerin pathleriiçinde aranýr, eðer bir alt directorysi 
											varsa derin öncelikli dolaþtýðýmýz için zaten ondan önce kayýt edilmiþ olacaðýndan bulunur ve onun 
											size'ý da toplam size'a eklenir.*/

	char * pathInArr =  malloc(264);
	int f;
	for(f = 0;f<264;f++)
				pathInArr[f] = 0;
	const char  *s = "\t";
	char *tArr;
	char *tPath;
	
	tPath = strtok(path, s);
	printf("%s\t", tPath+1);
	tPath = strtok(NULL, s);
	
	int pathSize = atoi(tPath);
	tPath = strtok(NULL, s);

	int k;
	int tmpSize = 0;
	int totalSize = pathSize;
	
	for(k = 0;k<i;k++){

		strcpy(pathInArr,arr[k]);

		tArr = strtok(pathInArr,s);
		tArr = strtok(NULL, s);
		tmpSize = atoi(tArr);
		tArr = strtok(NULL, s);

		if(strstr(tArr,tPath)!= NULL){
			
			totalSize += tmpSize;
		}
	}	

	printf("%d\t%s\n", totalSize,tPath);
	free(pathInArr);
	
	return totalSize;
}
	
