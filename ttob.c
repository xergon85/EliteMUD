/* ************************************************************************
*  File: ttob.c                                                           *
*  Usage: Converts an Elite text based player-file to a binary one        *
************************************************************************* */

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
  int i, j, k[13], cnt;
  long l;

  struct char_file_u player;

  if (!(infile = fopen(filename, "r+"))) {
    printf("Can't open %s to reading.\n", filename);
    exit(0);
  }

  outfile = fopen("players.new", "wb");

  for (;;) {

    if (feof(infile)) {
      fclose(infile);
      fclose(outfile);
      puts("Done.");
      exit(0);
    }

    fscanf(infile, "%s %s %s %ld", player.name, player.pwd, player.host,
            &player.last_logon);
    printf("-> Converting %s text to binary\n", player.name);
    fscanf(infile, "%d %d %d %d %d %d %d %d", &player.sex, &player.class,
            &player.level, &player.birth, &player.played, &player.weight,
            &player.height, &player.hometown);
    fscanf(infile, "%d %d %d %d %d %d %d", &player.abilities.str,
            &player.abilities.str_add, &player.abilities.intel, 
            &player.abilities.wis, &player.abilities.dex, &player.abilities.con,
            &player.abilities.cha);
    fscanf(infile, "%d %d %d %d %d %d", &player.points.mana, 
            &player.points.max_mana, &player.points.hit, &player.points.max_hit,
            &player.points.move, &player.points.max_move);
    fscanf(infile, "%d %d %d %d %d %d", &player.points.armor,
            &player.points.gold, &player.points.bank_gold, &player.points.exp,
            &player.points.hitroll, &player.points.damroll);
    fscanf(infile, "%d %d %d %d %d %d %d %d %d %d %d", k, k + 1, k + 2,
           k + 3, k + 4, k + 5, k + 6, k + 7, k + 8, k + 9, k + 10);
    for (i = 0; i < 11; i++)
      player.skills[i] = k[i];
    cnt = 11;
    for (j = 0; j < 38; j++) {
      fscanf(infile, "%d %d %d %d %d %d %d %d %d %d", k, k + 1, k + 2,
             k + 3, k + 4, k + 5, k + 6, k + 7, k + 8, k + 9);
      for (i = cnt; i < (cnt + 10); i++)
        player.skills[i] = k[i - cnt];
      cnt += 10;
    }
    fscanf(infile, "%d %d %d %d %d %d %d %d %d %d %d %d %d %ld", k, k + 1,
           k + 2, k + 3, k + 4, k + 5, k + 6, k + 7, k + 8, k + 9, k + 10,
           k + 11, k + 12, &l);
    for (i = 391; i < 400; i++)
      player.skills[i] = k[i - 391];
    player.affected[0].type = k[9];
    player.affected[0].duration = k[10];
    player.affected[0].modifier = k[11];
    player.affected[0].location = k[12];
    player.affected[0].bitvector = l;
    for (i = 1; i < MAX_AFFECT; i++)
      fscanf(infile, "%d %d %d %d %ld", &player.affected[i].type,
              &player.affected[i].duration, &player.affected[i].modifier,
              &player.affected[i].location, &player.affected[i].bitvector);
    fscanf(infile, "%d %d %d %d %ld %ld", &player.specials2.idnum,
            &player.specials2.loadroom, &player.specials2.spells_to_learn,
            &player.specials2.alignment, &player.specials2.act, 
            &player.specials2.pref);
    fscanf(infile, "%d %d %d", &player.specials2.wimp_level,
            &player.specials2.freeze_level, &player.specials2.bad_pws);
    fscanf(infile, "%d %d %d %d %d", &player.specials2.resistances[0],
           &player.specials2.resistances[1], &player.specials2.resistances[2],
           &player.specials2.resistances[3], &player.specials2.resistances[4]);
    fscanf(infile, "%d %d %d", &player.specials2.conditions[0],
           &player.specials2.conditions[1], &player.specials2.conditions[2]);
    fscanf(infile, "%ld %d %ld %d %d %d", &player.specials2.worships,
            &player.specials2.quest, &player.specials2.questflags,
            &player.specials2.clan, &player.specials2.clanlevel,
            &player.specials2.race);
    fscanf(infile, "%d %d %d %d %d %d", &player.specials2.class1,
            &player.specials2.class2, &player.specials2.class3,
            &player.specials2.level1, &player.specials2.level2,
            &player.specials2.level3);
    fscanf(infile, "%d %d %d\n", &player.specials2.scrlen,
            &player.specials2.remorts, &player.specials2.godpoints);

    fwrite(&player, sizeof(struct char_file_u), 1, outfile);
  }
}

int main(int argc, char **argv)
{
  if (argc != 2)
    printf("Usage: %s playerfile-name\n", argv[0]);
  else
    convert(argv[1]);

  return 0;
}
