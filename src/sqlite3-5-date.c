/************** Begin file status.c ******************************************/
/*
** 2008 June 18
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This module implements the sqlite3_status() interface and related
** functionality.
*/
/* #include "sqliteInt.h" */
/************** Include vdbeInt.h in the middle of status.c ******************/
/************** Begin file vdbeInt.h *****************************************/
/*
** 2003 September 6
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This is the header file for information that is private to the
** VDBE.  This information used to all be at the top of the single
** source code file "vdbe.c".  When that file became too big (over
** 6000 lines long) it was split up into several smaller files and
** this header information was factored out.
*/
#ifndef SQLITE_VDBEINT_H
#define SQLITE_VDBEINT_H

/*
** The maximum number of times that a statement will try to reparse
** itself before giving up and returning SQLITE_SCHEMA.
*/
#ifndef SQLITE_MAX_SCHEMA_RETRY
# define SQLITE_MAX_SCHEMA_RETRY 50
#endif

/*
** VDBE_DISPLAY_P4 is true or false depending on whether or not the
** "explain" P4 display logic is enabled.
*/
#if !defined(SQLITE_OMIT_EXPLAIN) || !defined(NDEBUG) \
     || defined(VDBE_PROFILE) || defined(SQLITE_DEBUG)
# define VDBE_DISPLAY_P4 1
#else
# define VDBE_DISPLAY_P4 0
#endif

/*
** SQL is translated into a sequence of instructions to be
** executed by a virtual machine.  Each instruction is an instance
** of the following structure.
*/
typedef struct VdbeOp Op;

/*
** Boolean values
*/
typedef unsigned Bool;

/* Opaque type used by code in vdbesort.c */
typedef struct VdbeSorter VdbeSorter;

/* Elements of the linked list at Vdbe.pAuxData */
typedef struct AuxData AuxData;

/* Types of VDBE cursors */
#define CURTYPE_BTREE       0
#define CURTYPE_SORTER      1
#define CURTYPE_VTAB        2
#define CURTYPE_PSEUDO      3

/*
** A VdbeCursor is an superclass (a wrapper) for various cursor objects:
**
**      * A b-tree cursor
**          -  In the main database or in an ephemeral database
**          -  On either an index or a table
**      * A sorter
**      * A virtual table
**      * A one-row "pseudotable" stored in a single register
*/
typedef struct VdbeCursor VdbeCursor;
struct VdbeCursor {
  u8 eCurType;            /* One of the CURTYPE_* values above */
  i8 iDb;                 /* Index of cursor database in db->aDb[] (or -1) */
  u8 nullRow;             /* True if pointing to a row with no data */
  u8 deferredMoveto;      /* A call to sqlite3BtreeMoveto() is needed */
  u8 isTable;             /* True for rowid tables.  False for indexes */
#ifdef SQLITE_DEBUG
  u8 seekOp;              /* Most recent seek operation on this cursor */
  u8 wrFlag;              /* The wrFlag argument to sqlite3BtreeCursor() */
#endif
  Bool isEphemeral:1;     /* True for an ephemeral table */
  Bool useRandomRowid:1;  /* Generate new record numbers semi-randomly */
  Bool isOrdered:1;       /* True if the table is not BTREE_UNORDERED */
  Btree *pBtx;            /* Separate file holding temporary table */
  i64 seqCount;           /* Sequence counter */
  int *aAltMap;           /* Mapping from table to index column numbers */

  /* Cached OP_Column parse information is only valid if cacheStatus matches
  ** Vdbe.cacheCtr.  Vdbe.cacheCtr will never take on the value of
  ** CACHE_STALE (0) and so setting cacheStatus=CACHE_STALE guarantees that
  ** the cache is out of date. */
  u32 cacheStatus;        /* Cache is valid if this matches Vdbe.cacheCtr */
  int seekResult;         /* Result of previous sqlite3BtreeMoveto() or 0
                          ** if there have been no prior seeks on the cursor. */
  /* NB: seekResult does not distinguish between "no seeks have ever occurred
  ** on this cursor" and "the most recent seek was an exact match". */

  /* When a new VdbeCursor is allocated, only the fields above are zeroed.
  ** The fields that follow are uninitialized, and must be individually
  ** initialized prior to first use. */
  VdbeCursor *pAltCursor; /* Associated index cursor from which to read */
  union {
    BtCursor *pCursor;          /* CURTYPE_BTREE.  Btree cursor */
    sqlite3_vtab_cursor *pVCur; /* CURTYPE_VTAB.   Vtab cursor */
    int pseudoTableReg;         /* CURTYPE_PSEUDO. Reg holding content. */
    VdbeSorter *pSorter;        /* CURTYPE_SORTER. Sorter object */
  } uc;
  KeyInfo *pKeyInfo;      /* Info about index keys needed by index cursors */
  u32 iHdrOffset;         /* Offset to next unparsed byte of the header */
  Pgno pgnoRoot;          /* Root page of the open btree cursor */
  i16 nField;             /* Number of fields in the header */
  u16 nHdrParsed;         /* Number of header fields parsed so far */
  i64 movetoTarget;       /* Argument to the deferred sqlite3BtreeMoveto() */
  u32 *aOffset;           /* Pointer to aType[nField] */
  const u8 *aRow;         /* Data for the current row, if all on one page */
  u32 payloadSize;        /* Total number of bytes in the record */
  u32 szRow;              /* Byte available in aRow */
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
  u64 maskUsed;           /* Mask of columns used by this cursor */
#endif

