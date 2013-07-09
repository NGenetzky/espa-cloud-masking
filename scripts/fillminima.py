"""
Module to implement filling of local minima in a raster surface. 

Original developer: (from Geoscience Australia)

The algorithm is from 
    Soille, P., and Gratin, C. (1994). An efficient algorithm for drainage network
        extraction on DEMs. J. Visual Communication and Image Representation. 
        5(2). 181-189. 
        
The algorithm is intended for hydrological processing of a DEM, but is used by the 
Fmask cloud shadow algorithm as part of its process for finding local minima which 
represent potential shadow objects. 

This implementation is done with weave, because it was quick to get going, but I 
think I ought to make something which is compiled once, e.g. using f2py, or 
even Cython, as this might be more robust in some situations. 

Song Guo: Modified support C code for checking memory allocation changed
          all exit(1) to exit(EXIT_FAILURE) 
"""
import numpy
from scipy import weave
from scipy.ndimage import grey_erosion, grey_dilation, minimum_filter


def fillMinima(img, nullval, boundaryval):
    """
    Fill all local minima in the input img. The input
    array should be a numpy 2-d array. This function returns
    an array of the same shape and datatype, with the same contents, but
    with local minima filled using the reconstruction-by-erosion algorithm. 
    
    """
    (nrows, ncols) = img.shape
    dtype = img.dtype
    nullmask = (img == nullval)
    nonNullmask = numpy.logical_not(nullmask)
    (hMax, hMin) = (int(img[nonNullmask].max()), int(img[nonNullmask].min()))
    img2 = numpy.zeros((nrows, ncols), dtype=dtype)
    img2.fill(hMax)

    if nullmask.sum() > 0:
        nullmaskDilated = grey_dilation(nullmask, size=(3, 3))
        innerBoundary = nullmaskDilated - nullmask
        (boundaryRows, boundaryCols) = numpy.where(innerBoundary)
    else:
        img2[0, :] = img[0, :]
        img2[-1, :] = img[-1, :]
        img2[:, 0] = img[:, 0]
        img2[:, -1] = img[:, -1]
        (boundaryRows, boundaryCols) = numpy.where(img2!=hMax)
    
    varList = ['img', 'img2', 'hMin', 'hMax', 'nullmask', 'boundaryval',
        'boundaryRows', 'boundaryCols']
    
    weave.inline(mainCcode, arg_names=varList, type_converters=weave.converters.blitz, 
        compiler="gcc", support_code=supportCcode)
    
    img2[nullmask] = nullval
    
    return img2


