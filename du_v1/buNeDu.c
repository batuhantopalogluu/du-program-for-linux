#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>


int sizepathfun (char *path);
int postOrderApply (char *path, int pathfun (char *path1));

int Z; /* -z parametresinin girilip girilmediğini tutmak için kullanılan değişken.
	    Tek bir değişken için kütüphane import etmek istemediğim için bool yerine int kullanıyorum.*/

int main(int argc, char *argv[]) {
		
	/* Hatalı parametreler ile run edilmeye çalışılırsa ekrana Usage basılır.*/
	if ((argc < 2 || argc > 3) || (argc == 3 && strcmp(argv[1],"-z") != 0)) {
		fprintf(stderr, "\nUsage:  \n \t%s directory_name  \n   -z \n\t%s -z directory_name \n\t For calculating total size\n", argv[0],argv[0]);
		return 1;
	}
	/* -z parametresiz koşulu*/
	if(argc == 2){
		Z = 0;
		postOrderApply(argv[1],sizepathfun);
	}
	/* -z parametresi eklenme durumu.*/
	else{ 
		Z = 1;
		postOrderApply(argv[2],sizepathfun);
	}
	
return 0;

}

/* depth first search mantığı ile yaptım ben.*/
int postOrderApply (char *path, int pathfun (char *path1)){

	DIR *dir;

	

	struct dirent *dptr;
	struct stat statbuf;	
	int size = 0;
	int rtnValue = 0;
	char* str = (char *)malloc(sizeof(char) *256);
  	size = 0; /* current directory'deki Regular dosyaların boyutlarının toplamını tutan değişken*/
	
	if ((dir = opendir(path)) == NULL) { /*path i verilen directoryi açılamazsa hata mesajı verilip -1 return edilir.*/
		perror ("Failed to open directory");
		return -1;
	}
	strcpy(str,path);

	while ((dptr = readdir(dir)) != NULL){ /* directory altındaki dosyaların birer birer gezildiği loop*/
		
		strcat (str,"/");			
		strcat (str,dptr->d_name); /* bulunan directory ismi / karakteri ile parametre olarak gelen path'e eklnir.*/

		stat(str, &statbuf);
		long sizeReg = pathfun(str);
		
		/* Bulunan eleman bir directory ise ve bu directory bir üst dizin veya bulunulan dizinin kendisi dışında bir dizinse
		 depth first search yapıldığından inilebildiğince alt dizine inilir.*/
		if(S_ISDIR(statbuf.st_mode) && strcmp(dptr->d_name,".") != 0 && strcmp(dptr->d_name,"..") !=0 ){
				
			rtnValue = postOrderApply(str,sizepathfun);
			
			if(rtnValue == -1) /*postOrderApply hata durumu dönerse çağıran fonksiyonda hata döner.*/
				return -1;
			if(Z == 1)		   /* -z parametresi ile çalıştırılması durumu.*/
				size += rtnValue;
		}

		else if(sizeReg != -1){
			size += sizeReg;
		}

		if(dptr->d_type != DT_REG && dptr->d_type != DT_DIR){
			printf("Special file \t %s \n",dptr->d_name);
			size -= pathfun(str);
		}	

		strcpy(str,path); /*üzerinde işlem yapılan kısım pathten çıkarılır.*/
	}

	printf("%d\t  %s\t \n",size,str);	
	
	while (closedir(dir) == -1); /*açılan directoryler kapatılır.*/
	
	free(str);
	return size;
}

int sizepathfun (char *path){

	struct stat statbuf;
	
	if(stat(path, &statbuf) != -1 && S_ISREG(statbuf.st_mode))
		return statbuf.st_size;
	else 
		return -1;
}