  /* 2*nField extra array elements allocated for aType[], beyond the one
  ** static element declared in the structure.  nField total array slots for
  ** aType[] and nField+1 array slots for aOffset[] */
  u32 aType[1];           /* Type values record decode.  MUST BE LAST */
};


/*
** A value for VdbeCursor.cacheStatus that means the cache is always invalid.
*/
#define CACHE_STALE 0

/*
** When a sub-program is executed (OP_Program), a structure of this type
** is allocated to store the current value of the program counter, as
** well as the current memory cell array and various other frame specific
** values stored in the Vdbe struct. When the sub-program is finished, 
** these values are copied back to the Vdbe from the VdbeFrame structure,
** restoring the state of the VM to as it was before the sub-program
** began executing.
**
** The memory for a VdbeFrame object is allocated and managed by a memory
** cell in the parent (calling) frame. When the memory cell is deleted or
** overwritten, the VdbeFrame object is not freed immediately. Instead, it
** is linked into the Vdbe.pDelFrame list. The contents of the Vdbe.pDelFrame
** list is deleted when the VM is reset in VdbeHalt(). The reason for doing
** this instead of deleting the VdbeFrame immediately is to avoid recursive
** calls to sqlite3VdbeMemRelease() when the memory cells belonging to the
** child frame are released.
**
** The currently executing frame is stored in Vdbe.pFrame. Vdbe.pFrame is
** set to NULL if the currently executing frame is the main program.
*/
typedef struct VdbeFrame VdbeFrame;
struct VdbeFrame {
  Vdbe *v;                /* VM this frame belongs to */
  VdbeFrame *pParent;     /* Parent of this frame, or NULL if parent is main */
  Op *aOp;                /* Program instructions for parent frame */
  i64 *anExec;            /* Event counters from parent frame */
  Mem *aMem;              /* Array of memory cells for parent frame */
  VdbeCursor **apCsr;     /* Array of Vdbe cursors for parent frame */
  void *token;            /* Copy of SubProgram.token */
  i64 lastRowid;          /* Last insert rowid (sqlite3.lastRowid) */
  AuxData *pAuxData;      /* Linked list of auxdata allocations */
  int nCursor;            /* Number of entries in apCsr */
  int pc;                 /* Program Counter in parent (calling) frame */
  int nOp;                /* Size of aOp array */
  int nMem;               /* Number of entries in aMem */
  int nChildMem;          /* Number of memory cells for child frame */
  int nChildCsr;          /* Number of cursors for child frame */
  int nChange;            /* Statement changes (Vdbe.nChange)     */
  int nDbChange;          /* Value of db->nChange */
};

#define VdbeFrameMem(p) ((Mem *)&((u8 *)p)[ROUND8(sizeof(VdbeFrame))])

/*
** Internally, the vdbe manipulates nearly all SQL values as Mem
** structures. Each Mem struct may cache multiple representations (string,
** integer etc.) of the same value.
*/
struct Mem {
  union MemValue {
    double r;           /* Real value used when MEM_Real is set in flags */
    i64 i;              /* Integer value used when MEM_Int is set in flags */
    int nZero;          /* Used when bit MEM_Zero is set in flags */
    FuncDef *pDef;      /* Used only when flags==MEM_Agg */
    RowSet *pRowSet;    /* Used only when flags==MEM_RowSet */
    VdbeFrame *pFrame;  /* Used when flags==MEM_Frame */
  } u;
  u16 flags;          /* Some combination of MEM_Null, MEM_Str, MEM_Dyn, etc. */
  u8  enc;            /* SQLITE_UTF8, SQLITE_UTF16BE, SQLITE_UTF16LE */
  u8  eSubtype;       /* Subtype for this value */
  int n;              /* Number of characters in string value, excluding '\0' */
  char *z;            /* String or BLOB value */
  /* ShallowCopy only needs to copy the information above */
  char *zMalloc;      /* Space to hold MEM_Str or MEM_Blob if szMalloc>0 */
  int szMalloc;       /* Size of the zMalloc allocation */
  u32 uTemp;          /* Transient storage for serial_type in OP_MakeRecord */
  sqlite3 *db;        /* The associated database connection */
  void (*xDel)(void*);/* Destructor for Mem.z - only valid if MEM_Dyn */
#ifdef SQLITE_DEBUG
  Mem *pScopyFrom;    /* This Mem is a shallow copy of pScopyFrom */
  void *pFiller;      /* So that sizeof(Mem) is a multiple of 8 */
#endif
};

