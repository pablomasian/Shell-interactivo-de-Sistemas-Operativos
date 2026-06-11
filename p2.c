#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <dirent.h>
#include <libgen.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>
//Types--------------------------------------------------------------------------
#define LNULL -1
#define LMAX 4096
#define MAX 1024
#define TAMANO 2048
struct CMD{
char *nombre;
void (*func)(char **);
};
static int static1;
static int static2;
static int static3;
int global1;
int global2;
int global3;
//declaration--------------------------------------------------------------------
typedef int tPosL;
typedef struct {
    char *data[LMAX];
    tPosL lastPos;
} tList;
typedef void* tMemAddress;
typedef size_t tSize;
typedef int tMemAllocationType; //1 = malloc; 2 = mapped file; 3 = shared memory
typedef char tFileName[MAX]; //-mmap only
typedef int tFileDescriptor; //-mmap only
typedef int tMemKey; //-createshared and -shared only
typedef struct tm tTime;
typedef int tPosM;
typedef struct {
    tMemAddress memAddress;
    tSize size;
    tMemAllocationType memAllocationType;
    tFileName fileName;
    tFileDescriptor fd;
    tMemKey memKey;
    tTime time;
} tItemM;

typedef struct {
    tItemM *data[LMAX];
    tPosM lastPos;
} tListMem;
struct utsname unameData;
void Cmd_autores (char **);
void Cmd_carpeta (char **);
void Cmd_fin (char **);
void Cmd_pid (char **);
void Cmd_salir (char **);
void Cmd_bye (char **);
void Cmd_fecha (char **);
void Cmd_infosis(char **);
void Cmd_ayuda (char **);
void Cmd_hist (char **,tList *L);
void Cmd_comando(char **, tList *L,tListMem *M);
void Cmd_create (char **);
void Cmd_stat (char **);
void Cmd_list (char **);
void Cmd_delete(char **);
void Cmd_deltree (char **);
void Cmd_recurse (char **);
void Cmd_memory (char **,tListMem *L);
void Cmd_allocate (char **,tListMem *L);
void Cmd_deallocate (char **,tListMem *L);
void Cmd_memdump(char **);
void Cmd_memfill(char **);
void Cmd_i_o(char **);
int terminado=0;
static struct CMD c[]={
        {"autores",     Cmd_autores    },
        {"carpeta",     Cmd_carpeta    },
        {"pid",         Cmd_pid        },
        {"fin",         Cmd_fin        },
        {"salir",       Cmd_salir      },
        {"bye",         Cmd_bye        },
        {"fecha",       Cmd_fecha      },
        {"infosis",     Cmd_infosis    },
        {"ayuda",       Cmd_ayuda      },
        {"create",      Cmd_create     },
        {"stat",        Cmd_stat       },
        {"list",        Cmd_list       },
        {"delete",      Cmd_delete     },
        {"deltree",     Cmd_deltree    },
        {"recurse",     Cmd_recurse    },
        {"memdump",     Cmd_memdump    },
        {"memfill",     Cmd_memfill    },
        {"i-o",         Cmd_i_o        },
        {NULL,NULL}
};
int Info(char *file,bool lng,bool acc,bool link);
int dirInfo(char *dir,bool lng,bool acc,bool link,bool hid,bool reca,bool recb);
int subDir(char *dir,bool lng,bool acc,bool link,bool hid,bool reca,bool recb);
int subDir2(char *dir,bool lng,bool acc,bool link,bool hid,bool reca,bool recb);
int subDir3(char *dir,bool lng,bool acc,bool link,bool hid,bool reca,bool recb);
int deleteDir(char *dir);
char LetraTF (mode_t m);
char* convierteModo(mode_t m);
void createEmptyList(tList* L);
void createEmptyListMem(tListMem* L);
tPosL next(tPosL p, tList* L);
tPosM nextMem(tPosM p, tListMem* L);
tPosL first(tList* L);
tPosM firstMem(tListMem* L);
tPosM lastMem(tListMem* L);
bool insertItem(char *d,tList* L);
bool insertItemMem(tItemM d, tPosM p, tListMem* L);
char getItem(tPosL p, tList* L);
tItemM getItemMem(tPosM p, tListMem* L);
tListMem deleteAtPositionMem(tPosM p, tListMem* L);
void deleteList(tList* L);
void deleteListMem(tListMem* L);
void printAllListCommands(tList* L);
void printMemList(int option, tListMem* L);
void printNListCommands(int N, tList* L);
void printMemItem(tItemM item);
tPosM findMemAddress(tMemAddress memAddress, tListMem* M);
tPosM findSize(tSize size, tMemAllocationType memAllocationType, tListMem* M);
tPosM findFileName(tFileName fileName, tMemAllocationType memAllocationType, tListMem* M);
tPosM findMemKey(tMemKey key, tMemAllocationType memAllocationType, tListMem* M);
void LlenarMemoria (void *p, size_t cont, unsigned char byte);
void * ObtenerMemoriaShmget (key_t clave, size_t tam, tItemM* newItem);
void do_AllocateCreateshared (char *tr[],tListMem* M);
void * MapearFichero (char * fichero, int protection, tListMem* M);
int TrocearCadena(char* cadena, char* trozos[]);
void ProcesarEntrada(char* cadena,char* tr[],tList *L,tListMem *M);
void Recursiva (int n);
void Do_pmap (void);
void do_AllocateMmap(char *tr[], tListMem* M);
ssize_t LeerFichero (char *f, void *p, size_t cont);
ssize_t EscribirFichero (char *f, void *p, size_t cont,int overwrite);
void do_I_O_read (char *ar[]);
//functions--------------------------------------------------------------------
void createEmptyList(tList* L){
    L->lastPos=LNULL;
}
void createEmptyListMem(tListMem* L){
    L->lastPos=LNULL;
}
tPosL next(tPosL p, tList* L){
    if(p>L->lastPos)
        return LNULL;
    else
        return (p+1);
}
tPosM nextMem(tPosM p, tListMem* L){
    if(p==L->lastPos)
        return LNULL;
    else
        return (p+1);
}
tPosL first(tList* L){
    if (L->lastPos != LNULL)
        return 0;
    else
        return LNULL;
}
tPosM firstMem(tListMem* L){
    if (L->lastPos != LNULL)
        return 0;
    else
        return LNULL;
}
tPosM lastMem(tListMem* L){
    if (L->lastPos != LNULL)
        return L->lastPos;
    else
        return LNULL;
}
bool insertItem(char *d,tList* L){
    if (L->lastPos >= LMAX - 1)
        return false;
    else
    {
        L->lastPos++;
        L->data[L->lastPos]=malloc (strlen(d)+1);
        strcpy (L->data[L->lastPos],d);
        return true;
    }
}
bool insertItemMem(tItemM d, tPosM p, tListMem* L){
    tPosM i;
    if (L->lastPos >= LMAX - 1)
        return false;
    else
    {
        L->lastPos++;
        if (p == LNULL) L->data[L->lastPos] = malloc(d);
        else
        {for (i = L->lastPos; i >= p+1; i--)
            {
                L->data[i] = malloc(L->data[i-1]);
                L->data[i] = L->data[i-1];
            }
            L->data[p] = &d;
        }
        return true;
    }
}
char getItem(tPosL p, tList* L){
    return (*L->data[p]);
}
tItemM getItemMem(tPosM p, tListMem* L){

    return (*L->data[p]);
}
tListMem deleteAtPositionMem(tPosM p, tListMem* L){

    tPosM i;
    L->lastPos--;
    for(i = p; i <= L->lastPos; i++)
    {
        L->data[i] = L->data[i+1];
    }
    return *L;
}
void deleteList(tList* L){ //fix this
    L->lastPos = LNULL;
}
void deleteListMem(tListMem* L){ //and this

    L->lastPos = LNULL;
}
tPosM findMemAddress(tMemAddress memAddress, tListMem* L){
    tPosM p;
    if (L->lastPos == LNULL)
        return LNULL;
    else
    {for (p = 0; (p < L->lastPos) && (L->data[p]->memAddress != memAddress); p++)
            ;
        {if (L->data[p]->memAddress == memAddress){
                return p;}
        return LNULL;
        }
    }
}
tPosM findSize(tSize size, tMemAllocationType memAllocationType, tListMem* L){
    tPosM p;
    if (L->lastPos == LNULL)
        return LNULL;
    else
    {for (p = 0; (p <= L->lastPos) && (L->data[p]->size != size); p++)
        {if (L->data[p]->size == size){
                if(L->data[p]->memAllocationType == memAllocationType) {
                    return p;}}
         return LNULL;
        }
    }
}
tPosM findFileName(tFileName fileName, tMemAllocationType memAllocationType, tListMem* L){
    tPosM p;
    if (L->lastPos == LNULL)
        return LNULL;
    else
    {for (p = 0; (p <= L->lastPos) && (strcmp(L->data->fileName, fileName) != 0); p++)
        {if (strcmp(L->data->fileName, fileName) == 0){
                if(L->data[p]->memAllocationType == memAllocationType) {
                    return p;}}
         return LNULL;
        }
    }
}
tPosM findMemKey(tMemKey key, tMemAllocationType memAllocationType, tListMem* L){
    tPosM p;
    if (L->lastPos == LNULL)
        return LNULL;
    else
    {for (p = 0; (p < L->lastPos) && (L->data[p]->memKey != key); p++)
        {if (L->data[p]->memKey == key){
                if(L->data[p]->memAllocationType == memAllocationType) {
                    return p;}
            }
        return LNULL;
        }
    }
}
void printAllListCommands(tList* L){
    tPosL i;
    for(i = 0; i<=L->lastPos; i++){
        printf("%d: %s", i, L->data[i]);
    }
}
void printMemList(int option, tListMem* L){
    tPosM i;
    tItemM item;
     for(i = firstMem(L); i != LNULL; i = nextMem(i, L)){
         item = getItemMem(i, L);
         if(option == 0){ //print all list
            printMemItem(item);
        }
        if(option == 1){ //print elements allocated with malloc
            if(item.memAllocationType == 1){
                printMemItem(item);
            }
        }
        if(option == 2){ //print elements allocated with mmap
            if(item.memAllocationType == 2){
                printMemItem(item);
            }
        }
        if(option == 3){ //print elements allocated with createshared or shared
            if(item.memAllocationType == 3){
                printMemItem(item);
            }
        }
    }
}
void printMemItem(tItemM item){

    //Time
    char datestring[256];
    strftime(datestring, sizeof(datestring), "%a %b %d %H:%M:%S %Y", &item.time);

    if(item.memAllocationType == 1){
        printf("%p  size:%ld  malloc  %s\n", item.memAddress, item.size, datestring);
    }
    if(item.memAllocationType == 2){
        printf("%p  size:%ld  mmap  %s  (fd:%d)  %s\n", item.memAddress, item.size, item.fileName, item.fd,
               datestring);
    }
    if(item.memAllocationType == 3){
        printf("%p  size:%ld  shared memory  (key %d)  %s\n", item.memAddress, item.size, item.memKey,
               datestring);
    }
}
char LetraTF (mode_t m){
     switch (m&S_IFMT) { /*and bit a bit con los bits de formato,0170000 */
        case S_IFSOCK: return 's'; /*socket */
        case S_IFLNK: return 'l'; /*symbolic link*/
        case S_IFREG: return '-'; /* fichero normal*/
        case S_IFBLK: return 'b'; /*block device*/
        case S_IFDIR: return 'd'; /*directorio */ 
        case S_IFCHR: return 'c'; /*char device*/
        case S_IFIFO: return 'p'; /*pipe*/
        default: return '?'; /*desconocido, no deberia aparecer*/
     }
}
char * ConvierteModo (mode_t m){
    static char permisos[12];
    strcpy (permisos,"---------- ");
    
    permisos[0]=LetraTF(m);
    if (m&S_IRUSR) permisos[1]='r';    /*propietario*/
    if (m&S_IWUSR) permisos[2]='w';
    if (m&S_IXUSR) permisos[3]='x';
    if (m&S_IRGRP) permisos[4]='r';    /*grupo*/
    if (m&S_IWGRP) permisos[5]='w';
    if (m&S_IXGRP) permisos[6]='x';
    if (m&S_IROTH) permisos[7]='r';    /*resto*/
    if (m&S_IWOTH) permisos[8]='w';
    if (m&S_IXOTH) permisos[9]='x';
    if (m&S_ISUID) permisos[3]='s';    /*setuid, setgid y stickybit*/
    if (m&S_ISGID) permisos[6]='s';
    if (m&S_ISVTX) permisos[9]='t';
    
    return permisos;
}
int Info(char *path,bool lng,bool acc,bool link){
	struct stat p;
	struct tm tm;
	if(lstat (path,&p)==-1) {
		printf("%s\n",strerror(errno));	
		return 0;}
	struct passwd *user=getpwuid(p.st_uid);
	struct group *group=getgrgid(p.st_gid);
	char date[MAX],*permissions = ConvierteModo(p.st_mode), symlink [MAX]="",*file=basename(path);	
	if (lng==false) printf("%9ld  %s\n",p.st_size,file);
	else{
        if(acc==true) localtime_r(&p.st_atime,&tm); //atime ->last acces time, mtime ->last modification time
        else localtime_r(&p.st_mtime,&tm);
        strftime(date, MAX, "%Y/%m/%d-%H:%M", &tm);//st_nlink number of hard links to file, st_ino serial number of file
        printf("%s%4ld(%ld)\t%s\t%s\t%s%9ld %s\n",date, p.st_nlink, p.st_ino,user->pw_name, group->gr_name, permissions, p.st_size, file) ;
        if(link==true && (readlink(file, symlink, MAX)!=-1)) printf(" ---> %s\n", symlink);
		}
	return 0;
}
int subDir2(char *dir,bool lng,bool acc,bool link,bool hid,bool reca,bool recb){
	DIR *directory;
	struct dirent *tt;
	struct stat p;
	char copy[MAX];
	if((directory=opendir(dir))==NULL) printf("%s\n",strerror(errno));
	while((tt=readdir(directory))!=NULL){
		if (hid==false && tt->d_name[0]=='.')continue;
		if (!strcmp(tt->d_name,".")||!strcmp(tt->d_name,"..")) continue;
		sprintf(copy, "%s/%s", dir, tt->d_name);
		stat(copy,&p);
		if(S_ISDIR(p.st_mode)){if(subDir2(copy,lng,acc,link,hid,reca,recb)==-1)printf("%s\n",strerror(errno));}
	}
	
	if(subDir3(dir,lng,acc,link,hid,reca,recb)) printf("%s\n",strerror(errno));
	closedir(directory);
	return 0;	
}
int subDir3(char *dir,bool lng,bool acc,bool link,bool hid,bool reca,bool recb){
	DIR *directory;
	struct dirent *tt;
	char copy[MAX];
	printf("************%s\n",dir);
	if((directory=opendir(dir))==NULL) printf("%s\n",strerror(errno));
	while((tt=readdir(directory))!=NULL){
		sprintf(copy, "%s/%s", dir, tt->d_name);
		if (hid==false && tt->d_name[0]=='.')continue;
		if (Info(copy,lng,acc,link)) printf("%s\n",strerror(errno));
		}
	closedir(directory);
	return 0;
	}
