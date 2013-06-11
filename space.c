#include <stdlib.h>
#include <math.h>
#include "space.h"
#include "hdf.h"
#include "mfhdf.h"
#include "HdfEosDef.h"
#include "proj.h"
#include "mystring.h"
#include "myproj.h"
#include "myproj_const.h"
#include "const.h"
#include "bool.h"
#include "myhdf.h"
#include "input.h"

/* Constants */
#define MAX_PROJ (99)  /* Maximum map projection number */
#define GCTP_OK (0)    /* Okay status return from the GCTP package */

/* Prototypes for initializing the GCTP projections */
int for_init( int outsys, int outzone, double *outparm, int outdatum,
    char *fn27, char *fn83, int *iflg,
    int (*for_trans[])(double, double, double *, double *));
int inv_init( int insys, int inzone, double *inparm, int indatum,
    char *fn27, char *fn83, int *iflg,
    int (*inv_trans[])(double, double, double*, double*));

/* Functions */
#define NLINE_MAX (20000)
#define NSAMP_MAX (20000)

typedef enum {
  SPACE_NULL = -1,
  SPACE_START = 0,
  SPACE_PROJ_NUM,
  SPACE_PROJ_PARAM,
  SPACE_PIXEL_SIZE,
  SPACE_UL_CORNER,
  SPACE_NSAMPLE,
  SPACE_NLINE,
  SPACE_ZONE,
  SPACE_SPHERE,
  SPACE_ORIEN_ANGLE,
  SPACE_END,
  SPACE_MAX
} Space_key_t;

Key_string_t Space_string[SPACE_MAX] = {
  {(int)SPACE_START,       "HEADER_FILE"},
  {(int)SPACE_PROJ_NUM,    "PROJECTION_NUMBER"},
  {(int)SPACE_PROJ_PARAM,  "PROJECTION_PARAMETERS"},
  {(int)SPACE_PIXEL_SIZE,  "PIXEL_SIZE"},
  {(int)SPACE_UL_CORNER,   "UPPER_LEFT_CORNER"},
  {(int)SPACE_NSAMPLE,     "NSAMPLE"},
  {(int)SPACE_NLINE,       "NLINE"},
  {(int)SPACE_ZONE,        "PROJECTION_ZONE"},
  {(int)SPACE_SPHERE,      "PROJECTION_SPHERE"},
  {(int)SPACE_ORIEN_ANGLE, "ORIENTATION"},
  {(int)SPACE_END,         "END"}
};

#define SPACE_NTYPE_HDF (10)

struct {
  int32 type;		/* type values */
  char *name;		/* type name */
} space_hdf_type[SPACE_NTYPE_HDF] = {
    {DFNT_CHAR8,   "DFNT_CHAR8"},
    {DFNT_UCHAR8,  "DFNT_UCHAR8"},
    {DFNT_INT8,    "DFNT_INT8"},
    {DFNT_UINT8,   "DFNT_UINT8"},
    {DFNT_INT16,   "DFNT_INT16"},
    {DFNT_UINT16,  "DFNT_UINT16"},
    {DFNT_INT32,   "DFNT_INT32"},
    {DFNT_UINT32,  "DFNT_UINT32"},
    {DFNT_FLOAT32, "DFNT_FLOAT32"},
    {DFNT_FLOAT64, "DFNT_FLOAT64"}
};

#define SPACE_HDFEOS_VERSION ("HDFEOSVersion");
#define SPACE_STRUCT_METADATA ("StructMetadata.0");
#define SPACE_ORIENTATION_ANGLE_HDF ("OrientationAngle")
#define SPACE_PIXEL_SIZE_HDF ("PixelSize")
#define NPROJ_PARAM_HDFEOS (13)


/******************************************************************************
MODULE:  append_meta

PURPOSE:  Appends the string to the metadata buffer.

RETURN VALUE:
Type = bool
Value      Description
-----      -----------
false      Error in the number of attributes
true       Successful processing

PROJECT:  Land Satellites Data System Science Research and Development (LSRD)
at the USGS EROS

HISTORY:
Date         Programmer       Reason
---------    ---------------  -------------------------------------
2/12/2012    Gail Schmidt     Original Development (based on input routines
                              from the LEDAPS lndsr application)

NOTES:
******************************************************************************/
bool append_meta
(
    char *cbuf,  /* I/O: input metadata buffer */
    int *ic,     /* I/O: index of current location in metadata buffer */
    char *s      /* I: string to append to the metadata buffer */
)
{
    int nc, i;
  
    /* Validate the string and number of attributes */
    if (ic < 0)
        return false;
    nc = strlen(s);
    if (nc <= 0)
        return false;
    if (*ic + nc > MYHDF_MAX_NATTR_VAL)
        return false;
  
    /* Add the string to the metadata */
    for (i = 0; i < nc; i++)
    {
        cbuf[*ic] = s[i];
        (*ic)++;
    }
  
    cbuf[*ic] = '\0';
  
    return true;
}

