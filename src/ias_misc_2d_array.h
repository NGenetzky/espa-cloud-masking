#ifndef IAS_MISC_2D_ARRAY_H
#define IAS_MISC_2D_ARRAY_H

#include <stdio.h>          /* FILE definition */
#include "mystring.h"
#include "ias_types.h"

/* The following routines add support for 2D arrays that can be dynamically
   allocated.  The arrays are created by allocating a 1D block of memory for
   the data storage and a 1D array of void pointers that point to the first
   member in each row of data in the 2D array.  The void** returned by the
   allocate_2d_array routine is a pointer to the array of void pointers.

   Example use:
        A 2D array of doubles would be allocated and used as follows:

        int rows = calculate number of rows...
        int columns = calculate number of columns...
        double **array = ias_misc_allocate_2d_array(rows, columns, 
                                sizeof(double));
        double value = array[2][3];
        ias_misc_free_2d_array((void **)array);

    Notes:
        - Since the array contents require two memory references to access the
          data, these 2D arrays should not be used in performance critical
          code.  Instead, 1D arrays should be used and the indices calculated
          manually.  For example, it would be best to not use this 2D array
          implementation for imagery.
        - Arrays allocated using this library need to be freed using this
          library due to how it manages the memory.  The library hides extra
          information about the array that can only be accessed by the 
          ias_misc_free_2d_array and ias_misc_get_2d_array_size routines.
        - It would be possible to add a routine to allow access to the 1D array
          of data that underlies the 2D pointer array and allow manually
          calculating the indices into the array for performance critical use
          of the library.
*/

void **ias_misc_allocate_2d_array
(
    int rows,            /* I: Number of rows for the 2D array */
    int columns,         /* I: Number of columns for the 2D array */
    size_t member_size   /* I: Size of the 2D array element */
);

int ias_misc_get_2d_array_size
(
    void **array_ptr,   /* I: Pointer returned by the alloc routine */
    int *rows,          /* O: Pointer to number of rows */
    int *columns        /* O: Pointer to number of columns */
);

int ias_misc_free_2d_array
(
    void **array_ptr    /* I: Pointer returned by the alloc routine */
);

#endif
