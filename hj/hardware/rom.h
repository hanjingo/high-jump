/*
 *  This file is part of hj.
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ROM_H
#define ROM_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// ROM structure definition
typedef struct
{
    void  *data;   // Pointer to ROM data
    size_t size;   // Size of ROM in bytes
    bool   loaded; // Whether ROM is loaded
} rom_t;

// Initialize ROM structure
inline void rom_init(rom_t *rom)
{
    if(rom)
    {
        rom->data   = NULL;
        rom->size   = 0;
        rom->loaded = false;
    }
}

// ----------------------------- ROM API define ------------------------------------
inline bool rom_load(rom_t *rom, const char *filename)
{
    if(!rom || !filename)
        return false;

    if(rom->data)
    {
        free(rom->data);
        rom->data   = NULL;
        rom->size   = 0;
        rom->loaded = false;
    }

    FILE *fp = fopen(filename, "rb");
    if(!fp)
        return false;

    fseek(fp, 0, SEEK_END);
    size_t sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if(sz == 0)
    {
        fclose(fp);
        return false;
    }

    void *buf = malloc(sz);
    if(!buf)
    {
        fclose(fp);
        return false;
    }

    if(fread(buf, 1, sz, fp) != sz)
    {
        free(buf);
        fclose(fp);
        return false;
    }

    fclose(fp);
    rom->data   = buf;
    rom->size   = sz;
    rom->loaded = true;
    return true;
}

inline size_t rom_read(const rom_t *rom, size_t offset, void *buf, size_t len)
{
    if(!rom || !rom->loaded || !buf)
        return 0;

    if(offset >= rom->size)
        return 0;

    size_t to_read = len;
    if(offset + len > rom->size)
        to_read = rom->size - offset;
    memcpy(buf, (const uint8_t *) rom->data + offset, to_read);
    return to_read;
}

inline void rom_free(rom_t *rom)
{
    if(!rom || !rom->data)
        return;

    free(rom->data);
    rom->data   = NULL;
    rom->size   = 0;
    rom->loaded = false;
}

#ifdef __cplusplus
}
#endif

#endif // ROM_H