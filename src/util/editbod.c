/* ************************************************************************
*  file: editlvl.c                                          Part of Elite * 
*  Usage: move imms up in level for expansion of level system             *
************************************************************************* */

#include "conf.h"
#include "sysdep.h"

#include "../structs.h"
#include "../utils.h"


void editlvl(char *filename)
{
  FILE *fl;
  FILE *outfile;
  struct char_file_u player;
  int num = 0;
  long size;

  if (!(fl = fopen(filename, "r+"))) {
    printf("Can't open %s.", filename);
    exit(1);
  }
  fseek(fl, 0L, SEEK_END);
  size = ftell(fl);
  rewind(fl);
  if (size % sizeof(struct char_file_u)) {
    fprintf(stderr, "\aWARNING:  File size does not match structure, recompile editlvl.\n");
    fclose(fl);
    exit(1);
  }

  outfile = fopen("players.new", "w");
  printf("Updating: \n");

  for (;;) {
    fread(&player, sizeof(struct char_file_u), 1, fl);
    if (feof(fl)) {
      fclose(fl);
      fclose(outfile);
      printf("Done.\n");
      exit(0);
    }

    if (player.level >= LEVEL_DEITY) {
        if (player.specials2.idnum == 92776) {
          player.level = LEVEL_IMPL;
          player.specials2.godlevel = IMM_ALL;
        }
        else if (player.level == 114)
          player.level = LEVEL_IMPL;
	else if (player.level == 113)
          player.level = LEVEL_ADMIN;
        else if (player.level == 112)
          player.level = LEVEL_LESSER;
        else if (player.level == 111)
          player.level = LEVEL_IMMORT;
        else if (player.level == 110)
          player.level = LEVEL_RETIRED;
        else
          player.level = LEVEL_DEITY;
    }

    fwrite(&player, sizeof(struct char_file_u), 1, outfile);
  }
}



int main(int argc, char *argv[])
{
  if (argc != 2)
    printf("Usage: %s playerfile-name\n", argv[0]);
  else
    editlvl(argv[1]);

  return (0);
}