int subDir(char *dir,bool lng,bool acc,bool link,bool hid,bool reca,bool recb){
	DIR *directory;
	struct dirent *tt;
	struct stat p;
	char copy[MAX];
	if((directory=opendir(dir))==NULL) printf("%s\n",strerror(errno));
	while((tt=readdir(directory))!=NULL){
		if (hid==false && tt->d_name[0]=='.')continue;
		if (!strcmp(tt->d_name,".")||!strcmp(tt->d_name,"..")) continue;
		sprintf(copy, "%s/%s", dir, tt->d_name);
		stat(copy,&p);
		if(S_ISDIR(p.st_mode)){if(dirInfo(copy,lng,acc,link,hid,reca,recb)==-1)printf("%s\n",strerror(errno));}
	}
	closedir(directory);
	return 0;	
}
int dirInfo(char *dir,bool lng,bool acc,bool link,bool hid,bool reca,bool recb){
	DIR *directory;
	struct dirent *tt;
	struct stat p;
	char copy[MAX];
	if(stat(dir,&p)==-1) return -1;
	if(S_ISDIR(p.st_mode)){
       if (recb==true) {if (subDir2(dir,lng,acc,link,hid,reca,recb)) printf("%s\n",strerror(errno));}
	   if(recb==false && reca==true) {
		  printf("************%s\n",dir);
	      if((directory=opendir(dir))==NULL) printf("%s\n",strerror(errno));
	      while((tt=readdir(directory))!=NULL){
	          sprintf(copy, "%s/%s", dir, tt->d_name);
	          if (hid==false && tt->d_name[0]=='.')continue;
	          if (Info(copy,lng,acc,link)) printf("%s\n",strerror(errno));
		  }
	      closedir(directory);
		  if(subDir(dir,lng,acc,link,hid,reca,recb)==-1)printf("%s\n",strerror(errno));
		}
	    if(recb==false && reca==false){
		   printf("************%s\n",dir);
	       if((directory=opendir(dir))==NULL) printf("%s\n",strerror(errno));
	       while((tt=readdir(directory))!=NULL){
		      sprintf(copy, "%s/%s", dir, tt->d_name);
		      if (hid==false && tt->d_name[0]=='.')continue;
		      if (Info(copy,lng,acc,link)) printf("%s\n",strerror(errno));
		   }
	       closedir(directory);
		}
	}
	else {if (Info(dir,lng,acc,link)) printf("%s\n",strerror(errno));}
	return 0;
}
int deleteDir(char *dir){
 DIR *directory;
 struct dirent *tt;
 struct stat p;
 char copy[MAX];
 directory=opendir(dir);
	while((tt=readdir(directory))!=NULL){
		strcpy(copy,dir);
		strcat(strcat(copy,"/"),tt->d_name);
		if (!strcmp(tt->d_name,".")||!strcmp(tt->d_name,"..")) continue;
		stat(copy,&p);
		if(S_ISDIR(p.st_mode)) deleteDir(copy);
		remove(copy);
		}
		closedir(directory);
return 0;
}
void Recursiva (int n){
  char automatico[TAMANO];
  static char estatico[TAMANO];

  printf ("parametro:%3d(%p) array %p, arr estatico %p\n",n,&n,automatico, estatico);

  if (n>0)
    Recursiva(n-1);
}
void Do_pmap (void) { /*sin argumentos*/
 pid_t pid;       /*hace el pmap (o equivalente) del proceso actual*/
   char elpid[32];
   char *argv[4]={"pmap",elpid,NULL};
   
   sprintf (elpid,"%d", (int) getpid());
   if ((pid=fork())==-1){
      perror ("Imposible crear proceso");
      return;
      }
   if (pid==0){
      if (execvp(argv[0],argv)==-1)
         perror("cannot execute pmap (linux, solaris)");
         
      argv[0]="procstat"; argv[1]="vm"; argv[2]=elpid; argv[3]=NULL;   
      if (execvp(argv[0],argv)==-1)/*No hay pmap, probamos procstat FreeBSD */
         perror("cannot execute procstat (FreeBSD)");
         
      argv[0]="procmap",argv[1]=elpid;argv[2]=NULL;    
            if (execvp(argv[0],argv)==-1)  /*probamos procmap OpenBSD*/
         perror("cannot execute procmap (OpenBSD)");
         
      argv[0]="vmmap"; argv[1]="-interleave"; argv[2]=elpid;argv[3]=NULL;
      if (execvp(argv[0],argv)==-1) /*probamos vmmap Mac-OS*/
         perror("cannot execute vmmap (Mac-OS)");      
      exit(1);
  }
  waitpid (pid,NULL,0);
}
void LlenarMemoria (void *p, size_t cont, unsigned char byte){
  unsigned char *arr=(unsigned char *) p;
  size_t i;

  for (i=0; i<cont;i++)
		arr[i]=byte;
}
void * ObtenerMemoriaShmget (key_t clave, size_t tam, tItemM* newItem){
    void * p;
    int aux,id,flags=0777;
    struct shmid_ds s;

    if (tam)     /*tam distito de 0 indica crear */
        flags=flags | IPC_CREAT | IPC_EXCL;
    if (clave==IPC_PRIVATE)  /*no nos vale*/
        {errno=EINVAL; return NULL;}
    if ((id=shmget(clave, tam, flags))==-1)
        return (NULL);
    if ((p=shmat(id,NULL,0))==(void*) -1){
        aux=errno;
        if (tam)
             shmctl(id,IPC_RMID,NULL);
        errno=aux;
        return (NULL);
    }
    shmctl (id,IPC_STAT,&s);
    newItem->size=s.shm_segsz;
 /* Guardar en la lista   InsertarNodoShared (&L, p, s.shm_segsz, clave); */
    return (p);
}
void do_AllocateCreateshared (char *tr[],tListMem *M){
   key_t cl;
   size_t tam;
   void *p;
   tItemM newItem;
   
   if (tr[0]==NULL || tr[1]==NULL) {
		printMemList(3,M);
		return;
   }
  
   cl=(key_t)  strtoul(tr[0],NULL,10);
   newItem.memKey=cl;
   tam=(size_t) strtoul(tr[1],NULL,10);
   if (tam==0) {
	printf ("No se asignan bloques de 0 bytes\n");
	return;
   }
   if ((newItem.memAddress=ObtenerMemoriaShmget(cl,tam,&newItem))!=NULL)
		{printf ("Asignados %lu bytes en %p\n",(unsigned long) tam, newItem.memAddress);
         newItem.memAllocationType = 3;
         time_t t = time(NULL);
         struct tm tm = *localtime(&t);
         newItem.time = tm;
         insertItemMem(newItem, LNULL, M);
          }
   else
		printf ("Imposible asignar memoria compartida clave %lu:%s\n",(unsigned long) cl,strerror(errno));
}
void * MapearFichero (char * fichero, int protection, tListMem *M){
    int df, map=MAP_PRIVATE,modo=O_RDONLY;
    struct stat s;
    void *p;
    tItemM newItem;
     strcpy(newItem.fileName, fichero);
    if (protection&PROT_WRITE)
          modo=O_RDWR;
    if (stat(fichero,&s)==-1 || (df=open(fichero, modo))==-1)
          return NULL;
    if ((p=mmap (NULL,s.st_size, protection,map,df,0))==MAP_FAILED)
           return NULL;
    newItem.size=s.st_size; 
    newItem.memAllocationType = 2;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    newItem.time = tm;  
    newItem.fd=df;
    newItem.memAddress =p;
    insertItemMem(newItem,LNULL,M);       
/* Guardar en la lista    InsertarNodoMmap (&L,p, s.st_size,df,fichero); */
    return p;
}
void do_AllocateMmap(char *arg[], tListMem *M){ 

     char *perm;
     void *p;
     int protection=0;
     
     if (arg[0]==NULL)
            {printMemList(0,M); return;}
     if ((perm=arg[1])!=NULL && strlen(perm)<4) {
            if (strchr(perm,'r')!=NULL) protection|=PROT_READ;
            if (strchr(perm,'w')!=NULL) protection|=PROT_WRITE;
            if (strchr(perm,'x')!=NULL) protection|=PROT_EXEC;
     }
     if ((p=MapearFichero(arg[0],protection,M))==NULL)
             perror ("Imposible mapear fichero");
     else
             printf ("fichero %s mapeado en %p\n", arg[0], p);
}
ssize_t LeerFichero (char *f, void *p, size_t cont){
   struct stat s;
   ssize_t  n;  
   int df,aux;
   if (stat (f,&s)==-1 || (df=open(f,O_RDONLY))==-1)return -1;
   if (cont==-1) cont=s.st_size;  /* si pasamos -1 como bytes a leer lo leemos entero*/
   if ((n=read(df,p,cont))==-1){
	aux=errno;
	close(df);
	errno=aux;
	return -1;
   }
   close (df);
   return n;
}
ssize_t EscribirFichero (char *f, void *p, size_t cont,int overwrite){

   ssize_t  n;
   int df,aux, flags=O_CREAT | O_EXCL | O_WRONLY;

   if (overwrite)
	flags=O_CREAT | O_WRONLY | O_TRUNC;

   if ((df=open(f,flags,0777))==-1)
	return -1;

   if ((n=write(df,p,cont))==-1){
	aux=errno;
	close(df);
	errno=aux;
	return -1;
   }
   close (df);
   return n;
}
void do_I_O_read (char *ar[]){

   void *p;
   size_t cont=-1;
   ssize_t n;
   if (ar[0]==NULL || ar[1]==NULL){
	printf ("faltan parametros\n");
	return;
   }
   p = (void *)strtoull(ar[1], NULL, 16); /*convertimos de cadena a puntero*/
   if (ar[2]!=NULL)
	cont=(size_t) atoll(ar[2]);

   if ((n=LeerFichero(ar[0],p,cont))==-1)
	perror ("Imposible leer fichero");
   else
	printf ("leidos %lld bytes de %s en %p\n",(long long) n,ar[0],p);
}
void do_I_O_write (char *ar[]){
	void *p;
   size_t cont=-1;
   ssize_t n;
   int overwrite=0;
   if (ar[0]==NULL){
	printf ("faltan parametros\n");
	return;
   }
   if(strcmp(ar[0],"-o")) {
	   overwrite=1;
	   if(ar[1]==NULL|| ar[2]==NULL){
	        printf ("faltan parametros\n");
	       return;}
   p = (void *)strtoull(ar[1], NULL, 16); /*convertimos de cadena a puntero*/
	  if (ar[3]!=NULL)
	cont=(size_t) atoll(ar[2]); 
   if ((n=EscribirFichero(ar[0],p,cont,overwrite))==-1)
	perror ("Imposible leer fichero");
   else
	printf ("sobrescritos %lld bytes de %s en %p\n",(long long) n,ar[0],p);
   }
   else {
	   if (ar[1]==NULL){
	printf ("faltan parametros\n");
	return;
   }
   p = (void *)strtoull(ar[1], NULL, 16);  /*convertimos de cadena a puntero*/
   if (ar[2]!=NULL)
	cont=(size_t) atoll(ar[2]);

   if ((n=EscribirFichero(ar[0],p,cont,overwrite))==-1)
	perror ("Imposible leer fichero");
   else
	printf ("escritos %lld bytes de %s en %p\n",(long long) n,ar[0],p);
	   }
   
	}