supportCcode = """
    /* Routines for handling the hierarchical pixel queue which the
       algorithm requires.
    */
    typedef struct PQstruct {
        int i, j;
        struct PQstruct *next;
    } PQel;
    
    typedef struct {
        PQel *first, *last;
        int n;
    } PQhdr;
    
    typedef struct PQ {
        int hMin;
        int numLevels;
        PQhdr *q;
    } PixelQueue;
    
    /* A new pixel structure */
    PQel *newPix(int i, int j) {
        PQel *p;
        
        p = (PQel *)calloc(1, sizeof(PQel));
        if (p == NULL)
        {
            printf("Error allocating memory for p in fillminima.py\n");
            exit(EXIT_FAILURE);
        }
        p->i = i;
        p->j = j;
        p->next = NULL;
        if (i>10000) {
            printf("i=%d\\n", i);
            exit(EXIT_FAILURE);
        }
        return p;
    }
    
    /* Initialize pixel queue */
    PixelQueue *PQ_init(int hMin, int hMax) {
        PixelQueue *pixQ;
        int numLevels, i;
        
        pixQ = (PixelQueue *)calloc(1, sizeof(PixelQueue));
        if (pixQ == NULL)
        {
            printf("Error allocating memory for pixQ in fillminima.py\n");
            exit(EXIT_FAILURE);
        }
        numLevels = hMax - hMin + 1;
        pixQ->hMin = hMin;
        pixQ->numLevels = numLevels;
        pixQ->q = (PQhdr *)calloc(numLevels, sizeof(PQhdr));
        if (pixQ->q == NULL)
        {
            printf("Error allocating memory for pixQ->q in fillminima.py\n");
            exit(EXIT_FAILURE);
        }
        for (i=0; i<numLevels; i++) {
            pixQ->q[i].first = NULL;
            pixQ->q[i].last = NULL;
            pixQ->q[i].n = 0;
        }
        return pixQ;
    }
    
    /* Add a pixel at level h */
    void PQ_add(PixelQueue *pixQ, PQel *p, int h) {
        int ndx;
        PQel *current, *newP;
        PQhdr *thisQ;
        
        /* Take a copy of the pixel structure */
        newP = newPix(p->i, p->j);
        
        ndx = h - pixQ->hMin;
        if (ndx > pixQ->numLevels) {
            printf("Level h=%d too large. ndx=%d, numLevels=%d\\n", h, ndx, pixQ->numLevels);
            exit(EXIT_FAILURE);
        }
        thisQ = &(pixQ->q[ndx]);
        /* Add to end of queue at this level */
        current = thisQ->last;
        if (current != NULL) {
            current->next = newP;
        }
        thisQ->last = newP;
        thisQ->n++;
        /* If head of queue is NULL, make the new one the head */
        if (thisQ->first == NULL) {
            thisQ->first = newP;
        }
    }
    
    /* Return TRUE if queue at level h is empty */
    int PQ_empty(PixelQueue *pixQ, int h) {
        int ndx, empty, n;
        PQel *current;
        
        ndx = h - pixQ->hMin;
        current = pixQ->q[ndx].first;
        n = pixQ->q[ndx].n;
        empty = (current == NULL);
        if (empty && (n != 0)) {
            printf("Empty, but n=%d\\n", n);
            exit(EXIT_FAILURE);
        }
        if ((n == 0) && (! empty)) {
            printf("n=0, but not empty\\n");
            while (current != NULL) {
                printf("    h=%d i=%d j=%d\\n", h, current->i, current->j);
                current = current->next;
            }
            exit(EXIT_FAILURE);
        }
        return empty;
    }
    
    /* Return the first element in the queue at level h, and remove it
       from the queue */
    PQel *PQ_first(PixelQueue *pixQ, int h) {
        int ndx;
        PQel *current;
        PQhdr *thisQ;
        
        ndx = h - pixQ->hMin;
        thisQ = &(pixQ->q[ndx]);
        current = thisQ->first;
        /* Remove from head of queue */
        if (current != NULL) {
            thisQ->first = current->next;
            if (thisQ->first == NULL) {
                thisQ->last = NULL;
            }
            thisQ->n--;
            if (thisQ->n < 0) {
                printf("n=%d in PQ_first()\\n", thisQ->n);
                exit(EXIT_FAILURE);
            } else if (thisQ->n == 0) {
                if (thisQ->first != NULL) {
                    printf("n=0, but 'first' != NULL. first(i,j) = %d,%d\\n", 
                        thisQ->first->i, thisQ->first->j);
                }
            }
        }
        return current;
    }
    
    /* Return a list of neighbouring pixels to given pixel p.  */
    PQel *neighbours(PQel *p, int nRows, int nCols) {
        int ii, jj, i, j;
        PQel *pl, *pNew;
        
        pl = NULL;
        for (ii=-1; ii<=1; ii++) {
            for (jj=-1; jj<=1; jj++) {
                if ((ii != 0) && (jj != 0)) {
                    i = p->i + ii;
                    j = p->j + jj;
                    if ((i >= 0) && (i < nRows) && (j >= 0) && (j < nCols)) {
                        pNew = newPix(i, j);
                        pNew->next = pl;
                        pl = pNew;
                    }
                }
            }
        }
        return pl;
    }
"""

mainCcode = """
    int i, r, c, imgval, img2val, h, nRows, nCols;
    PixelQueue *pixQ;
    PQel *p, *nbrs, *pNbr, *pNext;
    int hCrt;
    
    #define max(a,b) ((a) > (b) ? (a) : (b))
    
    nRows = Nimg[0];
    nCols = Nimg[1];
    
    pixQ = PQ_init(hMin, hMax);
    
    /* Initialize the boundary */
    for (i=0; i<NboundaryRows[0]; i++) {
        r = boundaryRows(i);
        c = boundaryCols(i);
        //img2(r, c) = img(r, c);
        img2(r, c) = boundaryval;
        
        p = newPix(r, c);
        h = img(r, c);
        //PQ_add(pixQ, p, img(r, c));
        PQ_add(pixQ, p, boundaryval);
    }
    
    
    /* Process until stability */
    hCrt = (int)hMin;
    do {
        while (! PQ_empty(pixQ, hCrt)) {
            p = PQ_first(pixQ, hCrt);
            nbrs = neighbours(p, nRows, nCols);
            pNbr = nbrs;
            while (pNbr != NULL) {
                r = pNbr->i;
                c = pNbr->j;
                /* Exclude null area of original image */
                if (! nullmask(r, c)) {
                    imgval = img(r, c);
                    img2val = img2(r, c);
                    if (img2val == hMax) {
                        img2(r, c) = max(hCrt, imgval);
                        PQ_add(pixQ, pNbr, img2(r, c));
                    }
                }
                pNext = pNbr->next;
                free(pNbr);
                pNbr = pNext;
            }
            free(p);
        }
        hCrt++;
    } while (hCrt < hMax);
    
    free(pixQ);
"""