/*
** Size of struct Mem not including the Mem.zMalloc member or anything that
** follows.
*/
#define MEMCELLSIZE offsetof(Mem,zMalloc)

/* One or more of the following flags are set to indicate the validOK
** representations of the value stored in the Mem struct.
**
** If the MEM_Null flag is set, then the value is an SQL NULL value.
** No other flags may be set in this case.
**
** If the MEM_Str flag is set then Mem.z points at a string representation.
** Usually this is encoded in the same unicode encoding as the main
** database (see below for exceptions). If the MEM_Term flag is also
** set, then the string is nul terminated. The MEM_Int and MEM_Real 
** flags may coexist with the MEM_Str flag.
*/
#define MEM_Null      0x0001   /* Value is NULL */
#define MEM_Str       0x0002   /* Value is a string */
#define MEM_Int       0x0004   /* Value is an integer */
#define MEM_Real      0x0008   /* Value is a real number */
#define MEM_Blob      0x0010   /* Value is a BLOB */
#define MEM_AffMask   0x001f   /* Mask of affinity bits */
#define MEM_RowSet    0x0020   /* Value is a RowSet object */
#define MEM_Frame     0x0040   /* Value is a VdbeFrame object */
#define MEM_Undefined 0x0080   /* Value is undefined */
#define MEM_Cleared   0x0100   /* NULL set by OP_Null, not from data */
#define MEM_TypeMask  0x81ff   /* Mask of type bits */


/* Whenever Mem contains a valid string or blob representation, one of
** the following flags must be set to determine the memory management
** policy for Mem.z.  The MEM_Term flag tells us whether or not the
** string is \000 or \u0000 terminated
*/
#define MEM_Term      0x0200   /* String rep is nul terminated */
#define MEM_Dyn       0x0400   /* Need to call Mem.xDel() on Mem.z */
#define MEM_Static    0x0800   /* Mem.z points to a static string */
#define MEM_Ephem     0x1000   /* Mem.z points to an ephemeral string */
#define MEM_Agg       0x2000   /* Mem.z points to an agg function context */
#define MEM_Zero      0x4000   /* Mem.i contains count of 0s appended to blob */
#define MEM_Subtype   0x8000   /* Mem.eSubtype is valid */
#ifdef SQLITE_OMIT_INCRBLOB
  #undef MEM_Zero
  #define MEM_Zero 0x0000
#endif

/* Return TRUE if Mem X contains dynamically allocated content - anything
** that needs to be deallocated to avoid a leak.
*/
#define VdbeMemDynamic(X)  \
  (((X)->flags&(MEM_Agg|MEM_Dyn|MEM_RowSet|MEM_Frame))!=0)

/*
** Clear any existing type flags from a Mem and replace them with f
*/
#define MemSetTypeFlag(p, f) \
   ((p)->flags = ((p)->flags&~(MEM_TypeMask|MEM_Zero))|f)

/*
** Return true if a memory cell is not marked as invalid.  This macro
** is for use inside assert() statements only.
*/
#ifdef SQLITE_DEBUG
#define memIsValid(M)  ((M)->flags & MEM_Undefined)==0
#endif

/*
** Each auxiliary data pointer stored by a user defined function 
** implementation calling sqlite3_set_auxdata() is stored in an instance
** of this structure. All such structures associated with a single VM
** are stored in a linked list headed at Vdbe.pAuxData. All are destroyed
** when the VM is halted (if not before).
*/
struct AuxData {
  int iOp;                        /* Instruction number of OP_Function opcode */
  int iArg;                       /* Index of function argument. */
  void *pAux;                     /* Aux data pointer */
  void (*xDelete)(void *);        /* Destructor for the aux data */
  AuxData *pNext;                 /* Next element in list */
};

/*
** The "context" argument for an installable function.  A pointer to an
** instance of this structure is the first argument to the routines used
** implement the SQL functions.
**
** There is a typedef for this structure in sqlite.h.  So all routines,
** even the public interface to SQLite, can use a pointer to this structure.
** But this file is the only place where the internal details of this
** structure are known.
**
** This structure is defined inside of vdbeInt.h because it uses substructures
** (Mem) which are only defined there.
*/
struct sqlite3_context {
  Mem *pOut;              /* The return value is stored here */
  FuncDef *pFunc;         /* Pointer to function information */
  Mem *pMem;              /* Memory cell used to store aggregate context */
  Vdbe *pVdbe;            /* The VM that owns this context */
  int iOp;                /* Instruction number of OP_Function */
  int isError;            /* Error code returned by the function. */
  u8 skipFlag;            /* Skip accumulator loading if true */
  u8 fErrorOrAux;         /* isError!=0 or pVdbe->pAuxData modified */
  u8 argc;                /* Number of arguments */
  sqlite3_value *argv[1]; /* Argument set */
};

