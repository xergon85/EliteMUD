/*
************************************************************************ *
File: btot.c * * Usage: Converts a binary Elite player-file to a text one
*
*************************************************************************
*/

#include <stdio.h>
#include <sys/types.h>

typedef signed char      sbyte;
typedef char             byte;
typedef signed short int sh_int;
typedef unsigned char    ubyte;

#define MAX_SKILLS     400
#define MAX_AFFECT     32
#define HOST_LEN       30
#define MAX_PWD_LENGTH 10

struct char_ability_data {
 sbyte str;
 sbyte str_add;
 sbyte intel;
 sbyte wis;
 sbyte dex;
 sbyte con;
 sbyte cha;
};

struct char_point_data {
 sh_int mana;
 sh_int max_mana;
 sh_int hit;
 sh_int max_hit;
 sh_int move;
 sh_int max_move;
 sh_int armor;
 int gold;
 int bank_gold;
 int exp;
 sbyte hitroll;
 sbyte damroll;
};

struct affected_type {
 int type;
 sh_int duration;
 sbyte modifier;
 byte location;
 long bitvector;
 struct affected_type *next;
};

struct char_special2_data {
 long idnum;
 sh_int loadroom;
 ubyte spells_to_learn;
 int alignment;
 long act;
 long pref;
 int wimp_level;
 byte freeze_level;
 ubyte bad_pws;
 sh_int resistances[5];
 sbyte conditions[3];
 long worships;
 int quest;
 long questflags;
 sh_int clan;
 ubyte clanlevel;
 ubyte race;
 ubyte class1;
 ubyte class2;
 ubyte class3;
 ubyte level1;
 ubyte level2;
 ubyte level3;
 ubyte scrlen;
 ubyte remorts;
 ubyte godpoints;
};

struct char_file_u {
 byte sex;
 byte class;
 ubyte level;
 time_t birth;
 int played;
 ubyte weight;
 ubyte height;
 sh_int hometown;
 struct char_ability_data abilities;
 struct char_point_data points;
 byte skills[MAX_SKILLS];
 struct affected_type affected[MAX_AFFECT];
 struct char_special2_data specials2;
 time_t last_logon;
 char host[HOST_LEN+1];
 char name[20];
 char pwd[MAX_PWD_LENGTH+1];
};

void convert(char *filename)
{
  FILE *infile;
  FILE *outfile;
  int i;
  char erk[240];
  int e = 0;
  struct char_file_u player;

  if (!(infile = fopen(filename, "r+"))) {
    printf("Can't open %s to reading.\n", filename);
    exit(0);
  }

  sprintf(erk, "benwizlist");
  outfile = fopen(erk, "w");

  for (;;) {
    fread(&player, sizeof(struct char_file_u), 1, infile);
  
    if (feof(infile)) {
      fclose(infile);
      fclose(outfile);
      puts("Done.");
      exit(0);
    }
     if(player.class == 15)
	fprintf(outfile, "%s (%d)\n", player.name, player.level);
 }
}

int main(int argc, char **argv)
{
  char crap[240];
  int clan;

  if (argc != 2)
    printf("Usage: %s playerfile-name\n", argv[0]);
  else {
    convert(argv[1]);  
  }

  return 0;
}

