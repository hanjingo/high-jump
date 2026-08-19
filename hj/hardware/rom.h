/*
 *  This file is part of high-jump(hj).
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

typedef enum
{
    ROM_SUCCESS = 0,
    ROM_ERR_INVALID_PARAM,
    ROM_ERR_FILE_NOT_FOUND,
    ROM_ERR_NOMEM,
    ROM_ERR_IO_FAILURE,
    ROM_ERR_EMPTY_FILE,
    ROM_ERR_NOT_LOADED,
    ROM_ERR_OUT_OF_BOUNDS
} rom_result_t;

#ifndef HJ_ROM_API
#if defined(HJ_ROM_STATIC)
#define HJ_ROM_API static inline
#else
#define HJ_ROM_API extern
#endif
#endif

// ROM structure definition
typedef struct
{
    void  *data;   // Pointer to ROM data
    size_t size;   // Size of ROM in bytes
    bool   loaded; // Whether ROM is loaded
} rom_t;

// ------------------------ ROM API Declarations ------------------------
HJ_ROM_API void         rom_init(rom_t *rom);
HJ_ROM_API rom_result_t rom_load(rom_t *rom, const char *filename);
HJ_ROM_API rom_result_t rom_read(
    const rom_t *rom, size_t offset, void *buf, size_t len, size_t *out_read);
HJ_ROM_API void rom_free(rom_t *rom);

#ifdef __cplusplus
}
#endif

#endif // ROM_H


// --------------------- Implementation -------------------------
#if (defined(HJ_ROM_IMPL) || defined(HJ_ROM_STATIC))                           \
    && !defined(HJ_ROM_IMPL_DONE)
#define HJ_ROM_IMPL_DONE

#ifdef __cplusplus
extern "C" {
#endif

HJ_ROM_API void rom_init(rom_t *rom)
{
    if(rom)
    {
        rom->data   = NULL;
        rom->size   = 0;
        rom->loaded = false;
    }
}

HJ_ROM_API rom_result_t rom_load(rom_t *rom, const char *filename)
{
    if(!rom || !filename)
        return ROM_ERR_INVALID_PARAM;

    FILE *fp = fopen(filename, "rb");
    if(!fp)
        return ROM_ERR_FILE_NOT_FOUND;

    if(fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        return ROM_ERR_IO_FAILURE;
    }

    long sz = ftell(fp);
    rewind(fp);
    if(sz < 0)
    {
        fclose(fp);
        return ROM_ERR_IO_FAILURE;
    }
    if(sz == 0)
    {
        fclose(fp);
        return ROM_ERR_EMPTY_FILE;
    }

    void *new_buf = malloc((size_t) sz);
    if(!new_buf)
    {
        fclose(fp);
        return ROM_ERR_NOMEM;
    }

    if(fread(new_buf, 1, (size_t) sz, fp) != (size_t) sz)
    {
        free(new_buf);
        fclose(fp);
        return ROM_ERR_IO_FAILURE;
    }

    fclose(fp);
    if(rom->data)
        free(rom->data);

    rom->data   = new_buf;
    rom->size   = (size_t) sz;
    rom->loaded = true;
    return ROM_SUCCESS;
}

HJ_ROM_API rom_result_t rom_read(
    const rom_t *rom, size_t offset, void *buf, size_t len, size_t *out_read)
{
    if(out_read)
        *out_read = 0;

    if(!rom || !buf)
        return ROM_ERR_INVALID_PARAM;

    if(!rom->loaded)
        return ROM_ERR_NOT_LOADED;

    if(len == 0)
        return ROM_SUCCESS;

    if(offset >= rom->size)
        return ROM_ERR_OUT_OF_BOUNDS;

    size_t to_read = len;
    if(to_read > (rom->size - offset))
    {
        to_read = rom->size - offset;
    }

    memcpy(buf, (const uint8_t *) rom->data + offset, to_read);

    if(out_read)
    {
        *out_read = to_read;
    }

    return ROM_SUCCESS;
}

HJ_ROM_API void rom_free(rom_t *rom)
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