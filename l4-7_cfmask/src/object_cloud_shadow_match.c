#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "2d_array.h"
#include "input.h"
#include "output.h"
#include "const.h"

#define MAX_CLOUD_TYPE 2000000
#define MIN_CLOUD_OBJ 9
#define PI (3.141592653589793238)

static int num_clouds = 0;

typedef struct cloud_node_t{
    int value;
    int16 row;
    int16 col;
    struct cloud_node_t* parent;
    struct cloud_node_t* child;
}cloud_node;

/******************************************************************************
MODULE:  viewgeo

PURPOSE: Calculate the geometric parameters needed for the cloud/shadow match

RETURN: None

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES:
******************************************************************************/
void viewgeo
(
    int x_ul, /* I: upper left column */
    int y_ul, /* I: upper left row */
    int x_ur, /* I: upper right column */
    int y_ur, /* I: upper right row */
    int x_ll, /* I: lower left column  */
    int y_ll, /* I: lower left row */
    int x_lr, /* I: lower right column */
    int y_lr, /* I: lower right row */
    float *a, /* O: coefficient */
    float *b, /* O: coefficient */
    float *c, /* O: coefficient */
    float *omiga_par, /* O: angle of parallel line (in pi) */
    float *omiga_per  /* O: angle of vertical line (in pi) */
)
{
    float x_u, x_l, y_u, y_l; /* intermediate variables */
    float k_ulr = 1.0;  /* intermediate slope value */
    float k_llr = 1.0;  /* intermediate slope value */
    float k_aver;       /* mean slope value */

    x_u=(float)(x_ul+x_ur)/2.0;
    x_l=(float)(x_ll+x_lr)/2.0;
    y_u=(float)(y_ul+y_ur)/2.0;
    y_l=(float)(y_ll+y_lr)/2.0;

    if (x_ul != x_ur)
        k_ulr=(float)(y_ul-y_ur)/(float)(x_ul-x_ur);
                     /* get k of upper left and right points */
    else
        k_ulr = 0.0;
    if (x_ll != x_lr)
        k_llr=(float)(y_ll-y_lr)/(float)(x_ll-x_lr);
                    /* get k of lower left and right points */
    else
        k_llr = 0.0;

    k_aver=(k_ulr+k_llr)/2.0;
    *omiga_par=atan(k_aver); /* get the angle of parallel lines k (in pi) */

    /* AX(j)+BY(i)+C=0 */
    *a=y_u-y_l;
    *b=x_l-x_u;
    *c=y_l*x_u-x_l*y_u;

    *omiga_per=atan((*b)/(*a)); /* get the angle which is perpendicular to the
                                   trace line */
}


/******************************************************************************
MODULE:  mat_truecloud

PURPOSE:  Calculate shadow pixel locations of a true cloud segment

RETURN: None

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES:
******************************************************************************/
/* mat_truecloud function */
void mat_truecloud
(
    int *x,   /*I: input pixel cloumn */
    int *y,   /*I: input pixel row */
    int array_length, /*I: number of input array */
    float *h, /*I: cloud pixel height */
    float a,  /*I: coefficient */
    float b,  /*I: coefficient */
    float c,  /*I: coefficient */
    float omiga_par, /*I: angle of parellel line */
    float omiga_per, /*I: angle of parellel line */
    float *x_new, /*O: output pixel cloumn */
    float *y_new  /*O: output pixel row */
)
{
    float dist;      /* distance */
    float dist_par;  /* distance in parallel direction */
    float dist_move; /* distance moved */
    float delt_x;    /* change in column */
    float delt_y;    /* change in row */

    float height=705000.0; /* average Landsat 4,5,&7 height (m) */
    int i;

    for (i = 0; i < array_length; i++)
    {
        dist = (a*(float)x[i]+b*(float)y[i]+c)/(sqrt(a*a+b*b));
                 /* from the cetral perpendicular (unit: pixel) */
        dist_par = dist/cos(omiga_per-omiga_par);
        dist_move = (dist_par*h[i])/height; /* cloud move distance (m) */
        delt_x = dist_move*cos(omiga_par);
        delt_y = dist_move*sin(omiga_par);

        x_new[i] = x[i] + delt_x; /* new x, j */
        y_new[i] = y[i] + delt_y; /* new y, i */
    }
}

/******************************************************************************
MODULE:  Find

PURPOSE:  Find the parent node

RETURN: parent node

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES:
******************************************************************************/
cloud_node* Find(cloud_node* node) {
    cloud_node* temp;
    /* Find the root */
    cloud_node* root = node;

    if (node->parent == node)
     return node;

    while (root->parent != root) {
        root = root->parent;
    }
    /* Update the parent pointers */
    while (node->parent != node) {
        temp = node->parent;
        node->parent = root;
        node = temp;
    }
    return root;
}

/******************************************************************************
MODULE:  Union_child

PURPOSE: Combine two cloud node child together

RETURN: None

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES:
******************************************************************************/
void Union_child(cloud_node* node1, cloud_node* node2) {
    node1->child = node2;
    node2->value = node1->value;
    while (node2->child != node2)
    {
         node2 = node2->child;
         node2->value = node1->value;
    }
}

/******************************************************************************
MODULE:  Find_child

PURPOSE: Find the child node

RETURN: None

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES:
******************************************************************************/
cloud_node* Find_child(cloud_node* node) {
    /* Find the root */
    cloud_node* root = node;
    if (node->child == node)
    {
        return node;
    }

    while (root->child != root) {
        root = root->child;
    }

    return root;
}