void Cmd_carpeta(char *tr[]){
    char dir[MAX];
    if (tr[0]==NULL) printf("%s\n",getcwd(dir,MAX));
    else if (chdir (tr[0])!=0){
        printf("Imposible cambiar directorio: %s\n",strerror(errno));} 
    
}
void Cmd_fin (char *tr[]){
    terminado=1;
}
void Cmd_salir (char *tr[]){
    terminado=1;
}
void Cmd_bye(char *tr[]){
    terminado=1;
}
void Cmd_autores(char *tr[]){
    if (tr[0]!=NULL)
    {
    if(!strcmp(tr[0],"-l"))  {printf("pablo.masian.carro\nr.canabalf\n");}
    else if(!strcmp(tr[0],"-n")) {printf("Pablo Masián Carro\nRaquel Canabal Fuentes\n");}
    }
    else {printf("pablo.masian.carro\nr.canabalf\n");
        printf("Pablo Masián Carro\nRaquel Canabal Fuentes\n");}
    return;
}
void Cmd_pid(char *tr[]){
    if (tr[0]!=NULL && !strcmp (tr[0],"-p"))
        printf("Parent process pid: %d\n",getppid());
    else printf("Process pid: %d\n",getpid());
}
void Cmd_fecha(char *tr[]){
    if (tr[0]==NULL)
    {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        printf("%02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
        printf("%02d-%02d-%02d \n",tm.tm_mday , tm.tm_mon + 1, tm.tm_year + 1900);
        return;
    }
    if(!strcmp(tr[0],"-d")){
        time_t t = time(NULL);
    struct tm tm = *localtime(&t);
        printf("%02d-%02d-%02d \n",tm.tm_mday , tm.tm_mon + 1, tm.tm_year + 1900);}
    if(!strcmp(tr[0],"-h"))
    {time_t t = time(NULL);
    struct tm tm = *localtime(&t);
        printf("%02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);}
}
void Cmd_hist(char *tr[], tList *L){
    if (tr[0]==NULL)
    {
        printAllListCommands(L);
        return;
    }
    else if(!strcmp(tr[0],"-c"))
        deleteList(L);
    else{
        tPosL p;
        p=atoi(strtok(*tr, "-"));
        printNListCommands(p,L); 
    }
}
void Cmd_comando(char *tr[],tList *L,tListMem *M){
if (tr[0]==NULL) printAllListCommands(L);
else{tPosL p;
p=atoi(strtok(*tr, " "));
if(p<0||p>L->lastPos) {
	printf("No hay elemento %d en el histórico\n",p);
	return;}
printf("Ejecutando hist (%d): %s",p,L->data[p]);
if(L->data[p]!=NULL) 
{TrocearCadena(L->data[p],tr);
	ProcesarEntrada(L->data[p],tr,L,M);}
}}
void Cmd_infosis(char *tr[]){
	uname(&unameData);
    printf("%s (%s), OS: %s-%s %s \n",unameData.nodename,unameData.machine,unameData.sysname,unameData.release,unameData.version);
}
void Cmd_ayuda(char *tr[]){
    if (tr[0]==NULL)
    {printf("autores [-l|-n]\npid[-p]\ncarpeta\nfecha[-d|-h]\nhist[-c|-N]\ncomando N\ninfosis\nfin\nsalir\nbye\nlist[-reca|-recb|-long|-hid|-acc|-link]\nstat[-long|-acc|-link]\ndelete\ndeltree\ncreate[-f]\n");
        return;}
    else if(!strcmp(tr[0],"autores")) printf("autores [-n|-l]	Muestra los nombres y/o logins de los autores\n");
    else if(!strcmp(tr[0],"pid")) printf("pid [-p]    Muestra el pid del shell o de su proceso padre\n");
    else if(!strcmp(tr[0],"carpeta")) printf("carpeta [dir]	Cambia (o muestra) el directorio actual del shell\n");
    else if(!strcmp(tr[0],"fecha")) printf("fecha [-d|-h	Muestra la fecha y o la hora actual\n");
    else if(!strcmp(tr[0],"hist")) printf("hist [-c|-N]	Muestra (o borra)el historico de comandos\n\t\t-N: muestra los N primeros\n\t\t-c: borra el historico\n");
    else if(!strcmp(tr[0],"infosis")) printf("infosis 	Muestra informacion de la maquina donde corre el shell\n");
    else if(!strcmp(tr[0],"exit")) printf("exit 	Termina la ejecucion del shell\n");
    else if(!strcmp(tr[0],"salir")) printf("salir 	Termina la ejecucion del shell\n");
    else if(!strcmp(tr[0],"bye")) printf("bye 	Termina la ejecucion del shell\n");
    else if(!strcmp(tr[0],"comando")) printf("comando [-N]	Repite el comando N (del historico)\n");
    else if(!strcmp(tr[0],"list")) printf("list [-reca] [-recb] [-hid][-long][-link][-acc] n1 n2 ..	lista contenidos de directorios\n\t\t-hid: incluye los ficheros ocultos\n\t\t-reca: recursivo (antes)\n\t\t-recb: recursivo (despues)\n\t\tresto parametros como stat\n");
    else if(!strcmp(tr[0],"stat")) printf("stat [-long][-link][-acc] name1 name2 ..	lista ficheros;\n\t\t-long: listado largo\n\t\t-acc: acesstime\n\t\t-link: si es enlace simbolico, el path contenido\n");
    else if(!strcmp(tr[0],"create")) printf("create [-f] [name]	Crea un directorio o un fichero (-f)\n");
    else if(!strcmp(tr[0],"delete")) printf("delete [name1 name2 ..]	Borra ficheros o directorios vacios\n");
    else if(!strcmp(tr[0],"deltree")) printf("deltree [name1 name2 ..]	Borra ficheros o directorios no vacios recursivamente\n");
    else printf("%s no encontrado\n",tr[0]);
}
void Cmd_create(char *tr[]){
if(tr[0]==NULL) Cmd_carpeta(tr);
if (tr[0]!=NULL){
	char path[MAX];
	getcwd(path, sizeof (path)); //get current directory
	strcat(path,"/"); //concatenate the current directory and "/"
	if (!strcmp(tr[0],"-f")){
		char* name =tr[1];
		strcpy(strcat(path,name),path);
		if(open(path, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)==-1)  //concatenate path/ and the name of the new file, create the file
		   printf("Imposible crear: %s\n",strerror(errno));}
	else {
		char* name= tr[0];
		strcpy(strcat(path,name),path);
		if (mkdir(path,0775)==-1) //concatenate path/ and the name of the new directory, create the directory
		   printf("Imposible crear: %s\n",strerror(errno));
		}
	}
}
void Cmd_stat (char *tr[]){
char dir[MAX];
if (tr[0]==NULL) Cmd_carpeta(tr);
if (tr[0]!=NULL){
	bool lng=false,acc=false,link=false;
	for(int i=0;tr[i]!=NULL;i++){
		if(!strcmp(tr[i],"-long")) lng=true;
		if(!strcmp(tr[i],"-link")) link=true;
		if(!strcmp(tr[i],"-acc")) acc=true;
		}
		int i=0;
		if (lng==true) i++;
		if(link == true) i++;
		if(acc==true) i++;
		if (tr[i]==NULL) printf("%s\n",getcwd(dir,MAX));
		while(tr[i]!=NULL)
		  {   if(Info(tr[i],lng,acc,link)==-1) {printf("%s\n",strerror(errno));}
			  i++;}
       }
		
}
void Cmd_list (char *tr[]){
	char dir[MAX];
    if (tr[0]==NULL) Cmd_carpeta(tr);
	if (tr[0]!=NULL){
	bool lng=false,acc=false,link=false,hid=false,reca=false,recb=false;
	for(int i=0;i<6;i++){
		if(tr[i]!=NULL){
		if(!strcmp(tr[i],"-long")) lng=true;
		if(!strcmp(tr[i],"-link")) link=true;
		if(!strcmp(tr[i],"-acc")) acc=true;
		if(!strcmp(tr[i],"-hid")) hid=true;
		if(!strcmp(tr[i],"-reca")) reca=true;
		if(!strcmp(tr[i],"-recb")) recb=true;}
		}
		int i=0;
		if (lng==true) i++;
		if(link == true) i++;
		if(acc==true) i++;
		if (hid==true) i++;
		if(reca == true) i++;
		if(recb==true) i++;
		if (tr[i]==NULL) printf("%s\n",getcwd(dir,MAX));
		while(tr[i]!=NULL)
          { if(dirInfo(tr[i],lng,acc,link,hid,reca,recb)==-1) {printf("****error al acceder a %s: %s\n",tr[i],strerror(errno));}
			  i++;}
			  
       }
		
}
void Cmd_delete (char *tr[]){
	int i=0;
	if (tr[0]==NULL) Cmd_carpeta(tr);
	if (tr[0]!=NULL){
		while(tr[i]!=NULL){
			if(remove(tr[i])) {printf("Imposible borrar %s:%s\n",tr[i],strerror(errno));}
			i++;}
	}
}
void Cmd_deltree (char *tr[]){
	int i=0;
	struct stat p;
	if (tr[0]==NULL) Cmd_carpeta(tr);
	if (tr[0]!=NULL){
		while(tr[i]!=NULL){
			stat(tr[i],&p);
			if (S_ISDIR(p.st_mode)){ 
				if(deleteDir(tr[i])){printf("%s\n",strerror(errno));}
				remove(tr[i]);}
			else if(remove(tr[i])) {printf("%s\n",strerror(errno));}
			i++;}
		}
	}
void Cmd_recurse(char *tr[]){ //FINISHED
	int p;
	if (tr[0]!=NULL) {
		p=atoi(strtok(*tr, " "));
		Recursiva(p);}
}
void Cmd_memory(char *tr[], tListMem *M){ //FINISHED
	int local1;
    int local2;
    int local3;
	if (tr[0]==NULL) {
		        printf("local  variables:  %p, %p, %p\n", &local1, &local2, &local3);
               printf("global variables:  %p, %p, %p\n", &global1, &global2, &global3);
                printf("static variables:  %p, %p, %p\n", &static1, &static2, &static3);
                printf("program functions: %p, %p, %p\n", Cmd_memory, TrocearCadena, ProcesarEntrada);
                printf("library functions: %p, %p, %p\n", &malloc, &strcmp, &strcpy);
                printf("******Lista de bloques asignados para el proceso %d\n",getpid());
				printMemList(0,M);}
	else{
			if(!strcmp(tr[0],"-blocks")){
				printf("******Lista de bloques asignados para el proceso %d\n",getpid());
				printMemList(0,M);
				}
			if(!strcmp(tr[0],"-funcs")){
				printf("program functions: %p, %p, %p\n", Cmd_memory, TrocearCadena, ProcesarEntrada);
                printf("library functions: %p, %p, %p\n", &malloc, &strcmp, &strcpy);
				}
			if(!strcmp(tr[0],"-vars")){
				printf("local  variables:  %p, %p, %p\n", &local1, &local2, &local3);
                printf("global variables:  %p, %p, %p\n", &global1, &global2, &global3);
                printf("static variables:  %p, %p, %p\n", &static1, &static2, &static3);}
			if(!strcmp(tr[0],"-pmap")){Do_pmap();}
			if(!strcmp(tr[0],"-all")){
				printf("local  variables:  %p, %p, %p\n", &local1, &local2, &local3);
                printf("global variables:  %p, %p, %p\n", &global1, &global2, &global3);
                printf("static variables:  %p, %p, %p\n", &static1, &static2, &static3);
                printf("program functions: %p, %p, %p\n", Cmd_memory, TrocearCadena, ProcesarEntrada);
                printf("library functions: %p, %p, %p\n", &malloc, &strcmp, &strcpy);
                printf("******Lista de bloques asignados para el proceso %d\n",getpid());
				printMemList(0,M);
				}
		}
	}
void Cmd_allocate(char *tr[],tListMem *L){ //FINISHED 
if (tr[0]==NULL) {
	printf("******Lista de bloques asignados para el proceso %d\n",getpid());
	printMemList(0,L);
	}
else {
	 if(!strcmp(tr[0],"-malloc")){
		if(tr[1]==NULL) {
		printf("******Lista de bloques malloc asignados para el proceso %d\n",getpid());
		printMemList(1,L);}
		else {
        tItemM newEntry;
        newEntry.size = strtoul(tr[1], NULL, 10);
        newEntry.memAddress = malloc(newEntry.size);
        if(newEntry.memAddress == NULL){ //malloc failed
            printf("Error: %s\n", strerror(errno));}
        else{newEntry.memAllocationType = 1; //malloc
			time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            newEntry.time = tm;
            insertItemMem(newEntry, LNULL, L);
            printf("%ld bytes allocated at %p\n", newEntry.size, newEntry.memAddress);}}	 
    }else if(!strcmp(tr[0],"-createshared")){
		if(tr[1]==NULL) printf("uso: allocate [-malloc|-shared|-createshared|-mmap] ....");
		else {
			if(tr[2]==NULL){
				printf("******Lista de bloques asignados shared para el proceso %d\n",getpid());
			   printMemList(3,L);}
			else do_AllocateCreateshared (tr,L);}
	}else if(!strcmp(tr[0],"-shared")){
		tItemM newEntry;
        newEntry.size = 0;
        newEntry.memKey = strtoul(tr[1], NULL, 10);
        newEntry.memAddress = ObtenerMemoriaShmget(newEntry.memKey, 0, &newEntry);
        if(newEntry.memAddress == NULL){
            printf("Error: %s\n", strerror(errno));
        }else{
            newEntry.memAllocationType = 3; //shared memory
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            newEntry.time = tm;
            insertItemMem(newEntry, LNULL, L);
            printf("Shared memory with key %d allocated at %p\n", newEntry.memKey, newEntry.memAddress);
        }
	}else if(!strcmp(tr[0],"-mmap")){
		if(tr[1]==NULL) {
			printf("******Lista de bloques malloc asignados para el proceso %d\n",getpid());
			printMemList(2,L);}
		else{
			do_AllocateMmap(tr,L);}
	}else {printf("uso: allocate [-malloc|-shared|-createshared|-mmap] ....");}
  }
}
void Cmd_deallocate(char *tr[],tListMem *L){ //FINISHED
if (tr[0]==NULL) {
	printf("******Lista de bloques asignados para el proceso %d\n",getpid());
	printMemList(0,L);
	}
else {
	  if(!strcmp(tr[0],"-malloc")){
		  if(tr[1]==NULL){
			  printf("******Lista de bloques asignados malloc para el proceso %d\n",getpid());
	          printMemList(1,L);}
		  else{
			  tSize size;
              tPosM pos;
              tItemM item; 
              size = strtoul(tr[2], NULL, 10);
              pos = findSize(size, 1, L);
              if (pos == LNULL){ 
              printf("No hay bloque de ese tamano asignado con malloc\n");}
              else{
              item = getItemMem(pos, L);
              free(item.memAddress);
              *L = deleteAtPositionMem(pos, L);}
             }
		  }
	  else if(!strcmp(tr[0],"-shared")){
		   if(tr[1]==NULL){
			  printf("******Lista de bloques asignados shared para el proceso %d\n",getpid());
	          printMemList(3,L);}
	        else{
				 tMemKey key;
                 tPosM pos;
                 tItemM item;
                 key = strtoul(tr[2], NULL, 10); 
                 pos = findMemKey(key, 3, L);
                 if (pos == LNULL){ //key not found: print list elements allocated with createshared and shared
                 printf("No hay bloque de esa clave mapeado en el proceso\n");
                 printMemList(3, L);}
                 else{
                 item = getItemMem(pos, L);
                 if(shmdt(item.memAddress) != 0){ //an error occurred
                 printf("Error: %s\n", strerror(errno));}
                 else *L = deleteAtPositionMem(pos, L); }
               }
		  }
	  else if(!strcmp(tr[0],"-delkey")){
		   key_t clave;
           int id;
           char *key=tr[1];
           if (key==NULL || (clave=(key_t) strtoul(key,NULL,10))==IPC_PRIVATE){
           printf ("      delkey necesita clave_valida\n");
           return;}
           if ((id=shmget(clave,0,0666))==-1){
           perror ("shmget: imposible obtener memoria compartida");
           return;}
           if (shmctl(id,IPC_RMID,NULL)==-1)
           perror ("shmctl: imposible eliminar memoria compartida\n");}
	  else if(!strcmp(tr[0],"-mmap")){
		  if(tr[1]==NULL){
			  printf("******Lista de bloques asignados map para el proceso %d\n",getpid());
	          printMemList(2,L);}
	      else {
			   char fileName[MAX];
               tPosM pos;
               tItemM item;
               strcpy(fileName, tr[2]);
               pos = findFileName(fileName, 2, L);
               if (pos == LNULL){ //block of specified memory not found: print list elements allocated with mmap
               printf("Fichero %s no mapeado\n",fileName);}
               else{
               item = getItemMem(pos, L);
               if(munmap(item.memAddress, item.size) != 0){ //an error occurred while unmapping the file
               printf("Error: %s\n", strerror(errno)); }
               else{if(close(item.fd) != 0){ //an error occurred while closing the file
                    printf("Error: %s\n", strerror(errno)); }
                    else *L= deleteAtPositionMem(pos, L); }
                 }
			  }
		  }
	  else{
		   tMemAddress memAddress = (void*) strtoul(tr[0], NULL, 0);//base = 0 makes strtoul to auto detect the number base (hexadecimal)
           tPosM pos = findMemAddress(memAddress, L);
           tItemM item;
           if (pos == LNULL){ //key not found:
           printf("Direccion %p no asignada con malloc, shared o mmap\n",memAddress);}
           else{
           item = getItemMem(pos, L);
           if(item.memAllocationType == 1){ //malloc
           free(item.memAddress);
           *L = deleteAtPositionMem(pos, L);  }
           if(item.memAllocationType == 2){ //mmap
			   if(munmap(item.memAddress, item.size) != 0){ //an error occurred while unmapping the file
                printf("Error: %s\n", strerror(errno));}
               else{
                if(close(item.fd) != 0){ //an error occurred while closing the file
                    printf("Error: %s\n", strerror(errno));}
                else *L = deleteAtPositionMem(pos, L);}
           }
           if(item.memAllocationType == 3){ //shared memory
               if(shmdt(item.memAddress) != 0){ //an error occurred
                printf("Error: %s\n", strerror(errno));}
               else *L = deleteAtPositionMem(pos, L);
        }
                }
		  }
	}
}
void Cmd_memdump(char *tr[]){//FINISHED
	int i,n;
	unsigned char c;
	size_t cont=25;
	char* memAddress;
	if (tr[0]==NULL) return; //memdump
	else { //memdump addr
		if(tr[1]!=NULL) cont=(size_t) strtoull(tr[1],NULL,10); //memdump addr count
		memAddress=(void *) strtoul(tr[0],NULL,16);
		printf("Volcando %zu bytes desde la direccion %p\n",cont,memAddress);
		char * byte =memAddress;
		for(int i = 0;i<cont;i++){
        printf("%c ",byte[i]);}
        printf("\n");
         for(i=0; i<cont;i++){
        printf("%2x ",byte[i]);}
        printf("\n");
	}
}
void Cmd_memfill(char *tr[]){//FINISHED
	
	int cont = 128;
    char c = 'A';
    int i;
    char* memAddress;
    if (tr[0]==NULL) return; //memfill
    else { //"memfill addr ..."

        if (tr[1]!=NULL) { //"memfill addr cont": fill cont bytes with 'A'
            cont = (int) strtol(tr[1], NULL, 10);
        }

        if (tr[2]!=NULL) { //"memfill addr cont byte": fill cont bytes with c
            c = (char) strtol(tr[2], NULL, 16); //value is given in hexadecimal
        }
        memAddress = (void *) strtoul(tr[0], NULL, 16);

        for(i = 0; i <= cont; i++){
            *(memAddress + i) = c;
        }

    }
}
void Cmd_i_o(char *tr[]){ //FINISHED
	if(tr[0]==NULL) printf("uso: e-s [read|write] ......");
	else{
		if(!strcmp(tr[0],"read")){do_I_O_read(tr);}
		if(!strcmp(tr[0],"write")){do_I_O_write(tr);}
		}
	}
void leerEntrada(char* cadena, tList* L) {
    fgets(cadena,MAX,stdin);
    insertItem(cadena,L);
}
int TrocearCadena(char* cadena,char* trozos[]){ 
	int i=1;
    if ((trozos[0]=strtok(cadena," \n\t"))==NULL) return 0;
    while((trozos[i]=strtok(NULL," \n\t"))!=NULL) {
        i++;}
        return i;
}
void ProcesarEntrada(char* cadena,char * tr[],tList *L,tListMem *M){
    int i;
    if (tr[0]==NULL) return;
    for(i=0;c[i].nombre!=NULL;i++){ //sizeof (c)/sizeof(strict CMD)
        if(!strcmp(c[i].nombre,tr[0])){
            (*c[i].func)(tr+1);}}
     if(!strcmp("hist",tr[0])) Cmd_hist(tr+1,L);
     if(!strcmp("comando",tr[0])) Cmd_comando(tr+1,L,M);
     if(!strcmp("allocate",tr[0])) Cmd_allocate(tr+1,M);
     if(!strcmp("memory",tr[0])) Cmd_memory(tr+1,M);
     //printf("Comand not found");
    }
int main(int arc, char *argv[]){
        char entrada[MAX];
        char * trozos [MAX/2];
        tList L;
        createEmptyList(&L);
        tListMem M;
        createEmptyListMem(&M);
        while(!terminado){
            printf("-->");
            leerEntrada(entrada,&L);
            if(TrocearCadena(entrada,trozos)==0 )continue;
            ProcesarEntrada(entrada,trozos,&L,&M);}
        return 0;
    }