/* A bitfield type for use inside of structures.  Always follow with :N where
** N is the number of bits.
*/
typedef unsigned bft;  /* Bit Field Type */

typedef struct ScanStatus ScanStatus;
struct ScanStatus {
  int addrExplain;                /* OP_Explain for loop */
  int addrLoop;                   /* Address of "loops" counter */
  int addrVisit;                  /* Address of "rows visited" counter */
  int iSelectID;                  /* The "Select-ID" for this loop */
  LogEst nEst;                    /* Estimated output rows per loop */
  char *zName;                    /* Name of table or index */
};

/*
** An instance of the virtual machine.  This structure contains the complete
** state of the virtual machine.
**
** The "sqlite3_stmt" structure pointer that is returned by sqlite3_prepare()
** is really a pointer to an instance of this structure.
*/
struct Vdbe {
  sqlite3 *db;            /* The database connection that owns this statement */
  Vdbe *pPrev,*pNext;     /* Linked list of VDBEs with the same Vdbe.db */
  Parse *pParse;          /* Parsing context used to create this Vdbe */
  ynVar nVar;             /* Number of entries in aVar[] */
  u32 magic;              /* Magic number for sanity checking */
  int nMem;               /* Number of memory locations currently allocated */
  int nCursor;            /* Number of slots in apCsr[] */
  u32 cacheCtr;           /* VdbeCursor row cache generation counter */
  int pc;                 /* The program counter */
  int rc;                 /* Value to return */
  int nChange;            /* Number of db changes made since last reset */
  int iStatement;         /* Statement number (or 0 if has not opened stmt) */
  i64 iCurrentTime;       /* Value of julianday('now') for this statement */
  i64 nFkConstraint;      /* Number of imm. FK constraints this VM */
  i64 nStmtDefCons;       /* Number of def. constraints when stmt started */
  i64 nStmtDefImmCons;    /* Number of def. imm constraints when stmt started */

  /* When allocating a new Vdbe object, all of the fields below should be
  ** initialized to zero or NULL */

  Op *aOp;                /* Space to hold the virtual machine's program */
  Mem *aMem;              /* The memory locations */
  Mem **apArg;            /* Arguments to currently executing user function */
  Mem *aColName;          /* Column names to return */
  Mem *pResultSet;        /* Pointer to an array of results */
  char *zErrMsg;          /* Error message written here */
  VdbeCursor **apCsr;     /* One element of this array for each open cursor */
  Mem *aVar;              /* Values for the OP_Variable opcode. */
  VList *pVList;          /* Name of variables */
#ifndef SQLITE_OMIT_TRACE
  i64 startTime;          /* Time when query started - used for profiling */
#endif
  int nOp;                /* Number of instructions in the program */
#ifdef SQLITE_DEBUG
  int rcApp;              /* errcode set by sqlite3_result_error_code() */
#endif
  u16 nResColumn;         /* Number of columns in one row of the result set */
  u8 errorAction;         /* Recovery action to do in case of an error */
  u8 minWriteFileFormat;  /* Minimum file format for writable database files */
  bft expired:1;          /* True if the VM needs to be recompiled */
  bft doingRerun:1;       /* True if rerunning after an auto-reprepare */
  bft explain:2;          /* True if EXPLAIN present on SQL command */
  bft changeCntOn:1;      /* True to update the change-counter */
  bft runOnlyOnce:1;      /* Automatically expire on reset */
  bft usesStmtJournal:1;  /* True if uses a statement journal */
  bft readOnly:1;         /* True for statements that do not write */
  bft bIsReader:1;        /* True for statements that read */
  bft isPrepareV2:1;      /* True if prepared with prepare_v2() */
  yDbMask btreeMask;      /* Bitmask of db->aDb[] entries referenced */
  yDbMask lockMask;       /* Subset of btreeMask that requires a lock */
  u32 aCounter[5];        /* Counters used by sqlite3_stmt_status() */
  char *zSql;             /* Text of the SQL statement that generated this */
  void *pFree;            /* Free this when deleting the vdbe */
  VdbeFrame *pFrame;      /* Parent frame */
  VdbeFrame *pDelFrame;   /* List of frame objects to free on VM reset */
  int nFrame;             /* Number of frames in pFrame list */
  u32 expmask;            /* Binding to these vars invalidates VM */
  SubProgram *pProgram;   /* Linked list of all sub-programs used by VM */
  AuxData *pAuxData;      /* Linked list of auxdata allocations */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  i64 *anExec;            /* Number of times each op has been executed */
  int nScan;              /* Entries in aScan[] */
  ScanStatus *aScan;      /* Scan definitions for sqlite3_stmt_scanstatus() */
#endif
};

