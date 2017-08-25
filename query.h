#pragma once

#include "vfs.h"

/**
 * Convert filename to correct case and optionally remove suffix (.sql)
 * */
int str_fn2obj(char **dst, char *src, int removeSuffix);

/**
 * Populate g_vfs with list of schemas. Those are first-level folder entries.
 * */
void qry_schemas();

/**
 * Populate g_vfs with list of object types.
 * */
int qry_types(t_fsentry *schema);

/**
 * Populate g_vfs with list of actual objects (.sql files). 
 * */
void qry_objects(t_fsentry *schema, t_fsentry *type);

/**
 * Write DDL to of object specified by schema,type,object parameters to file name fname.
 * fname is determined by this function.
 * */
int qry_object(const char *schema, 
			   const char *type, 
			   const char *object,
			   char **fname);

/**
 * Assamble temporary filename based on schema, type and object.
 * */
int qry_object_fname(const char *schema,
					 const char *type,
					 const char *object,
					 char **fname);

/**
 * Execute DDL statement.
 * */
int qry_exec_ddl(char *ddl);

