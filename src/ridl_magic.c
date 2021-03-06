#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef strlcpy
#undef strlcpy
#endif

#ifdef strlcat
#undef strlcat
#endif

#include "idl_export.h"
#include "readline/history.h"

#include "ridl.h"
#include "ridl_strings.h"
#include "ridl_config.h"


/**
   @file
   This file contains helper routines dealing with the magic commands.
*/


/**
   Print help for the magic commands.
*/
int ridl_magic_help(char *line, int firstcharIndex) {
  char *indent = "  ";
  char *magic_format = "%s%-*s %s\n";
  int magic_width = 21;
  int length = 5;
  int search_length;
  char *argument = ridl_getnextword(line, firstcharIndex + length, &search_length);
  char show_cmd[RIDL_MAX_LINE_LENGTH];
  char help_filename[RIDL_MAX_LINE_LENGTH];
  int result;
  char first_char[2];
  int plain_help = 1;
  int i;

  first_char[1] = '\0';

  if (strlen(argument) > 0) {
    if (strcmp(argument, "verbose") == 0) {
      sprintf(show_cmd, "$more %s/share/ridl_verbosehelp.txt", RIDL_DIR);
      result = IDL_ExecuteStr(show_cmd);
    } else {
      plain_help = 0;
      first_char[0] = toupper(argument[0]);
      for (i = 0; i < strlen(argument); i++) {
        argument[i] = toupper(argument[i]);
      }
      sprintf(help_filename, "%s/online_help/%s/%s.txt", RIDL_DIR, first_char, argument);
      if (!ridl_file_exists(help_filename)) {
        ridl_warning("unknown :help topic '%s'", argument);
      } else {
        sprintf(show_cmd, "$more %s", help_filename);
        result = IDL_ExecuteStr(show_cmd);
      }
    }
  } else {
    ridl_printversion();

    printf("\nmagic commands:\n");

    printf(magic_format, indent, magic_width, ":colors",
           "toggle whether color is used");
    printf(magic_format, indent, magic_width, ":doc routine",
           "show calling syntax and comment header for the routine");
    printf(magic_format, indent, magic_width, ":help [cmd]",
           "show this help message; show help on 'cmd', if present");
    printf(magic_format, indent, magic_width, ":history [n] [nonum]",
           "show last n commands (defaults to 10); 'nonum' option hides command numbers");
    printf(magic_format, indent, magic_width, ":histedit n filename",
           "send last n commands to filename and launch editor");
    printf(magic_format, indent, magic_width, ":log filename",
           "start logging all commands and output to filename");
    printf(magic_format, indent, magic_width, ":unlog",
           "stop logging commands and output");
    printf(magic_format, indent, magic_width, ":notebook filename",
           "start sending all commands and output to an HTML notebook filename");
    printf(magic_format, indent, magic_width, ":unnotebook",
           "stop logging commands and output to notebook");
    printf(magic_format, indent, magic_width, ":pref [NAME=value]",
           "set rIDL preference (or list all rIDL preferences) for the current session");
    printf(magic_format, indent, magic_width, ":save_graphic",
           "save current new graphic, iTool, or direct graphic window to notebook");
    printf(magic_format, indent, magic_width, ":tee filename",
           "start logging output to filename");
    printf(magic_format, indent, magic_width, ":untee",
           "stop logging output");
    printf(magic_format, indent, magic_width, ":time cmd",
           "time the given command");
    printf(magic_format, indent, magic_width, ":version",
           "print version information");

    printf("\nintrospection:\n");
    printf("%splace a '?' after a variable or expression to show metadata\n",
           indent);
  }

  free(argument);
  return(plain_help);
}


/**
   Time a command.
   
   @param[in] cmd IDL command to time
*/
void ridl_magic_time(char *cmd) {
  clock_t clk0 = clock(), clk1;
  double seconds;

  int error = ridl_executestr(cmd, 1);

  clk1 = clock();
  seconds = (double) (clk1 - clk0) / (double) CLOCKS_PER_SEC;

  printf("%% time for '%s': %f seconds\n", cmd, seconds);
}


/**
   Prints the last `numCmds`.

   @param[in] numCmds number of commands to show
   @param[in] showCmdnum 1 if command number should be printed, 0 if not
*/
void ridl_magic_printhistory(int numCmds, int showCmdnum) {
  int i;
  HIST_ENTRY *h;
  HISTORY_STATE *hs;

  hs = history_get_history_state();
  for (i = numCmds - 1; i >= 0; i--) {
    h = history_get(hs->offset - i);
    if (showCmdnum) {
      printf("[%d]: %s\n", hs->offset - i, h == 0 ? "" : h->line);
    } else {
      printf("%s\n", h == 0 ? "" : h->line);
    }
  }
}


/**
   Print history of last n commands.
   
   @param[in] line history magic command line
   @param[in] firstcharIndex index of magic command in line
   
*/
void ridl_magic_history(char *line, int firstcharIndex) {
  int showCmdnum = 1, numCmds = 10, test, show = 1, length = 8;
  int search_length;
  
  char *first, *second;
  
  first = ridl_getnextword(line, firstcharIndex + length, &search_length);
  
  if (strlen(first) > 0) {
    test = atoi(first);
    if (test != 0) {
      numCmds = test;
    } else if (strcmp(first, "nonum") == 0) {
      showCmdnum = 0;
    } else {
      ridl_warning("unknown :history option '%s'", first);
      show = 0;
    }
  }

  if (show) {
    second = ridl_getnextword(line, firstcharIndex + length + search_length + 1, &search_length);

    if (strlen(second) > 0) {
      test = atoi(second);
      if (test != 0) {
        numCmds = test;
      } else if (strcmp(second, "nonum") == 0) {
        showCmdnum = 0;
      } else {
        ridl_warning("unknown :history option '%s'", second);
        show = 0;
      }
    }

    free(second);
  }

  free(first);

  if (show) ridl_magic_printhistory(numCmds, showCmdnum);
}


/**
   Sends history of last n commands to a file and launches an editor.
   
   @param[in] line histedit magic command line
   @param[in] firstcharIndex index of magic command in line
*/
void ridl_magic_histedit(char *line, int firstcharIndex) {
  int search_length;
  HIST_ENTRY *h;
  HISTORY_STATE *hs;
  char *snlines = ridl_getnextword(line, firstcharIndex + 10, &search_length);
  char *filename = ridl_getnextword(line, firstcharIndex + 10 + search_length + 1, &search_length);
  int i, nlines = atoi(snlines);

  if (ridl_file_exists(filename)) {
    ridl_warning("file '%s' already exists", filename);
    return;
  }

  FILE *fp = fopen(filename, "w");

  hs = history_get_history_state();
  for (i = nlines - 1; i >= 0; i--) {
    h = history_get(hs->offset - i);
    fprintf(fp, "%s\n", h == 0 ? "" : h->line);
  }
  free(snlines);
  free(filename);
  fclose(fp);
  
  ridl_launcheditor(filename);
}
