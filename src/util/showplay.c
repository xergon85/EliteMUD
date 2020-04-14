/* ************************************************************************
*  file:  showplay.c                                  Part of CircleMud   *
*  Usage: list a diku playerfile                                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
************************************************************************* */

#include "conf.h"
#include "sysdep.h"

#include "../structs.h"


void show(char *filename)
{
  char sexname;
  char classname[20];
  char racename[20];
  char buf1[MAX_STRING_LENGTH];
  FILE *fl;
  struct char_file_u player;
  int num = 0;
  long size;

  if (!(fl = fopen(filename, "r+"))) {
    perror("error opening playerfile");
    exit(1);
  }
  fseek(fl, 0L, SEEK_END);
  size = ftell(fl);
  rewind(fl);
  if (size % sizeof(struct char_file_u)) {
    fprintf(stderr, "\aWARNING:  File size does not match structure, recompile showplay.\n");
    fclose(fl);
    exit(1);
  }

  for (;;) {
    fread(&player, sizeof(struct char_file_u), 1, fl);
    if (feof(fl)) {
      fclose(fl);
      exit(0);
    }

    switch (player.class) {
    case CLASS_MAGIC_USER:
      strcpy(classname, "Magic User ");
      break;
    case CLASS_CLERIC:
      strcpy(classname, "Cleric     ");
      break;
    case CLASS_THIEF: 
      strcpy(classname, "Thief      ");
      break;
    case CLASS_WARRIOR:
      strcpy(classname, "Warrior    ");
      break;
    case CLASS_PSIONICIST:
      strcpy(classname, "Psionicist ");
      break;
    case CLASS_MONK:
      strcpy(classname, "Monk       ");
      break;
    case CLASS_BARD:
      strcpy(classname, "Bard       ");
      break;
    case CLASS_KNIGHT:
      strcpy(classname, "Knight     ");
      break;
    case CLASS_WIZARD:
      strcpy(classname, "Wizard     ");
      break;
    case CLASS_DRUID: 
      strcpy(classname, "Druid      ");
      break;
    case CLASS_ASSASSIN:
      strcpy(classname, "Assassin   ");
      break;
    case CLASS_RANGER:
      strcpy(classname, "Ranger     ");
      break;
    case CLASS_ILLUSIONIST:
      strcpy(classname, "Illusionist");
      break;
    case CLASS_PALADIN:
      strcpy(classname, "Paladin    ");
      break;
    case CLASS_MARINER:
      strcpy(classname, "Mariner    ");
      break;
    case CLASS_CAVALIER:
      strcpy(classname, "Cavalier   ");
      break;
    case CLASS_NONEMU:
      strcpy(classname, "None Mu    ");
      break;
    case CLASS_NONECL:
      strcpy(classname, "None Cl    ");
      break;
    case CLASS_NINJA:
      strcpy(classname, "Ninja      ");
      break;
    case CLASS_NONEWA:
      strcpy(classname, "None Wa    ");
      break;
    case CLASS_DUAL:
      strcpy(classname, "Dual       ");
      break;
    case CLASS_2MULTI:
      strcpy(classname, "2-Multi    ");
      break;
    case CLASS_3MULTI:
      strcpy(classname, "3-Multi    ");
      break;
    case CLASS_MAX:
      strcpy(classname, "Max        ");
      break;

    default:
      strcpy(classname, "Unknown    ");
      break;
    }


    switch (player.sex) {
    case SEX_FEMALE:
      sexname = 'F';
      break;
    case SEX_MALE:
      sexname = 'M';
      break;
    case SEX_NEUTRAL:
      sexname = 'N';
      break;
    default:
      sexname = '-';
      break;
    }

    switch (player.specials2.race) {
    case RACE_GOD:
      strcpy(racename, "God       ");
      break;
    case RACE_HUMAN:
      strcpy(racename, "Human     ");
      break;
    case RACE_ELF:
      strcpy(racename, "Elf       ");
      break;
    case RACE_HALFELF:
      strcpy(racename, "Half-Elf  ");
      break;
    case RACE_DWARF:
      strcpy(racename, "Dwarf     ");
      break;
    case RACE_GNOME:
      strcpy(racename, "Gnome     ");
      break;
    case RACE_HALFLING:
      strcpy(racename, "Halfing   ");
      break;
    case RACE_HALFTROLL:
      strcpy(racename, "Half-Troll");
      break;
    case RACE_HALFORC:
      strcpy(racename, "Half-Orc  ");
      break;
    case RACE_HALFOGRE:
      strcpy(racename, "Half-Ogre ");
      break;
    case RACE_DUCK:
      strcpy(racename, "Duck      ");
      break;
    case RACE_FAIRY:
      strcpy(racename, "Fairy     ");
      break;
    case RACE_MINOTAUR:
      strcpy(racename, "Minotaur  ");
      break;
    case RACE_RATMAN:
      strcpy(racename, "Ratman    ");
      break;
    case RACE_DROW:
      strcpy(racename, "Drow      ");
      break;
    case RACE_LIZARDMAN:
      strcpy(racename, "Lizardman ");
      break;
    case RACE_VAMPIRE:
      strcpy(racename, "Vampire   ");
      break;
    case RACE_TROLL:
      strcpy(racename, "Troll     ");
      break;
    case RACE_DRACONIAN:
      strcpy(racename, "Draconian ");
      break;
    case RACE_AVATAR:
      strcpy(racename, "Avatar    ");
      break;
    case RACE_WEREWOLF:
      strcpy(racename, "Werewolf  ");
      break;
    case RACE_DEMON:
      strcpy(racename, "Demon     ");
      break;
    case RACE_DRAGON:
      strcpy(racename, "Dragon    ");
      break;
    case RACE_FELINE:
      strcpy(racename, "Feline    ");
      break;
    case RACE_ANGEL:
      strcpy(racename, "Angel     ");
      break;

    default:
      strcpy(racename, "Unknown   ");
      break;
    }

    strcpy(buf1, (char *)asctime(localtime(&(player.birth))));

    printf("%5d. ID: %5ld (%c) [%3d %s] %-16s Race(%s) Room[%5d]\n"
           "       Remort[%d] Host[%-15s] Created: %s  %9dg %9db\n", ++num,
           player.specials2.idnum, sexname, player.level,
           classname, player.name, racename, player.specials2.load_room,
           player.specials2.remorts, player.host, buf1,
           player.points.gold, player.points.bank_gold);
  }
}


int main(int argc, char **argv)
{
  if (argc != 2)
    printf("Usage: %s playerfile-name\n", argv[0]);
  else
    show(argv[1]);

  return (0);
}
