/*
 * $Id: utils.h,v 1.1 2014/04/15 15:34:47 xzm Exp $
 *
 * Copyright (C) 2001, 2002, 2003 Flavio de Ayra Mendes <h4@locked.org>
 */

#ifndef _UTILS_H
#define _UTILS_H

extern void spacefix(char *string);
extern void chomp(char *str);
extern FILE *my_tmpfile(char *_template, const char *path);
extern int my_fgets(char *dest, ssize_t dest_size, FILE *fp);
extern int is_an_option(const char *buffer, char *dest_opt, size_t opt_size, 
			char *dest_val, size_t val_size);
extern int is_a_section(const char *buffer, char *dest, size_t dest_size);

#endif /* utils.h */


