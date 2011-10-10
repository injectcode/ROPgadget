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

/* Check if phdr have a EXEC bit */
int check_exec_maps(t_maps_exec *exec_maps, Elf32_Addr addr)
{
  while (exec_maps != NULL)
    {
      if (addr >= exec_maps->addr_start && addr <= exec_maps->addr_end)
        return (TRUE);
      exec_maps = exec_maps->next;
    }
  return (FALSE);
}