/******************************************************************************
MODULE:  find_minimum

PURPOSE: Find the minimum value and its location in an array

RETURN: None

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES:
******************************************************************************/
void find_minimum
(
    int *array, /*I: input array */
    int nums,   /*I: number of elements in input array */
    int *min,   /*O: minimum value found */
    int *index  /*O: minimum value location */
)
{
    int i; /* loop index */

    *min = 0;
    *index = 0;
    for (i = 0; i < nums; i++)
    {
        if (array[i] > 0)
        {
            *min = array[i];
            *index = i;
            break;
        }
    }
    for (i = 0; i < nums; i++)
    {
        if (array[i] > 0 && array[i] < *min)
        {
            *min = array[i];
            *index = i;
        }
    }
}

/******************************************************************************
MODULE:  label

PURPOSE: label each cloud pixel with a cloud number

RETURN: None

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES:
******************************************************************************/
void label
(
    unsigned char **pixel_mask, /*I: cloud pixel mask */
    int nrows,                  /*I: number of rows */
    int ncols,                  /*I: number of columns */
    cloud_node **cloud,         /*O: cloud pixel node */
    unsigned int *obj_num,               /*O: cloud number */
    unsigned int **first_cloud_node      /*O: fisrt cloud pixel node */
)
{
    int row, col; /* loop indices */
    int array[4]; /* array of 4 elements */
    int min;      /* minimum value */
    int index;    /* minimum value location */

    for (row = 0; row < nrows; row++)
    {
        for (col = 0; col <ncols; col++)
        {
            if (pixel_mask[row][col] & (1 << CLOUD_BIT))
            {
                if (row > 0 && col > 0 && (pixel_mask[row-1][col-1] & 
                                        (1 << CLOUD_BIT)))
                    array[0] = cloud[row-1][col-1].value;
                else
                    array[0] = 0;
                if ( row > 0 && (pixel_mask[row-1][col] & (1 << CLOUD_BIT)))
                    array[1] = cloud[row-1][col].value;
                else
                    array[1] = 0;
                if (row > 0 && (col < ncols-1) && (pixel_mask[row-1][col+1] &
                                                   (1 << CLOUD_BIT)))
                    array[2] = cloud[row-1][col+1].value;
                else
                    array[2] = 0;
                if (col > 0 && (pixel_mask[row][col-1] & (1 << CLOUD_BIT)))
                    array[3] = cloud[row][col-1].value;
                else
                    array[3] = 0;

                /* The cloud pixel will be labeled as a new cloud if neighboring
                   pixels before it are not cloud pixels, otherwise it will be
                   labeled as lowest cloud number neighboring it */
                find_minimum(array, 4, &min, &index);
                if (min == 0)
                {
                    num_clouds++;
                    cloud[row][col].value = num_clouds;
                    cloud[row][col].row = row;
                    cloud[row][col].col = col;
                    first_cloud_node[0][num_clouds] = row;
                    first_cloud_node[1][num_clouds] = col;
                }
                else
                {
                    cloud[row][col].value = min;
                    cloud[row][col].row = row;
                    cloud[row][col].col = col;
                    if (index == 0)
                    {
                        cloud[row][col].parent = Find(&cloud[row-1][col-1]);
                        Find_child(&cloud[row-1][col-1])->child =
                            &cloud[row][col];
                    }
                    else if (index == 1)
                    {
                        cloud[row][col].parent = Find(&cloud[row-1][col]);
                        Find_child(&cloud[row-1][col])->child =
                            &cloud[row][col];
                    }
                    else if (index == 2)
                    {
                        cloud[row][col].parent = Find(&cloud[row-1][col+1]);
                        Find_child(&cloud[row-1][col+1])->child =
                            &cloud[row][col];
                    }
                    else if (index == 3)
                    {
                        cloud[row][col].parent = Find(&cloud[row][col-1]);
                        Find_child(&cloud[row][col-1])->child =
                            &cloud[row][col];
                    }
                    else
                         continue;

                    /* If two neighboring pixels are labeled as different cloud
                       numbers, the two cloud pixels are relabeled as the same
                       cloud */
                    if ((row > 0 && col > 0
                        && (pixel_mask[row-1][col-1] & (1 << CLOUD_BIT)))
                        && (cloud[row-1][col-1].value != min))
                    {
                       if (index == 1)
                       {
                           Union_child(Find_child(&cloud[row-1][col]),
                                 Find(&cloud[row-1][col-1]));
                           Find(&cloud[row-1][col-1])->parent =
                                 Find(&cloud[row-1][col]);
                       }
                       else if (index == 2)
                       {
                           Union_child(Find_child(&cloud[row-1][col+1]),
                                 Find(&cloud[row-1][col-1]));
                           Find(&cloud[row-1][col-1])->parent =
                                 Find(&cloud[row-1][col+1]);
                       }
                       else if (index == 3)
                       {
                           Union_child(Find_child(&cloud[row][col-1]),
                                 Find(&cloud[row-1][col-1]));
                           Find(&cloud[row-1][col-1])->parent =
                                 Find(&cloud[row][col-1]);
                       }
                       else
                           continue;
                    }
                    if ((row > 0 && (pixel_mask[row-1][col] & (1 << CLOUD_BIT)))
                        && (cloud[row-1][col].value != min))
                    {
                        if (index == 0)
                        {
                            Union_child(Find_child(&cloud[row-1][col-1]),
                                Find(&cloud[row-1][col]));
                            Find(&cloud[row-1][col])->parent =
                                Find(&cloud[row-1][col-1]);
                        }
                        else if (index == 2)
                        {
                            Union_child(Find_child(&cloud[row-1][col+1]),
                                Find(&cloud[row-1][col]));
                            Find(&cloud[row-1][col])->parent =
                                Find(&cloud[row-1][col+1]);
                        }
                        else if (index == 3)
                        {
                            Union_child(Find_child(&cloud[row][col-1]),
                                Find(&cloud[row-1][col]));
                            Find(&cloud[row-1][col])->parent =
                                Find(&cloud[row][col-1]);
                        }
                        else
                            continue;
                    }
                    if ((row > 0 && (col < ncols-1)
                         && (pixel_mask[row-1][col+1] & (1 << CLOUD_BIT)))
                        && (cloud[row-1][col+1].value != min))
                    {
                        if (index == 0)
                        {
                            Union_child(Find_child(&cloud[row-1][col-1]),
                                Find(&cloud[row-1][col+1]));
                            Find(&cloud[row-1][col+1])->parent =
                                Find(&cloud[row-1][col-1]);
                        }
                        else if (index == 1)
                        {
                            Union_child(Find_child(&cloud[row-1][col]),
                                Find(&cloud[row-1][col+1]));
                            Find(&cloud[row-1][col+1])->parent =
                                Find(&cloud[row-1][col]);
                        }
                        else if (index == 3)
                        {
                            Union_child(Find_child(&cloud[row][col-1]),
                                Find(&cloud[row-1][col+1]));
                            Find(&cloud[row-1][col+1])->parent =
                                Find(&cloud[row][col-1]);
                        }
                        else
                            continue;
                    }
                    if ((col > 0 && (pixel_mask[row][col-1] & (1 << CLOUD_BIT)))
                        && (cloud[row][col-1].value != min))
                    {
                        if (index == 0)
                        {
                            Union_child(Find_child(&cloud[row-1][col-1]),
                                Find(&cloud[row][col-1]));
                            Find(&cloud[row][col-1])->parent =
                                Find(&cloud[row-1][col-1]);
                        }
                        else if (index == 1)
                        {
                            Union_child(Find_child(&cloud[row-1][col]),
                                Find(&cloud[row][col-1]));
                            Find(&cloud[row][col-1])->parent =
                                Find(&cloud[row-1][col]);
                        }
                        else if (index == 2)
                        {
                            Union_child(Find_child(&cloud[row-1][col+1]),
                                Find(&cloud[row][col-1]));
                            Find(&cloud[row][col-1])->parent =
                                Find(&cloud[row-1][col+1]);
                        }
                        else
                            continue;
                    }
                }
            }
        }
    }
    printf("First pass in labeling algorithm done\n");


    /* The second pass labels all cloud pixels according two their root
       parent cloud pixel values */
    for (row = 0; row < nrows; row++)
    {
        for (col = 0; col < ncols; col++)
        {
            if (pixel_mask[row][col] & (1 << CLOUD_BIT))
            {
                cloud[row][col].value = Find(&cloud[row][col])->value;
                obj_num[cloud[row][col].value]++;
            }
        }
    }
    printf("Second pass in labeling algorithm done\n");
}