/*
** The following are allowed values for Vdbe.magic
*/
#define VDBE_MAGIC_INIT     0x16bceaa5    /* Building a VDBE program */
#define VDBE_MAGIC_RUN      0x2df20da3    /* VDBE is ready to execute */
#define VDBE_MAGIC_HALT     0x319c2973    /* VDBE has completed execution */
#define VDBE_MAGIC_RESET    0x48fa9f76    /* Reset and ready to run again */
#define VDBE_MAGIC_DEAD     0x5606c3c8    /* The VDBE has been deallocated */

/*
** Structure used to store the context required by the 
** sqlite3_preupdate_*() API functions.
*/
struct PreUpdate {
  Vdbe *v;
  VdbeCursor *pCsr;               /* Cursor to read old values from */
  int op;                         /* One of SQLITE_INSERT, UPDATE, DELETE */
  u8 *aRecord;                    /* old.* database record */
  KeyInfo keyinfo;
  UnpackedRecord *pUnpacked;      /* Unpacked version of aRecord[] */
  UnpackedRecord *pNewUnpacked;   /* Unpacked version of new.* record */
  int iNewReg;                    /* Register for new.* values */
  i64 iKey1;                      /* First key value passed to hook */
  i64 iKey2;                      /* Second key value passed to hook */
  Mem *aNew;                      /* Array of new.* values */
  Table *pTab;                    /* Schema object being upated */          
};

/*
** Function prototypes
*/
SQLITE_PRIVATE void sqlite3VdbeError(Vdbe*, const char *, ...);
SQLITE_PRIVATE void sqlite3VdbeFreeCursor(Vdbe *, VdbeCursor*);
void sqliteVdbePopStack(Vdbe*,int);
SQLITE_PRIVATE int sqlite3VdbeCursorMoveto(VdbeCursor**, int*);
SQLITE_PRIVATE int sqlite3VdbeCursorRestore(VdbeCursor*);
#if defined(SQLITE_DEBUG) || defined(VDBE_PROFILE)
SQLITE_PRIVATE void sqlite3VdbePrintOp(FILE*, int, Op*);
#endif
SQLITE_PRIVATE u32 sqlite3VdbeSerialTypeLen(u32);
SQLITE_PRIVATE u8 sqlite3VdbeOneByteSerialTypeLen(u8);
SQLITE_PRIVATE u32 sqlite3VdbeSerialType(Mem*, int, u32*);
SQLITE_PRIVATE u32 sqlite3VdbeSerialPut(unsigned char*, Mem*, u32);
SQLITE_PRIVATE u32 sqlite3VdbeSerialGet(const unsigned char*, u32, Mem*);
SQLITE_PRIVATE void sqlite3VdbeDeleteAuxData(sqlite3*, AuxData**, int, int);

int sqlite2BtreeKeyCompare(BtCursor *, const void *, int, int, int *);
SQLITE_PRIVATE int sqlite3VdbeIdxKeyCompare(sqlite3*,VdbeCursor*,UnpackedRecord*,int*);
SQLITE_PRIVATE int sqlite3VdbeIdxRowid(sqlite3*, BtCursor*, i64*);
SQLITE_PRIVATE int sqlite3VdbeExec(Vdbe*);
SQLITE_PRIVATE int sqlite3VdbeList(Vdbe*);
SQLITE_PRIVATE int sqlite3VdbeHalt(Vdbe*);
SQLITE_PRIVATE int sqlite3VdbeChangeEncoding(Mem *, int);
SQLITE_PRIVATE int sqlite3VdbeMemTooBig(Mem*);
SQLITE_PRIVATE int sqlite3VdbeMemCopy(Mem*, const Mem*);
SQLITE_PRIVATE void sqlite3VdbeMemShallowCopy(Mem*, const Mem*, int);
SQLITE_PRIVATE void sqlite3VdbeMemMove(Mem*, Mem*);
SQLITE_PRIVATE int sqlite3VdbeMemNulTerminate(Mem*);
SQLITE_PRIVATE int sqlite3VdbeMemSetStr(Mem*, const char*, int, u8, void(*)(void*));
SQLITE_PRIVATE void sqlite3VdbeMemSetInt64(Mem*, i64);
#ifdef SQLITE_OMIT_FLOATING_POINT
# define sqlite3VdbeMemSetDouble sqlite3VdbeMemSetInt64
#else
SQLITE_PRIVATE   void sqlite3VdbeMemSetDouble(Mem*, double);
#endif
SQLITE_PRIVATE void sqlite3VdbeMemInit(Mem*,sqlite3*,u16);
SQLITE_PRIVATE void sqlite3VdbeMemSetNull(Mem*);
SQLITE_PRIVATE void sqlite3VdbeMemSetZeroBlob(Mem*,int);
SQLITE_PRIVATE void sqlite3VdbeMemSetRowSet(Mem*);
SQLITE_PRIVATE int sqlite3VdbeMemMakeWriteable(Mem*);
SQLITE_PRIVATE int sqlite3VdbeMemStringify(Mem*, u8, u8);
SQLITE_PRIVATE i64 sqlite3VdbeIntValue(Mem*);
SQLITE_PRIVATE int sqlite3VdbeMemIntegerify(Mem*);
SQLITE_PRIVATE double sqlite3VdbeRealValue(Mem*);
SQLITE_PRIVATE void sqlite3VdbeIntegerAffinity(Mem*);
SQLITE_PRIVATE int sqlite3VdbeMemRealify(Mem*);
SQLITE_PRIVATE int sqlite3VdbeMemNumerify(Mem*);
SQLITE_PRIVATE void sqlite3VdbeMemCast(Mem*,u8,u8);
SQLITE_PRIVATE int sqlite3VdbeMemFromBtree(BtCursor*,u32,u32,Mem*);
SQLITE_PRIVATE void sqlite3VdbeMemRelease(Mem *p);
SQLITE_PRIVATE int sqlite3VdbeMemFinalize(Mem*, FuncDef*);
SQLITE_PRIVATE const char *sqlite3OpcodeName(int);
SQLITE_PRIVATE int sqlite3VdbeMemGrow(Mem *pMem, int n, int preserve);
SQLITE_PRIVATE int sqlite3VdbeMemClearAndResize(Mem *pMem, int n);
SQLITE_PRIVATE int sqlite3VdbeCloseStatement(Vdbe *, int);
SQLITE_PRIVATE void sqlite3VdbeFrameDelete(VdbeFrame*);
SQLITE_PRIVATE int sqlite3VdbeFrameRestore(VdbeFrame *);
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
SQLITE_PRIVATE void sqlite3VdbePreUpdateHook(Vdbe*,VdbeCursor*,int,const char*,Table*,i64,int);
#endif
SQLITE_PRIVATE int sqlite3VdbeTransferError(Vdbe *p);