/******************************************************************************
MODULE:  put_space_def_hdf

PURPOSE:  Write the spatial definition attributes to the HDF file and move
the SDSs to the Grid.

RETURN VALUE:
Type = int
Value      Description
-----      -----------
-1         Error occurred writing the metadata to the HDF file
0          Successful completion

PROJECT:  Land Satellites Data System Science Research and Development (LSRD)
at the USGS EROS

HISTORY:
Date         Programmer       Reason
---------    ---------------  -------------------------------------
2/12/2012    Gail Schmidt     Original Development (based on input routines
                              from the LEDAPS lndsr application)
4/19/2013    Song Guo         Modified for CFmask usage

NOTES:
******************************************************************************/
int put_space_def_hdf
(
    Space_def_t *this,     /* I: space definition structure */
    char *file_name,       /* I: HDF file to write attributes to */
    int nsds,              /* I: number of SDS to write */
    char *sds_names[],     /* I: array of SDS names to write */
    int *sds_types,        /* I: array of types for each SDS */
    char *grid_name        /* I: name of the grid to write the SDSs to */
)
{
    int32 sds_file_id;
    char FUNC_NAME[] = "put_space_def_hdf";   /* function name */
    char errmsg[STR_SIZE];    /* error message */
    char struct_meta[MYHDF_MAX_NATTR_VAL];
    char cbuf[MYHDF_MAX_NATTR_VAL];
    char *dim_names[2] = {"YDim", "XDim"};
    int ic;
    Map_coord_t lr_corner;
    double ds, dl, dy, dx;
    double sin_orien, cos_orien;
    Myhdf_attr_t attr;
    int isds;
    int ip;
    double f_fractional, f_integral;
    char *cproj;
    double dval[1];
    char *ctype;
    int it;
    int32 hdf_id;
    int32 vgroup_id[3];
    int32 sds_index, sds_id;
  
    /* Check inputs */
    if (nsds <= 0) 
    {
        sprintf (errmsg, "Invalid number of SDSs for writing (less than 0)");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    for (isds = 0; isds < nsds; isds++)
    {
        if (strlen(sds_names[isds]) < 1)  
        {
            sprintf (errmsg, "Invalid SDS name: %s", sds_names[isds]);
            error_handler (true, FUNC_NAME, errmsg);
            return (-1);
        }
    }
    if (strlen(grid_name) < 1)  
    {
        sprintf (errmsg, "Invalid grid name (empty string)");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Put header */
    ic = 0;
    sprintf (cbuf, 
        "GROUP=SwathStructure\n" 
        "END_GROUP=SwathStructure\n" 
        "GROUP=GridStructure\n" 
        "\tGROUP=GRID_1\n");
  
    if (!append_meta (struct_meta, &ic, cbuf))
    {
        sprintf (errmsg, "Error appending to the start of the metadata string");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Compute lower right corner */
    dl = this->img_size.l * this->pixel_size;
    ds = this->img_size.s * this->pixel_size;
  
    sin_orien = sin(this->orientation_angle);
    cos_orien = cos(this->orientation_angle);
  
    dy = (ds * sin_orien) - (dl * cos_orien);
    dx = (ds * cos_orien) + (dl * sin_orien);
  
    lr_corner.y = this->ul_corner.y + dy;
    lr_corner.x = this->ul_corner.x + dx;
  
    /* Get the projection name string */
    cproj = (char *)NULL;
    for (ip = 0; ip < PROJ_NPROJ; ip++)
    {
        if (this->proj_num == ip)
        { 
            cproj = Proj_type[ip].short_name;
            break;
        }
    }
    if (cproj == (char *)NULL)
    {
        sprintf (errmsg, "Error getting the projection name string");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Put Grid information */
    sprintf(cbuf, 
        "\t\tGridName=\"%s\"\n" 
        "\t\tXDim=%d\n" 
        "\t\tYDim=%d\n" 
        "\t\tUpperLeftPointMtrs=(%.6f,%.6f)\n" 
        "\t\tLowerRightMtrs=(%.6f,%.6f)\n" 
        "\t\tProjection=GCTP_%s\n", 
        grid_name, 
        this->img_size.s, this->img_size.l, 
        this->ul_corner.x, this->ul_corner.y,
        lr_corner.x, lr_corner.y, 
        cproj);
  
    if (!append_meta (struct_meta, &ic, cbuf))
    {
        sprintf (errmsg, "Error appending to metadata string (grid information "
            "start)");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    if (this->proj_num == PROJ_UTM  ||  this->proj_num == PROJ_SPCS)
    {
        sprintf (cbuf, "\t\tZoneCode=%d\n", this->zone);
        if (!append_meta (struct_meta, &ic, cbuf))
        {
            sprintf (errmsg, "Error appending to metadata string (zone "
                "number)");
            error_handler (true, FUNC_NAME, errmsg);
            return (-1);
        }
    }
    else
    {
        sprintf (cbuf, "\t\tProjParams=(");
        if (!append_meta (struct_meta, &ic, cbuf))
        {
            sprintf (errmsg, "Error appending to metadata string (grid "
                "projection parameters start)");
            error_handler (true, FUNC_NAME, errmsg);
            return (-1);
        }
  
        for (ip = 0; ip < NPROJ_PARAM_HDFEOS; ip++)
        {
            f_fractional = modf (this->proj_param[ip], &f_integral);
            if (fabs (f_fractional) < 0.5e-6)
            {
                if (ip < (NPROJ_PARAM_HDFEOS - 1)) 
                    sprintf(cbuf, "%g,", this->proj_param[ip]);
                else 
                    sprintf(cbuf, "%g)", this->proj_param[ip]);
            }
            else
            {
                if (ip < (NPROJ_PARAM_HDFEOS + 1)) 
                    sprintf(cbuf, "%.6f,", this->proj_param[ip]);
                else 
                    sprintf(cbuf, "%.6f)", this->proj_param[ip]);
            }
  
            if (!append_meta (struct_meta, &ic, cbuf))
            {
                sprintf (errmsg, "Error appending to metadata string ("
                    "individual grid projection parameters)");
                error_handler (true, FUNC_NAME, errmsg);
                return (-1);
            }
        }
        sprintf(cbuf, "\n");
  
        if (!append_meta (struct_meta, &ic, cbuf))
        {
            sprintf (errmsg, "Error appending to metadata string (grid "
                "projection parameters end)");
            error_handler (true, FUNC_NAME, errmsg);
            return (-1);
        }
    }
  
    sprintf (cbuf, 
        "\t\tSphereCode=%d\n" 
        "\t\tGridOrigin=HDFE_GD_UL\n",
        this->sphere);
  
    if (!append_meta (struct_meta, &ic, cbuf))
    {
        sprintf (errmsg, "Error appending to metadata string (grid information "
            "end)");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Put SDS group */
    sprintf (cbuf, 
        "\t\tGROUP=Dimension\n" 
        "\t\tEND_GROUP=Dimension\n"
        "\t\tGROUP=DataField\n");
  
    if (!append_meta (struct_meta, &ic, cbuf))
    {
        sprintf (errmsg, "Error appending to metadata string (SDS group "
            "start)");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    for (isds = 0; isds < nsds; isds++)
    {
        /* Get the hdf type name string */
        ctype = (char *)NULL;
        for (it = 0; it < SPACE_NTYPE_HDF; it++)
        {
            if (sds_types[isds] == space_hdf_type[it].type)
            { 
                ctype = space_hdf_type[it].name;
                break;
            }
        }

        if (ctype == (char *)NULL)
        {
              sprintf (errmsg, "Error getting hdf type name string");
              error_handler (true, FUNC_NAME, errmsg);
              return (-1);
        }
  
        sprintf (cbuf, 
            "\t\t\tOBJECT=DataField_%d\n"
            "\t\t\t\tDataFieldName=\"%s\"\n"
            "\t\t\t\tDataType=%s\n"
            "\t\t\t\tDimList=(\"%s\",\"%s\")\n"
            "\t\t\tEND_OBJECT=DataField_%d\n",
            isds+1, sds_names[isds], ctype, dim_names[0], dim_names[1], isds+1);
  
        if (!append_meta (struct_meta, &ic, cbuf))
        {
            sprintf (errmsg, "Error appending to metadata string (SDS group)");
            error_handler (true, FUNC_NAME, errmsg);
            return (-1);
        }
    }
  
    sprintf (cbuf, 
      "\t\tEND_GROUP=DataField\n" 
      "\t\tGROUP=MergedFields\n" 
      "\t\tEND_GROUP=MergedFields\n");
  
    if (!append_meta (struct_meta, &ic, cbuf))
    {
        sprintf (errmsg, "Error appending to metadata string (SDS group end)");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Put trailer */
    sprintf (cbuf, 
        "\tEND_GROUP=GRID_1\n"
        "END_GROUP=GridStructure\n"
        "GROUP=PointStructure\n"
        "END_GROUP=PointStructure\n"
        "END\n");
  
    if (!append_meta (struct_meta, &ic, cbuf))
    {
        sprintf (errmsg, "Error appending to metadata string (tail)");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Write file attributes */
    sds_file_id = SDstart ((char *)file_name, DFACC_RDWR);
    if (sds_file_id == HDF_ERROR) 
    {
        sprintf (errmsg, "Error opening file for SD access: %s", file_name);
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    
    attr.type = DFNT_FLOAT64;
    attr.nval = 1;
    attr.name = SPACE_ORIENTATION_ANGLE_HDF;
    dval[0] = (double)this->orientation_angle * DEG;
    if (!PutAttrDouble(sds_file_id, &attr, dval))
    {
        sprintf (errmsg, "Error writing attribute (orientation angle)");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    attr.type = DFNT_FLOAT64;
    attr.nval = 1;
    attr.name = SPACE_PIXEL_SIZE_HDF;
    dval[0] = (double)this->pixel_size;
    if (!PutAttrDouble(sds_file_id, &attr, dval))
    {
        sprintf (errmsg, "Error writing attribute (pixel_size)");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    attr.type = DFNT_CHAR8;
    attr.nval = strlen(struct_meta);
    attr.name = SPACE_STRUCT_METADATA;
    if (!PutAttrString(sds_file_id, &attr, struct_meta))
    {
        sprintf (errmsg, "Error writing attribute (struct_meta)");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    if (SDend (sds_file_id) == HDF_ERROR) 
    {
        sprintf (errmsg, "Error ending SD access");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Setup the HDF Vgroup */
    hdf_id = Hopen ((char *)file_name, DFACC_RDWR, 0);
    if (hdf_id == HDF_ERROR) 
    {
        sprintf (errmsg, "Error opening the HDF file for Vgroup access: %s",
            file_name);
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Start the Vgroup access */
    if (Vstart (hdf_id) == HDF_ERROR) 
    {
        sprintf (errmsg, "Error starting Vgroup access");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Create Root Vgroup for Grid */
    vgroup_id[0] = Vattach (hdf_id, -1, "w");
    if (vgroup_id[0] == HDF_ERROR) 
    {
        sprintf (errmsg, "Error getting Grid Vgroup ID");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    if (Vsetname (vgroup_id[0], grid_name) == HDF_ERROR) 
    {
        sprintf (errmsg, "Error setting Grid Vgroup name");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    if (Vsetclass (vgroup_id[0], "GRID") == HDF_ERROR) 
    {
        sprintf (errmsg, "Error setting Grid Vgroup class");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Create Data Fields Vgroup */
    vgroup_id[1] = Vattach (hdf_id, -1, "w");
    if (vgroup_id[1] == HDF_ERROR) 
    {
        sprintf (errmsg, "Error getting Data Fields Vgroup ID");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    if (Vsetname (vgroup_id[1], "Data Fields") == HDF_ERROR) 
    {
        sprintf (errmsg, "Error setting Data Fields Vgroup name");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    if (Vsetclass (vgroup_id[1], "GRID Vgroup") == HDF_ERROR) 
    {
        sprintf (errmsg, "Error setting Data Fields Vgroup class");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    if (Vinsert (vgroup_id[0], vgroup_id[1]) == HDF_ERROR) 
    {
        sprintf (errmsg, "Error inserting Data Fields Vgroup");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Create Attributes Vgroup */
    vgroup_id[2] = Vattach (hdf_id, -1, "w");
    if (vgroup_id[2] == HDF_ERROR) 
    {
        sprintf (errmsg, "Error getting attributes Vgroup ID");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    if (Vsetname (vgroup_id[2], "Grid Attributes") == HDF_ERROR) 
    {
        sprintf (errmsg, "Error setting attributes Vgroup name");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    if (Vsetclass (vgroup_id[2], "GRID Vgroup") == HDF_ERROR) 
    {
        sprintf (errmsg, "Error setting attributes Vgroup class");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    if (Vinsert (vgroup_id[0], vgroup_id[2]) == HDF_ERROR) 
    {
        sprintf (errmsg, "Error inserting attributes Vgroup");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Attach SDSs to Data Fields Vgroup */
    sds_file_id = SDstart ((char *)file_name, DFACC_RDWR);
    if (sds_file_id == HDF_ERROR) 
    {
        sprintf (errmsg, "Error opening output file for SD access");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    for (isds = 0; isds < nsds; isds++)
    {
        sds_index = SDnametoindex (sds_file_id, sds_names[isds]);
        if (sds_index == HDF_ERROR) 
        {
            sprintf (errmsg, "Error getting SDS index for SDS[%d]: %s", isds,
                sds_names[isds]);
            error_handler (true, FUNC_NAME, errmsg);
            return (-1);
        }

        sds_id = SDselect (sds_file_id, sds_index);
        if (sds_id == HDF_ERROR) 
        {
            sprintf (errmsg, "Error getting SDS ID");
            error_handler (true, FUNC_NAME, errmsg);
            return (-1);
        }

        if (Vaddtagref (vgroup_id[1], DFTAG_NDG, SDidtoref(sds_id)) == 
            HDF_ERROR) 
        {
            sprintf (errmsg, "Error adding reference tag to SDS");
            error_handler (true, FUNC_NAME, errmsg);
            return (-1);
        }

        if (SDendaccess (sds_id) == HDF_ERROR) 
        {
            sprintf (errmsg, "Error ending access to SDS");
            error_handler (true, FUNC_NAME, errmsg);
            return (-1);
        }
    }
    
    if (SDend (sds_file_id) == HDF_ERROR) 
    {
        sprintf (errmsg, "Error ending SD access");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Detach Vgroups */
    if (Vdetach (vgroup_id[0]) == HDF_ERROR) 
    {
        sprintf (errmsg, "Error detaching from Grid Vgroup");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    if (Vdetach (vgroup_id[1]) == HDF_ERROR) 
    {
        sprintf (errmsg, "Error detaching from Data Fields Vgroup");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    if (Vdetach (vgroup_id[2]) == HDF_ERROR) 
    {
        sprintf (errmsg, "Error detaching from Attributes Vgroup");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }

    /* Close access */
    if (Vend (hdf_id) == HDF_ERROR) 
    {
        sprintf (errmsg, "Error ending Vgroup access");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    if (Hclose (hdf_id) == HDF_ERROR) 
    {
        sprintf (errmsg, "Error ending HDF access");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    return (0);
}


/******************************************************************************
MODULE:  get_space_def_hdf

PURPOSE:  Read the spatial definition attributes from the HDF file.

RETURN VALUE:
Type = int
Value      Description
-----      -----------
-1         Error occurred reading the metadata from the HDF file
0          Successful completion

PROJECT:  Land Satellites Data System Science Research and Development (LSRD)
at the USGS EROS

HISTORY:
Date         Programmer       Reason
---------    ---------------  -------------------------------------
2/12/2012    Gail Schmidt     Original Development (based on input routines
                              from the LEDAPS lndsr application)
4/19/2013    Song Guo         Modified for CFmask usage

NOTES:
******************************************************************************/
int get_space_def_hdf
(
    Space_def_t *this,   /* I/O: spatial information structure which will be
                                 populated from the metadata */
    char *file_name,     /* I: name of the HDF file to read */
    char *grid_name      /* I: name of the grid to read metadata from */
)
{
    char FUNC_NAME[] = "get_space_def_hdf";   /* function name */
    char errmsg[STR_SIZE];    /* error message */
    int32 gd_id;              /* HDF-EOS grid ID */
    int32 gd_file_id;         /* HDF-EOS file ID */
    int32 sds_file_id;        /* HDF file ID */
    int32 xdim_size;          /* number of elements in the x dimension */
    int32 ydim_size;          /* number of elements in the y dimension */
    float64 ul_corner[2];     /* UL corner projection coords (x,y) */
    float64 lr_corner[2];     /* LR corner projection coords (x,y) */
    int32 proj_num;           /* projection number */
    int32 zone;               /* UTM zone */
    int32 sphere;             /* sphere number */
    float64 proj_param[NPROJ_PARAM_HDFEOS];   /* projection parameters */
    int ip;                   /* looping variable */
    double dval[MYHDF_MAX_NATTR_VAL];  /* double attribute value read */
    Myhdf_attr_t attr;        /* HDF attributes read */

    /* Initialize the spatial information data structure */
    this->proj_num = -1;
    for (ip = 0; ip < NPROJ_PARAM; ip++)
        this->proj_param[ip] = 0.0;
    this->pixel_size = -1.0;
    this->ul_corner.x = -1.0;
    this->ul_corner.y = -1.0;
    this->ul_corner_set = false;
    this->img_size.l = -1;
    this->img_size.s = -1;
    this->zone = 0;
    this->zone_set = false;
    this->sphere = -1;
    this->isin_type = SPACE_NOT_ISIN;
    this->orientation_angle = 0.0;
  
    /* Open the HDF-EOS file for reading */
    gd_file_id = GDopen ((char *)file_name, DFACC_READ);
    if (gd_file_id == HDF_ERROR) 
    {
        sprintf (errmsg, "Error opening the HDF-EOS file: %s", file_name);
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Attach to the grid */
    gd_id = GDattach (gd_file_id, grid_name);
    if (gd_id == HDF_ERROR) 
    {
        sprintf (errmsg, "Error attaching to HDF-EOS grid: %s", grid_name);
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Get grid information */
    if (GDgridinfo (gd_id, &xdim_size, &ydim_size, ul_corner, lr_corner) ==
        HDF_ERROR)
    {
        sprintf (errmsg, "Error getting the HDF-EOS grid information");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    this->img_size.l = ydim_size;
    this->img_size.s = xdim_size;
    this->ul_corner.x = ul_corner[0];
    this->ul_corner.y = ul_corner[1];
    this->ul_corner_set = true;
  
    if (GDprojinfo (gd_id, &proj_num, &zone, &sphere, proj_param) == HDF_ERROR)
    {
        sprintf (errmsg, "Error getting HDF-EOS map projection information");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    this->proj_num = proj_num;
    if (this->proj_num == PROJ_UTM  ||  this->proj_num == PROJ_SPCS)
    {
        this->zone = zone;
        this->zone_set = true;
    }
    this->sphere = sphere;
    for (ip = 0; ip < NPROJ_PARAM_HDFEOS; ip++)
        this->proj_param[ip] = proj_param[ip];
  
    /* Close the HDF-EOS file */
    if (GDclose (gd_file_id) == HDF_ERROR) 
    {
        sprintf (errmsg, "Error closing the HDF-EOS file.");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    /* Read file attributes */
    sds_file_id = SDstart ((char *)file_name, DFACC_READ);
    if (sds_file_id == HDF_ERROR) 
    {
        sprintf (errmsg, "Error opening HDF file for SD access: %s", file_name);
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    
    attr.type = DFNT_FLOAT64;
    attr.nval = 1;
    attr.name = SPACE_ORIENTATION_ANGLE_HDF;
    if (!GetAttrDouble(sds_file_id, &attr, dval))
    {
        sprintf (errmsg, "Error reading attribute (orientation angle)");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    if (attr.nval != 1)
    {
        sprintf (errmsg, "Error invalid number of values (orientation angle)");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    this->orientation_angle = dval[0] * RAD;   /* convert to radians */
  
    attr.type = DFNT_FLOAT64;
    attr.nval = 1;
    attr.name = SPACE_PIXEL_SIZE_HDF;
    if (!GetAttrDouble(sds_file_id, &attr, dval))
    {
        sprintf (errmsg, "Error reading attribute (pixel size)");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    if (attr.nval != 1)
    {
        sprintf (errmsg, "Error invalid number of values (pixel size)");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
    this->pixel_size = (float)dval[0];
  
    if (SDend(sds_file_id) == HDF_ERROR) 
    {
        sprintf (errmsg, "Error ending SD access");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }
  
    return (0);
}