/******************************************************************************
MODULE:  image_dilate

PURPOSE: Dilate the image with a n x n rectangular buffer

RETURN: None

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
11/18/2013   Song Guo         Original Development

NOTES:
******************************************************************************/
void image_dilate
(
    unsigned char **in_mask,      /* I: Mask to be dilated */
    int nrows,                    /* I: Number of rows in the mask */
    int ncols,                    /* I: Number of columns in the mask */
    int idx,                      /* I: pixel buffer 2 * idx + 1 */
    int bit,                      /* I: type of image to dilute */
    unsigned char **out_mask      /* O: Mask after dilate */
)
{
    int row, col, ir, ic; /* loop indices */
    unsigned char mask;   /* temporarily output pixel mask value */

    for (row = 0; row < nrows; row++)
    {
        for (col = 0; col < ncols; col++)
        {
            mask = 0;
            for (ir = 0; ir < idx + 1; ir++)
            {
                for (ic = 0; ic < idx + 1; ic++)
                {
                    if (((row-ir) > 0) && ((col-ic) > 0) && mask != 1)
                    {
                        if (in_mask[row-ir][col-ic] & (1 << bit))
                            mask = 1;
                    }
                    if (((row-ir) > 0) && ((col+ic) < (ncols-1)) && mask != 1)
                    {
                        if (in_mask[row-ir][col+ic] & (1 << bit))
                            mask = 1;
                    }
                    if (((row+ir) < (nrows-1)) && ((col-ic) > 0) && mask != 1)
                    {
                        if (in_mask[row+ir][col-ic] & (1 << bit))
                            mask = 1;
                    }
                    if (((row+ir) < (nrows-1)) && ((col+ic) < (ncols-1))
                        && mask != 1)
                    {
                        if (in_mask[row+ir][col+ic] & (1 << bit))
                            mask = 1;
                    }
                }
            }
            if (mask == 1)
                out_mask[row][col] |= 1 << bit;
            else
                out_mask[row][col] &= ~(1 << bit);
        }
    }
}