SQLITE_PRIVATE int sqlite3VdbeSorterInit(sqlite3 *, int, VdbeCursor *);
SQLITE_PRIVATE void sqlite3VdbeSorterReset(sqlite3 *, VdbeSorter *);
SQLITE_PRIVATE void sqlite3VdbeSorterClose(sqlite3 *, VdbeCursor *);
SQLITE_PRIVATE int sqlite3VdbeSorterRowkey(const VdbeCursor *, Mem *);
SQLITE_PRIVATE int sqlite3VdbeSorterNext(sqlite3 *, const VdbeCursor *, int *);
SQLITE_PRIVATE int sqlite3VdbeSorterRewind(const VdbeCursor *, int *);
SQLITE_PRIVATE int sqlite3VdbeSorterWrite(const VdbeCursor *, Mem *);
SQLITE_PRIVATE int sqlite3VdbeSorterCompare(const VdbeCursor *, Mem *, int, int *);

#if !defined(SQLITE_OMIT_SHARED_CACHE) 
SQLITE_PRIVATE   void sqlite3VdbeEnter(Vdbe*);
#else
# define sqlite3VdbeEnter(X)
#endif

#if !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
SQLITE_PRIVATE   void sqlite3VdbeLeave(Vdbe*);
#else
# define sqlite3VdbeLeave(X)
#endif

#ifdef SQLITE_DEBUG
SQLITE_PRIVATE void sqlite3VdbeMemAboutToChange(Vdbe*,Mem*);
SQLITE_PRIVATE int sqlite3VdbeCheckMemInvariants(Mem*);
#endif

#ifndef SQLITE_OMIT_FOREIGN_KEY
SQLITE_PRIVATE int sqlite3VdbeCheckFk(Vdbe *, int);
#else
# define sqlite3VdbeCheckFk(p,i) 0
#endif

SQLITE_PRIVATE int sqlite3VdbeMemTranslate(Mem*, u8);
#ifdef SQLITE_DEBUG
SQLITE_PRIVATE   void sqlite3VdbePrintSql(Vdbe*);
SQLITE_PRIVATE   void sqlite3VdbeMemPrettyPrint(Mem *pMem, char *zBuf);
#endif
SQLITE_PRIVATE int sqlite3VdbeMemHandleBom(Mem *pMem);

#ifndef SQLITE_OMIT_INCRBLOB
SQLITE_PRIVATE   int sqlite3VdbeMemExpandBlob(Mem *);
  #define ExpandBlob(P) (((P)->flags&MEM_Zero)?sqlite3VdbeMemExpandBlob(P):0)
#else
  #define sqlite3VdbeMemExpandBlob(x) SQLITE_OK
  #define ExpandBlob(P) SQLITE_OK
#endif

#endif /* !defined(SQLITE_VDBEINT_H) */

/************** End of vdbeInt.h *********************************************/
/************** Continuing where we left off in status.c *********************/

