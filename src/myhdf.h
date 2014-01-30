#ifndef MYHDF_H
#define MYHDF_H

#include "hdf.h"
#include "mfhdf.h"
#include "bool.h"

#define MYHDF_MAX_RANK (4)     /* maximum rank of an SDS expected */
#define MYHDF_MAX_NATTR_VAL (3000)
                               /* maximum number of attribute values expected */
#define HDF_ERROR (-1)

/* Structure to store information about the HDF SDS */
typedef struct {
  int32 nval, id;        /* number of values and id */ 
  int32 type, nattr;     /* HDF data type and number of attributes */
  char *name;            /* dimension name */
} Myhdf_dim_t;

typedef struct {
  int32 index, id, rank;           /* index, id and rank */
  int32 type, nattr;               /* HDF data type and number of attributes */
  char *name;                      /* SDS name */
  Myhdf_dim_t dim[MYHDF_MAX_RANK]; /* dimension data structure */
} Myhdf_sds_t;

/* Structure to store information about the HDF attribute */
typedef struct {
  int32 id, type, nval;	 /* id, data type and number of values */
  char *name;            /* attribute name */
} Myhdf_attr_t;

/* Prototypes */
bool GetSDSInfo(int32 sds_file_id, Myhdf_sds_t *sds);
bool GetSDSDimInfo(int32 sds_id, Myhdf_dim_t *dim, int irank);
bool PutSDSInfo(int32 sds_file_id, Myhdf_sds_t *sds);
bool PutSDSDimInfo(int32 sds_id, Myhdf_dim_t *dim, int irank);
bool GetAttrDouble(int32 sds_id, Myhdf_attr_t *attr, double *val);
bool PutAttrDouble(int32 sds_id, Myhdf_attr_t *attr, double *val);
bool GetAttrString(int32 sds_id, Myhdf_attr_t *attr, char *string);
bool PutAttrString(int32 sds_id, Myhdf_attr_t *attr, char *string);

#endif