/******************************************************************************
MODULE:  object_cloud_shadow_match

PURPOSE: Identify the final shadow pixels by doing a geometric cloud and shadow
         matching which ends in with the maximum cloud and shadow similarity

RETURN: SUCCESS
        FAILURE

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES: All variable names are same as in matlab code
******************************************************************************/
int object_cloud_shadow_match
(
    Input_t *input,             /*I: input structure */
    float ptm,                  /*I: percent of clear-sky pixels */
    float t_templ,              /*I: percentile of low background temp */
    float t_temph,              /*I: percentile of high background temp */
    int cldpix,                 /*I: cloud buffer size */
    int sdpix,                  /*I: shadow buffer size */
    int max_cloud_pixels,       /*I: max cloud pixel number to divide cloud */
    unsigned char **pixel_mask, /*I/O: pixel mask */
    bool verbose                /*I: value to indicate if intermediate messages
                                     be printed */
)
{
    char errstr[MAX_STR_LEN];  /* error string */
    int nrows = input->size.l; /* number of rows */
    int ncols = input->size.s; /* number of columns */
    int row;                   /* row index */
    int col = 0;               /* column index */
    float sun_ele;             /* sun elevation angle */
    float sun_ele_rad;         /* sun elevation angle in radiance */
    float sun_tazi;            /* sun azimuth angle */
    float sun_tazi_rad;        /* sun azimuth angle in radiance */
    int sub_size = 30;         /* pixel size */
    int status;                /* return value */
    int cloud_counter = 0;     /* cloud pixel counter */
    int boundary_counter = 0;  /* boundary pixel counter */
    float revised_ptm;         /* revised percent of cloud */
    float t_similar;           /* similarity threshold */
    float t_buffer;            /* threshold for matching buffering */
    float max_similar = 0.95;  /* max similarity threshold */
    int num_cldoj = 9; /* minimum matched cloud object (pixels) */
    int num_pix = 3;   /* number of inward pixes (240m) for cloud base
                          temperature */
    float a, b, c, omiga_par, omiga_per; /* variables used for viewgeo
                                            routine, see it for detail */
    int i_step;     /* ietration step */
    int x_ul = 0;   /* upper left column */
    int y_ul = 0;   /* upper left row */
    int x_lr = 0;   /* lower right column */
    int y_lr = 0;   /* lower right row */
    int x_ll = 0;   /* lower left column */
    int y_ll = 0;   /* lower left row */
    int x_ur = 0;   /* upper right column */
    int y_ur = 0;   /* upper right row */
    unsigned int *obj_num = NULL; /* cloud object number */
    cloud_node **cloud = NULL; /* cloud node for all pixels */
    unsigned int **cloud_first_node;    /* first cloud node */
    int num;                   /* number */
    int counter = 0;           /* counter */
    int16 **temp = NULL;       /* brightness temperature */
    int cloud_type;            /* cloud type iterator */
    int **xy_type;             /* intermediate variables */
    int **tmp_xy_type;         /* intermediate variables */
    float **tmp_xys;           /* intermediate variables */
    int **orin_xys;            /* intermediate variables */
    int16 *temp_obj;           /* temperature for each cloud */
    int16 temp_obj_max = 0;    /* maximum temperature for each cloud */
    int16 temp_obj_min = 0;    /* minimum temperature for each cloud */
    int index;                 /* loop index */
    float r_obj;               /* cloud radius */
    float pct_obj;             /* percent of edge pixels */
    float t_obj;               /* cloud percentile value */
    float rate_elapse=6.5;     /* wet air lapse rate */
    float rate_dlapse=9.8;     /* dry air lapse rate */
    int max_cl_height; /* Max cloud base height (m) */
    int min_cl_height; /* Min cloud base height (m) */
    int max_height;    /* refined maximum height (m) */
    int min_height;    /* refined minimum height (m) */
    float record_thresh; /* record thresh value */
    float *record_h;     /* record height value */
    int base_h;          /* cloud base height */
    float *h;            /* cloud height */
    float i_xy;          /* intermediate cloud height */
    int out_all;         /* total number of pixels outdside boundary */
    int match_all;       /* total number of matched pixels */
    int total_all;       /* total number of pixels */
    float thresh_match;  /* thresh match value */
    int i;
    cloud_node *node;    /* temporary cloud node */
    int cloud_count = 0; /* cloud counter */
    int shadow_count = 0;/* shadow counter */
    float cloud_shadow_percent;/* cloud shadow percent */
    int extra_clouds;          /* extra clouds needed to 
                                  decrease memory usage */ 
    int number;                /* loop variable */
    int total_num_clouds;      /* total number of clouds after
                                  large clouds division */ 
    cloud_node *temp_node;     /* temporary cloud node */

    /* Dynamic memory allocation */
    unsigned char **cal_mask = NULL;      /* calibration pixel mask */

    cal_mask = (unsigned char **)allocate_2d_array(input->size.l,
                 input->size.s, sizeof(unsigned char));
    if (cal_mask == NULL)
    {
        sprintf (errstr, "Allocating cal_mask memory");
        RETURN_ERROR (errstr, "cloud/shadow match", FAILURE);
    }

    /* Read in potential mask ... */
    /* Solar elevation angle */
    sun_ele = 90 - input->meta.sun_zen;
    sun_ele_rad = (PI / 180.0) * sun_ele;

    /* Solar azimuth angle */
    sun_tazi = input->meta.sun_az - 90;
    sun_tazi_rad = (PI / 180.0) * sun_tazi;

    for (row = 0; row < nrows; row++)
    {
        for (col = 0; col < ncols; col++)
        {
            if (pixel_mask[row][col] & (1 << CLOUD_BIT))
                cloud_counter++;

            /* Boundary layer includes both cloud_mask equals 0 and 1 */
            if (!(pixel_mask[row][col] & (1 << FILL_BIT)))
                boundary_counter++;
        }
    }

    /* Revised percent of cloud on the scene after plcloud */
    if (boundary_counter != 0)
        revised_ptm = (float)cloud_counter / (float)boundary_counter;
    else
        revised_ptm = 0.0;

    if (verbose)
    {
        printf("cloud_counter, boundary_counter = %d, %d\n", cloud_counter,
               boundary_counter);
        printf("Revised percent of cloud = %f\n", revised_ptm);
    }

    /* cloud covers more than 90% of the scene
        => no match => rest are definite shadows */
    if (ptm <= 0.1 || revised_ptm >= 0.90)
    {
        for (row = 0; row < nrows; row++)
        {
            for (col = 0; col < ncols; col++)
            {
                /* No Shadow Match due to too much cloud (>90 percent) 
                   non-cloud pixels are just shadow pixels */
                if (!(pixel_mask[row][col] & (1 << CLOUD_BIT)) &&
                    (!(pixel_mask[row][col] & (1 << FILL_BIT))))
                    pixel_mask[row][col] |= 1 << SHADOW_BIT;
            }
        }
    }
    else
    {
        if (verbose)
        {
            printf("Shadow Match in processing\n");
            printf("Shadow match for cloud object >= %d pixels\n", num_cldoj);
        }
        i_step=rint(2.0*(float)sub_size*tan(sun_ele_rad));
                              /* move 2 pixel at a time */
        if (i_step < (2 * sub_size))
            i_step = 2 * sub_size;   /* Make i_step = 2 * sub_size for polar
                                        large solar zenith angle case */

        /* Get moving direction, the idea is to get the corner rows/cols */
        for (row = 0; row < nrows; row++)
        {
            for (col = 0; col < ncols; col++)
            {
                if (!(pixel_mask[row][col] & (1 << FILL_BIT)))
                {
                    y_ul = row;
                    x_ul = col;
                    break;
                }
            }
        }

        for (col = ncols - 1; col >= 0; col--)
        {
            for (row = 0; row < nrows; row++)
            {
                if (!(pixel_mask[row][col] & (1 << FILL_BIT)))
                {
                    y_ur = row;
                    x_ur = col;
                    break;
                }
            }
        }

        for (col = 0; col < ncols; col++)
        {
            for (row = nrows - 1; row >= 0; row--)
            {
                if (!(pixel_mask[row][col] & (1 << FILL_BIT)))
                {
                    y_ll = row;
                    x_ll = col;
                    break;
                }
            }
        }

        for (row = nrows - 1; row >= 0; row--)
        {
            for (col = ncols - 1; col >= 0; col--)
            {
                if (!(pixel_mask[row][col] & (1 << FILL_BIT)))
                {
                    y_lr = row;
                    x_lr = col;
                    break;
                }
            }
        }

        /* get view angle geometry */
        viewgeo(x_ul,y_ul,x_ur,y_ur,x_ll,y_ll,x_lr,y_lr, &a, &b, &c,
            &omiga_par, &omiga_per);

        /* Allocate memory for segment cloud portion */
        obj_num = (unsigned int *)calloc(MAX_CLOUD_TYPE, sizeof(unsigned int));
        cloud = (cloud_node **)allocate_2d_array(nrows,
               ncols, sizeof(cloud_node));
        cloud_first_node = (unsigned int **)allocate_2d_array(2,
               MAX_CLOUD_TYPE, sizeof(unsigned int));
        if (obj_num == NULL || cloud == NULL || cloud_first_node == NULL)
        {
            sprintf (errstr, "Allocating memory");
            RETURN_ERROR (errstr, "cloud/shadow match", FAILURE);
        }

        /* Initialize the cloud nodes */
        for (row = 0; row < nrows; row++)
        {
            for (col = 0; col <ncols; col++)
            {
                cloud[row][col].value = 0;
                cloud[row][col].row = 0;
                cloud[row][col].col = 0;
                cloud[row][col].parent = &cloud[row][col];
                cloud[row][col].child = &cloud[row][col];
            }
        }

        /* Labeling the cloud pixels */
        label(pixel_mask, nrows, ncols, cloud, obj_num, cloud_first_node);

        total_num_clouds = num_clouds;
        /* The cloud pixels are not counted as cloud pixels if the total number
           of cloud pixels is less than 9 within a cloud cluster */
        for (num = 1; num <= num_clouds; num++)
        {
            if (obj_num[num] <= MIN_CLOUD_OBJ)
                obj_num[num] = 0;
            else
                counter++;
            if ((max_cloud_pixels > 0) && (obj_num[num] > max_cloud_pixels))
            {
                extra_clouds = (int) (obj_num[num] / max_cloud_pixels);
                node = &cloud[cloud_first_node[0][num]][cloud_first_node[1][num]];
                obj_num[num] = max_cloud_pixels;
                int i;
                for (i = 0; i < max_cloud_pixels; i++)
                    node = node->child;
                temp_node = node->child;
                node->child = node;
                for (number = 1; number <= extra_clouds; number++)
                {
                    total_num_clouds++;
                    node = temp_node;
                    cloud_first_node[0][total_num_clouds] = node->row;
                    cloud_first_node[1][total_num_clouds] = node->col;
                    for (i = 0; i < max_cloud_pixels; i++)
                    {
                        if (node->child != node)
                        {
                            node->value = total_num_clouds;
                            node->parent = &cloud[cloud_first_node[0][total_num_clouds]]
                                      [cloud_first_node[1][total_num_clouds]];
                            obj_num[total_num_clouds]++;
                            if (i == max_cloud_pixels - 1)
                            {
                                temp_node = node->child;
                                node->child = node;
                            }
                            else
                                node = node->child;
                        }
                        else
                        {
                            node->value = total_num_clouds;
                            node->parent = &cloud[cloud_first_node[0][total_num_clouds]]
                                      [cloud_first_node[1][total_num_clouds]];
                            obj_num[total_num_clouds]++;
                        }
                    }
                }
            }
        }

        if (verbose)
            printf("Num of real clouds = %d\n", counter);

        /* Cloud_cal pixels are cloud_mask pixels with < 9 pixels removed */
        for (row = 0; row < nrows; row++)
        {
            for (col = 0; col <ncols; col++)
            {
                cal_mask[row][col] &= ~(1 << SHADOW_BIT);
                if ((pixel_mask[row][col] & (1 << CLOUD_BIT))
                    && (!(pixel_mask[row][col] & (1 << FILL_BIT)))
                    && (obj_num[cloud[row][col].value] != 0))
                    cal_mask[row][col] |= 1 << CLOUD_BIT;
                else
                    cal_mask[row][col] &= ~(1 << CLOUD_BIT);
            }
        }

        /* Need to read out whole image brightness temperature for band 6 */
        temp = (int16 **)allocate_2d_array(input->size.l,
                 input->size.s, sizeof(int16));
        if (temp == NULL)
        {
            sprintf (errstr, "Allocating temp memory");
            RETURN_ERROR (errstr, "cloud/shadow match", FAILURE);
        }

        /* Read out thermal band in 2d */
        for (row = 0; row < nrows; row++)
        {
	    if (!GetInputThermLine(input, row))
            {
	        sprintf (errstr, "Reading input thermal data for line %d", row);
	        RETURN_ERROR (errstr,"cloud/shadow match", FAILURE);
	    }
            memcpy(&temp[row][0], &input->therm_buf[0],
                input->size.s * sizeof(int16));
        }

        /* Use iteration to get the optimal move distance, Calulate the
           moving cloud shadow */
        for (cloud_type = 1; cloud_type <= total_num_clouds; cloud_type++)
        {
            if (obj_num[cloud_type] == 0)
                continue;
            else
            {
                /* Update ib Fmask v3.3, for larger (> 10% scene area), use
                   another set of t_similar and t_buffer to address some
                   missing cloud shadow at edge area */
                if (obj_num[cloud_type] <= (int)(0.1 * boundary_counter))
                {
                    t_similar = 0.3;
                    t_buffer = 0.95;
                }
                else
                {
                    t_similar = 0.1;
                    t_buffer = 0.98;
                }

                /* Note: matlab array index starts with 1 and C starts with 0,
                         array(3,1) in matlab is equal to array[2][0] in C */
                min_cl_height = 200;
                max_cl_height = 12000;
                xy_type = (int **)allocate_2d_array(2,
                           obj_num[cloud_type], sizeof(int));
                tmp_xy_type = (int **)allocate_2d_array(2,
                     obj_num[cloud_type], sizeof(int));
                /* corrected for view angle xys */
                tmp_xys = (float **)allocate_2d_array(2,
                          obj_num[cloud_type], sizeof(float));
                /* record the original xys */
                orin_xys = (int **)allocate_2d_array(2,
                      obj_num[cloud_type], sizeof(int));
                if (xy_type == NULL || tmp_xy_type == NULL || tmp_xys == NULL
                    || orin_xys == NULL)
                {
                    sprintf (errstr, "Allocating cloud memory");
                    RETURN_ERROR (errstr, "cloud/shadow match", FAILURE);
                }

                /* Temperature of the cloud object */
                temp_obj = malloc(obj_num[cloud_type] * sizeof(int16));
                if (temp_obj == NULL)
                {
                    sprintf (errstr, "Allocating temp_obj memory");
                    RETURN_ERROR (errstr, "cloud/shadow match", FAILURE);
                }

                temp_obj_max = 0;
                temp_obj_min = 0;
                index = 0;
                node = &cloud[cloud_first_node[0][cloud_type]]
                       [cloud_first_node[1][cloud_type]];
                while (node->child != node)
                {
                    temp_obj[index] = temp[node->row][node->col];
                    if (temp_obj[index] > temp_obj_max)
                        temp_obj_max = temp_obj[index];
                    if (temp_obj[index] < temp_obj_min)
                        temp_obj_min = temp_obj[index];
                    orin_xys[0][index] = node->col;
                    orin_xys[1][index] = node->row;
                    index++;
                    node = node->child;
                }
                temp_obj[index] = temp[node->row][node->col];
                if (temp_obj[index] > temp_obj_max)
                    temp_obj_max = temp_obj[index];
                if (temp_obj[index] < temp_obj_min)
                    temp_obj_min = temp_obj[index];
                orin_xys[0][index] = node->col;
                orin_xys[1][index] = node->row;
                index++;
                obj_num[cloud_type] = index;

                /* the base temperature for cloud
                   assume object is round r_obj is radium of object */
                r_obj=sqrt((float)obj_num[cloud_type]/ (2.0 * PI));

                /* number of inward pixes for correct temperature */
                pct_obj=((r_obj-(float)num_pix)*(r_obj-(float)num_pix)) /
                          (r_obj * r_obj);
                if ((pct_obj-1.0) >= MINSIGMA)
                    pct_obj = 1.0;/* pct of edge pixel should be less than 1 */

                status = prctile(temp_obj, obj_num[cloud_type],temp_obj_min,
                        temp_obj_max, 100.0 * pct_obj, &t_obj);
                if (status != SUCCESS)
                {
                    sprintf (errstr, "Error calling prctile");
                    RETURN_ERROR (errstr, "cloud/shadow match", FAILURE);
                }

                /* refine cloud height range (m) */
                min_height = (int)rint(10.0*(t_templ-t_obj)/rate_dlapse);
                max_height = (int)rint(10.0*(t_temph-t_obj));
                if (min_cl_height < min_height)
                    min_cl_height = min_height;
                if (max_cl_height > max_height)
                    max_cl_height = max_height;

                /* put the edge of the cloud the same value as t_obj */
                for (i = 0; i < obj_num[cloud_type]; i++)
                {
                    if (temp_obj[i] >rint(t_obj))
                         temp_obj[i]=rint(t_obj);
                }

                /* Allocate memory for h and record_h*/
                h = malloc(obj_num[cloud_type] * sizeof(float));
                record_h = calloc(obj_num[cloud_type], sizeof(float));
                if (h == NULL || record_h == NULL)
                {
                    sprintf (errstr, "Allocating h memory");
                    RETURN_ERROR (errstr, "cloud/shadow match", FAILURE);
                }

                /* initialize height and similarity info */
                record_thresh=0.0;
                for (base_h = min_cl_height; base_h <= max_cl_height;
                              base_h+=i_step)
                {
                    for (i = 0; i < obj_num[cloud_type]; i++)
                    {
                        h[i] = (10.0*(t_obj-(float)temp_obj[i]))
                            / rate_elapse+(float)base_h;
                    }

                    /* Get the true postion of the cloud
                       calculate cloud DEM with initial base height */
                    mat_truecloud(orin_xys[0], orin_xys[1],
                        obj_num[cloud_type], h, a, b, c, omiga_par, omiga_per,
                        tmp_xys[0], tmp_xys[1]);

                    out_all = 0;
                    match_all = 0;
                    total_all = 0;
                    for (i = 0; i < obj_num[cloud_type]; i++)
                    {
                        i_xy=h[i]/((float)sub_size*tan(sun_ele_rad));
                        /* The check here can assume to handle the south up
                           north down scene case correctly as azimuth angle
                           needs to be added by 180.0 degree */
                        if ((input->meta.sun_az - 180.0) < MINSIGMA)
                        {
                           xy_type[1][i] =
                               rint(tmp_xys[0][i]-i_xy*cos(sun_tazi_rad));
                           xy_type[0][i] =
                               rint(tmp_xys[1][i]-i_xy*sin(sun_tazi_rad));
                        }
                        else
                        {
                           xy_type[1][i] =
                               rint(tmp_xys[0][i]+i_xy*cos(sun_tazi_rad));
                           xy_type[0][i] =
                               rint(tmp_xys[1][i]+i_xy*sin(sun_tazi_rad));
                        }

                        /* the id that is out of the image */
                        if (xy_type[0][i] < 0 || xy_type[0][i] >= nrows
                            || xy_type[1][i] < 0 || xy_type[1][i] >= ncols)
                        {
                            out_all++;
                        }
                        else
                        {
                            if ((pixel_mask[xy_type[0][i]][xy_type[1][i]] & 
                                  (1 << FILL_BIT))
                                || (cloud[xy_type[0][i]][xy_type[1][i]].value !=
                                cloud_type &&
                                (((pixel_mask[xy_type[0][i]][xy_type[1][i]] &
                                (1 << CLOUD_BIT)) ||
                                (pixel_mask[xy_type[0][i]][xy_type[1][i]] &
                                (1 << FILL_BIT))) ||
                                (pixel_mask[xy_type[0][i]][xy_type[1][i]] &
                                 (1 << SHADOW_BIT)))))
                            {
                                match_all++;
                            }
                            if (cloud[xy_type[0][i]][xy_type[1][i]].value !=
                               cloud_type)
                            {
                                total_all++;
                            }
                        }
                    }
                    match_all += out_all;
                    total_all+=out_all;

                    thresh_match=(float)match_all/(float)total_all;
                    if (((thresh_match - t_buffer*record_thresh) >= MINSIGMA)
                        && (base_h < max_cl_height-i_step) && ((record_thresh
                           - max_similar)<MINSIGMA))
                    {
                        if ((thresh_match - record_thresh) > MINSIGMA)
                        {
                            record_thresh=thresh_match;
                            for (i = 0; i < obj_num[cloud_type]; i++)
                                record_h[i] = h[i];
                        }
                    }
                    else if ((record_thresh - t_similar) > MINSIGMA)
                    {
                        float i_vir;
                        for (i = 0; i < obj_num[cloud_type]; i++)
                        {
                            i_vir = record_h[i] /
                                    ((float)sub_size*tan(sun_ele_rad));
                            /* The check here can assume to handle the south
                               up north down scene case correctly as azimuth
                               angle needs to be added by 180.0 degree */
                            if ((input->meta.sun_az - 180.0) < MINSIGMA)
                            {
                                tmp_xy_type[1][i]=rint(tmp_xys[0][i]-
                                     i_vir*cos(sun_tazi_rad));
                                tmp_xy_type[0][i]=rint(tmp_xys[1][i]-
                                     i_vir*sin(sun_tazi_rad));
                            }
                            else
                            {
                                tmp_xy_type[1][i]=rint(tmp_xys[0][i]+
                                i_vir*cos(sun_tazi_rad));
                                tmp_xy_type[0][i]=rint(tmp_xys[1][i]+
                                i_vir*sin(sun_tazi_rad));
                            }

                            /* put data within range */
                            if (tmp_xy_type[0][i]<0)
                                tmp_xy_type[0][i]=0;
                            if (tmp_xy_type[0][i]>=nrows)
                                tmp_xy_type[0][i]=nrows-1;
                            if (tmp_xy_type[1][i]<0)
                                tmp_xy_type[1][i]=0;
                            if (tmp_xy_type[1][i]>=ncols)
                                tmp_xy_type[1][i]=ncols-1;
                            {
                                cal_mask[tmp_xy_type[0][i]][tmp_xy_type[1][i]] |= 
                                                                 1 << SHADOW_BIT;
                            }
                        }
                        break;
                    }
                    else
                    {
                        record_thresh=0.0;
                        continue;
                    }
                }
                free(h);
                free(record_h);
                h = NULL;
                record_h = NULL;

                /* Free all the memory */
                status = free_2d_array((void **)xy_type);
                if (status != SUCCESS)
                {
                    sprintf (errstr, "Freeing memory: xy_type\n");
                    RETURN_ERROR (errstr, "pcloud", FAILURE);
                }
                status = free_2d_array((void **)tmp_xys);
                if (status != SUCCESS)
                {
                    sprintf (errstr, "Freeing memory: tmp_xys\n");
                    RETURN_ERROR (errstr, "pcloud", FAILURE);
                }
                status = free_2d_array((void **)tmp_xy_type);
                if (status != SUCCESS)
                {
                    sprintf (errstr, "Freeing memory: tmp_xy_type\n");
                    RETURN_ERROR (errstr, "pcloud", FAILURE);
                }
                status = free_2d_array((void **)orin_xys);
                if (status != SUCCESS)
                {
                    sprintf (errstr, "Freeing memory: orin_xys\n");
                    RETURN_ERROR (errstr, "pcloud", FAILURE);
                }
                free(temp_obj);
                temp_obj = NULL;
            }
        }
        free(obj_num);
        obj_num = NULL;
        status = free_2d_array((void **)temp);
        if (status != SUCCESS)
        {
            sprintf (errstr, "Freeing memory: temp\n");
            RETURN_ERROR (errstr, "object_cloud_shadow_match", FAILURE);
        }
        status = free_2d_array((void **)cloud);
        if (status != SUCCESS)
        {
            sprintf (errstr, "Freeing memory: cloud\n");
            RETURN_ERROR (errstr, "object_cloud_shadow_match", FAILURE);
        }
        status = free_2d_array((void **)cloud_first_node);
        if (status != SUCCESS)
        {
            sprintf (errstr, "Freeing memory: cloud_first_node\n");
            RETURN_ERROR (errstr, "object_cloud_shadow_match", FAILURE);
        }

        /* Do image dilate for cloud, shadow, snow */
        image_dilate(cal_mask, nrows, ncols, cldpix, CLOUD_BIT, pixel_mask);
        image_dilate(cal_mask, nrows, ncols, cldpix, SHADOW_BIT, pixel_mask);
    }

    /* Use cal_mask as the output mask, and cal_mask is changed to be a value 
       mask */
    for (row = 0; row < nrows; row++)
    {
        for (col = 0; col < ncols; col++)
        {
            if (pixel_mask[row][col] & (1 << FILL_BIT))
                cal_mask[row][col] = FILL_VALUE;
            else if (pixel_mask[row][col] & (1 << CLOUD_BIT))
            {
                cal_mask[row][col] = 4;
                cloud_count++;
            }
            else if (pixel_mask[row][col] & (1 << SHADOW_BIT))
            {
	        cal_mask[row][col] = 2;
                shadow_count++;
            }
            else if (pixel_mask[row][col] & (1 << SNOW_BIT))
                cal_mask[row][col] = 3;
            else if (pixel_mask[row][col] & (1 <<WATER_BIT))
	        cal_mask[row][col] = 1;
            else
                cal_mask[row][col] = 0;
        }
    }

    /* Copy back to use pixel_mask as final output mask, and it's also
       changed to be a value mask */
    for (row = 0; row < nrows; row++)
    {
        for (col = 0; col < ncols; col++)
        {
            pixel_mask[row][col] = cal_mask[row][col];
        }
    }

    /* Release the memory */
    status = free_2d_array((void **)cal_mask);
    if (status != SUCCESS)
    {
        sprintf (errstr, "Freeing memory: cal_mask\n");
        RETURN_ERROR (errstr, "object_cloud_shadow_match", FAILURE);
    }

    if (verbose)
    {
        printf("cloud_count, shadow_count, boundary_counter = %d,%d,%d\n",
              cloud_count, shadow_count, boundary_counter);

        /* record cloud and cloud shadow percent; */
        cloud_shadow_percent = (float)(cloud_count + shadow_count)
                         / (float)boundary_counter;
        printf("The cloud and shadow percentage is %f\n", cloud_shadow_percent);
    }

    return SUCCESS;
}