/*
** Variables in which to record status information.
*/
#if SQLITE_PTRSIZE>4
typedef sqlite3_int64 sqlite3StatValueType;
#else
typedef u32 sqlite3StatValueType;
#endif
typedef struct sqlite3StatType sqlite3StatType;
static SQLITE_WSD struct sqlite3StatType {
  sqlite3StatValueType nowValue[10];  /* Current value */
  sqlite3StatValueType mxValue[10];   /* Maximum value */
} sqlite3Stat = { {0,}, {0,} };

/*
** Elements of sqlite3Stat[] are protected by either the memory allocator
** mutex, or by the pcache1 mutex.  The following array determines which.
*/
static const char statMutex[] = {
  0,  /* SQLITE_STATUS_MEMORY_USED */
  1,  /* SQLITE_STATUS_PAGECACHE_USED */
  1,  /* SQLITE_STATUS_PAGECACHE_OVERFLOW */
  0,  /* SQLITE_STATUS_SCRATCH_USED */
  0,  /* SQLITE_STATUS_SCRATCH_OVERFLOW */
  0,  /* SQLITE_STATUS_MALLOC_SIZE */
  0,  /* SQLITE_STATUS_PARSER_STACK */
  1,  /* SQLITE_STATUS_PAGECACHE_SIZE */
  0,  /* SQLITE_STATUS_SCRATCH_SIZE */
  0,  /* SQLITE_STATUS_MALLOC_COUNT */
};


