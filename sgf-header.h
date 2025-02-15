
#ifndef __SGF_DATA__
#define __SGF_DATA__


/************************************************************
 DÉFINITION D'UN BLOC.
 ---------------------
 Le bloc est l'unité élémentaire d'entrée/sortie sur le
 disque. Les fonctions ci-dessous permettent la lecture et
 l'écriture synchrone d'un bloc.
 ************************************************************/
 
#define BLOCK_SIZE              (128)    /* 128 octets */

typedef char BLOCK[ BLOCK_SIZE ];

void read_block (int n, BLOCK* b);    /* lire un bloc     */
void write_block (int n, BLOCK* b);   /* écrire un bloc   */
int  get_disk_size();                 /* taille du disque */


/************************************************************
 DÉFINITION D'UN DESCRIPTEUR DE FICHIER (INODE)
 -----------------------------------------------
 Un fichier est complètement décrit par un INODE qui est aussi
 un bloc. Un INODE sur disque a la structure suivante.
 ************************************************************/

typedef struct {       /* INODE: Descripteur de fichiers   */
    int  size;         /* taille du fichier (en octets)    */
    int  first;        /* adresse du premier bloc logique  */
    int  last;         /* adresse du dernier bloc logique  */
} INODE;

INODE read_inode (int n);
void  write_inode (int n, INODE inode);


/************************************************************
 DÉFINITION DE LA FAT (FILE ALLOCATION TABLE)
 --------------------------------------------
 Le chaînage des blocs qui composent un fichier est placé
 dans une table unique appelée la FAT (File Allocation
 Table). Cette table comporte autant d'entrées que de blocs
 sur le disque. Elle est stockée  sur disque à partir du bloc
 ADR_FAT_BLOCK. La valeur de chaque entrée est interpretée
 comme suit :

 FAT[n] < 0
     voir explications ci-dessous

 FAT[n] = m avec (m > 0) :
     le bloc m est le suivant du bloc n dans le fichier
     auquel ils appartiennent tous les deux.

 Exemple : Si un fichier est constitué des blocs 300, 150, 30,
 alors nous avons

     inode.first == 300
     inode.last  == 30
     FAT[300]    == 150
     FAT[150]    == 30
     FAT[30]     == FAT_EOF

 ************************************************************/

#define ADR_FAT_BLOCK           (1)

enum {
    FAT_FREE     = -1,    /* le bloc est libre */
    FAT_RESERVED = -2,    /* le bloc est réservé */
    FAT_INODE    = -3,    /* le bloc contient un INODE */
    FAT_EOF      = -4,    /* le bloc est le dernier du fichier */
};

/* initialisation du module */
void init_sgf_fat (void);

/* chercher un bloc libre (-1 si échec) */
int alloc_block (void);

/* lire/écrire une entrée de la FAT */
int get_fat (int n);
void set_fat (int n, int value);

/* créer une FAT vide */
void create_empty_fat (void);
 

/************************************************************
 DÉFINITION DU RÉPERTOIRE
 -------------------------
 Les couples (nom-de-fichier,adr-inode) sont contenus dans un
 répertoire unique.  Il n'existe donc pas de structure
 arborescente. Ce répertoire est composé d'un seul bloc
 (stocké à l'adresse 0 sur le disque). Un couple a la
 structure suivante :
 ************************************************************/

#define ADR_DIR_BLOCK           (0)
#define FILENAME_SIZE           (10)
#define SIGNATURE_FS            (0xAA88FF33)

typedef struct {                /* Une entrée de répertoire         */
    char name [FILENAME_SIZE];  /* nom du fichier                   */
    int  adr_inode;             /* adresse du descripteur           */
} DIR_ENTRY;


#define DIR_SIZE            ((BLOCK_SIZE - sizeof(int)) / sizeof(DIR_ENTRY))

typedef struct {                /* bloc répertoire                  */
    int       signature;        /* signature du système de fichier  */
    DIR_ENTRY files[DIR_SIZE];  /* tableau des couples              */
} DIRECTORY;


/* lecture/écriture du répertoire */
void read_dir_block (DIRECTORY* b);
void write_dir_block (DIRECTORY* b);

/* initialiser le module */
void init_sgf_dir (void);

/* chercher un nom et renvoyer l'adresse de l'INODE (-1 si échec) */
int find_inode (const char* name);

/* ajout d'un couple et renvoi de l'ancien INODE (-1 si absent) */
int add_inode (const char* name, int adr_inode);

/* suppression d'un couple */
void delete_inode (const char* name);

/* création d'un répertoire vide */
void create_empty_directory (void);

/* lister les fichiers (avec leur taille) */
void list_directory (void);


/************************************************************
 DEFINITION D'UN FICHIER OUVERT
 ------------------------------
 La structure OFILE décrit un fichier ouvert.
 ************************************************************/

typedef enum {
    READ_MODE,         /* Fichier ouvert en lecture         */
    WRITE_MODE,        /* Fichier ouvert en écriture        */
} MODE;

typedef struct {       /* "Un fichier ouvert"               */
    INODE  inode;      /* INODE (descripteur) du fichier    */
    int    adr_inode;  /* adresse de l'INODE (descripteur)  */
    int    ptr;        /* n° logique du prochain caractère  */
    MODE   mode;       /* READ_MODE ou WRITE_MODE           */
    BLOCK  buffer;     /* buffer contenant le bloc courant  */
} OFILE;


/* ouverture */
OFILE* sgf_open(const char *name, MODE mode);

/* lecture d'un caractère (-1 si EOF) */
int sgf_getc (OFILE* f);

/* déplacement dans le fichier (accès direct) */
int sgf_seek (OFILE* f, int pos);

/* écriture d'un caractère ou d'une chaîne */
void sgf_putc (OFILE* f, char  c);
void sgf_puts (OFILE* f, char *s);

/* fermeture */
void sgf_close(OFILE* f);

/* initialisation */
void init_sgf (void);


/************************************************************
 *
 *  FONCTIONS UTILITAIRES
 *
 ***********************************************************/
 
/* afficher un message et stopper */
void panic (const char *format, ...);


#endif