/* The "wsdStat" macro will resolve to the status information
** state vector.  If writable static data is unsupported on the target,
** we have to locate the state vector at run-time.  In the more common
** case where writable static data is supported, wsdStat can refer directly
** to the "sqlite3Stat" state vector declared above.
*/
#ifdef SQLITE_OMIT_WSD
# define wsdStatInit  sqlite3StatType *x = &GLOBAL(sqlite3StatType,sqlite3Stat)
# define wsdStat x[0]
#else
# define wsdStatInit
# define wsdStat sqlite3Stat
#endif

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
SQLITE_PRIVATE sqlite3_int64 sqlite3StatusValue(int op){
  wsdStatInit;
  assert( op>=0 && op<ArraySize(wsdStat.nowValue) );
  assert( op>=0 && op<ArraySize(statMutex) );
  assert( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  return wsdStat.nowValue[op];
}

/*
** Add N to the value of a status record.  The caller must hold the
** appropriate mutex.  (Locking is checked by assert()).
**
** The StatusUp() routine can accept positive or negative values for N.
** The value of N is added to the current status value and the high-water
** mark is adjusted if necessary.
**
** The StatusDown() routine lowers the current value by N.  The highwater
** mark is unchanged.  N must be non-negative for StatusDown().
*/
SQLITE_PRIVATE void sqlite3StatusUp(int op, int N){
  wsdStatInit;
  assert( op>=0 && op<ArraySize(wsdStat.nowValue) );
  assert( op>=0 && op<ArraySize(statMutex) );
  assert( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.nowValue[op] += N;
  if( wsdStat.nowValue[op]>wsdStat.mxValue[op] ){
    wsdStat.mxValue[op] = wsdStat.nowValue[op];
  }
}
SQLITE_PRIVATE void sqlite3StatusDown(int op, int N){
  wsdStatInit;
  assert( N>=0 );
  assert( op>=0 && op<ArraySize(statMutex) );
  assert( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  assert( op>=0 && op<ArraySize(wsdStat.nowValue) );
  wsdStat.nowValue[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
SQLITE_PRIVATE void sqlite3StatusHighwater(int op, int X){
  sqlite3StatValueType newValue;
  wsdStatInit;
  assert( X>=0 );
  newValue = (sqlite3StatValueType)X;
  assert( op>=0 && op<ArraySize(wsdStat.nowValue) );
  assert( op>=0 && op<ArraySize(statMutex) );
  assert( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  assert( op==SQLITE_STATUS_MALLOC_SIZE
          || op==SQLITE_STATUS_PAGECACHE_SIZE
          || op==SQLITE_STATUS_SCRATCH_SIZE
          || op==SQLITE_STATUS_PARSER_STACK );
  if( newValue>wsdStat.mxValue[op] ){
    wsdStat.mxValue[op] = newValue;
  }
}

/*
** Query status information.
*/
SQLITE_API int sqlite3_status64(
  int op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  int resetFlag
){
  sqlite3_mutex *pMutex;
  wsdStatInit;
  if( op<0 || op>=ArraySize(wsdStat.nowValue) ){
    return SQLITE_MISUSE_BKPT;
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  if( pCurrent==0 || pHighwater==0 ) return SQLITE_MISUSE_BKPT;
#endif
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.nowValue[op];
  *pHighwater = wsdStat.mxValue[op];
  if( resetFlag ){
    wsdStat.mxValue[op] = wsdStat.nowValue[op];
  }
  sqlite3_mutex_leave(pMutex);
  (void)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  return SQLITE_OK;
}
SQLITE_API int sqlite3_status(int op, int *pCurrent, int *pHighwater, int resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  int rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  if( pCurrent==0 || pHighwater==0 ) return SQLITE_MISUSE_BKPT;
#endif
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  if( rc==0 ){
    *pCurrent = (int)iCur;
    *pHighwater = (int)iHwtr;
  }
  return rc;
}

/*
** Query status information for a single database connection
*/
SQLITE_API int sqlite3_db_status(
  sqlite3 *db,          /* The database connection whose status is desired */
  int op,               /* Status verb */
  int *pCurrent,        /* Write current value here */
  int *pHighwater,      /* Write high-water mark here */
  int resetFlag         /* Reset high-water mark if true */
){
  int rc = SQLITE_OK;   /* Return code */
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwater==0 ){
    return SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  switch( op ){
    case SQLITE_DBSTATUS_LOOKASIDE_USED: {
      *pCurrent = db->lookaside.nOut;
      *pHighwater = db->lookaside.mxOut;
      if( resetFlag ){
        db->lookaside.mxOut = db->lookaside.nOut;
      }
      break;
    }

    case SQLITE_DBSTATUS_LOOKASIDE_HIT:
    case SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE:
    case SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_HIT );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL );
      assert( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)>=0 );
      assert( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)<3 );
      *pCurrent = 0;
      *pHighwater = db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_HIT];
      if( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_HIT] = 0;
      }
      break;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    case SQLITE_DBSTATUS_CACHE_USED_SHARED:
    case SQLITE_DBSTATUS_CACHE_USED: {
      int totalUsed = 0;
      int i;
      sqlite3BtreeEnterAll(db);
      for(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        if( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          int nByte = sqlite3PagerMemUsed(pPager);
          if( op==SQLITE_DBSTATUS_CACHE_USED_SHARED ){
            nByte = nByte / sqlite3BtreeConnectionCount(pBt);
          }
          totalUsed += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = totalUsed;
      *pHighwater = 0;
      break;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwater is set to zero.
    */
    case SQLITE_DBSTATUS_SCHEMA_USED: {
      int i;                      /* Used to iterate through schemas */
      int nByte = 0;              /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pnBytesFreed = &nByte;
      for(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        if( ALWAYS(pSchema!=0) ){
          HashElem *p;

          nByte += sqlite3GlobalConfig.m.xRoundup(sizeof(HashElem)) * (
              pSchema->tblHash.count 
            + pSchema->trigHash.count
            + pSchema->idxHash.count
            + pSchema->fkeyHash.count
          );
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          for(p=sqliteHashFirst(&pSchema->trigHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqliteHashData(p));
          }
          for(p=sqliteHashFirst(&pSchema->tblHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTable(db, (Table *)sqliteHashData(p));
          }
        }
      }
      db->pnBytesFreed = 0;
      sqlite3BtreeLeaveAll(db);

      *pHighwater = 0;
      *pCurrent = nByte;
      break;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store all prepared statements.
    ** *pHighwater is set to zero.
    */
    case SQLITE_DBSTATUS_STMT_USED: {
      struct Vdbe *pVdbe;         /* Used to iterate through VMs */
      int nByte = 0;              /* Used to accumulate return value */

      db->pnBytesFreed = &nByte;
      for(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pNext){
        sqlite3VdbeClearObject(db, pVdbe);
        sqlite3DbFree(db, pVdbe);
      }
      db->pnBytesFreed = 0;

      *pHighwater = 0;  /* IMP: R-64479-57858 */
      *pCurrent = nByte;

      break;
    }

    /*
    ** Set *pCurrent to the total cache hits or misses encountered by all
    ** pagers the database handle is connected to. *pHighwater is always set 
    ** to zero.
    */
    case SQLITE_DBSTATUS_CACHE_HIT:
    case SQLITE_DBSTATUS_CACHE_MISS:
    case SQLITE_DBSTATUS_CACHE_WRITE:{
      int i;
      int nRet = 0;
      assert( SQLITE_DBSTATUS_CACHE_MISS==SQLITE_DBSTATUS_CACHE_HIT+1 );
      assert( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_HIT+2 );

      for(i=0; i<db->nDb; i++){
        if( db->aDb[i].pBt ){
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwater = 0; /* IMP: R-42420-56072 */
                       /* IMP: R-54100-20147 */
                       /* IMP: R-29431-39229 */
      *pCurrent = nRet;
      break;
    }

    /* Set *pCurrent to non-zero if there are unresolved deferred foreign
    ** key constraints.  Set *pCurrent to zero if all foreign key constraints
    ** have been satisfied.  The *pHighwater is always set to zero.
    */
    case SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwater = 0;  /* IMP: R-11967-56545 */
      *pCurrent = db->nDeferredImmCons>0 || db->nDeferredCons>0;
      break;
    }

    default: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

/************** End of status.c **********************************************/
