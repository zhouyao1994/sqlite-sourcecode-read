/******************************************************************************
** This file is an amalgamation of many separate C source files from SQLite
** version 3.16.2.  By combining all the individual C code files into this
** single large file, the entire code can be compiled as a single translation
** unit.  This allows many compilers to do optimizations that would not be
** possible if the files were compiled separately.  Performance improvements
** of 5% or more are commonly seen when SQLite is compiled as a single
** translation unit.
**
** This file is all you need to compile SQLite.  To use SQLite in other
** programs, you need this file and the "sqlite3.h" header file that defines
** the programming interface to the SQLite library.  (If you do not have
** the "sqlite3.h" header file at hand, you will find a copy embedded within
** the text of this file.  Search for "Begin file sqlite3.h" to find the start
** of the embedded sqlite3.h header file.) Additional code files may be needed
** if you want a wrapper to interface SQLite with your choice of programming
** language. The code for the "sqlite3" command-line shell is also in a
** separate file. This file contains only code for the core SQLite library.
*/
#define SQLITE_CORE 1
#define SQLITE_AMALGAMATION 1
#ifndef SQLITE_PRIVATE
# define SQLITE_PRIVATE static
#endif
#include "sqlite3-1-"
#include "sqlite3-2-sqliteInt.h"
#include "sqlite3-3-global.c"
#include "sqlite3-4-ctime.c"
#include "sqlite3-5-status.c"
#include "sqlite3-6-date.c"
#include "sqlite3-7-os.c"
#include "sqlite3-8-fault.c"
#include "sqlite3-9-mem0.c"
#include "sqlite3-10-mem1.c"
#include "sqlite3-11-mem2.c"
#include "sqlite3-12-mem3.c"
#include "sqlite3-13-mem5.c"
#include "sqlite3-14-mutex.c"
#include "sqlite3-15-mutex_noop.c"
#include "sqlite3-16-mutex_unix.c"
#include "sqlite3-17-mutex_w32.c"
#include "sqlite3-18-malloc.c"
#include "sqlite3-19-printf.c"
#include "sqlite3-20-treeview.c"
#include "sqlite3-21-random.c"
#include "sqlite3-22-threads.c"
#include "sqlite3-23-utf.c"
#include "sqlite3-24-util.c"
#include "sqlite3-25-hash.c"
#include "sqlite3-26-opcodes.c"
#include "sqlite3-27-os_unix.c"
#include "sqlite3-28-os_win.c"
#include "sqlite3-29-bitvec.c"
#include "sqlite3-30-pcache.c"
#include "sqlite3-31-pcache1.c"
#include "sqlite3-32-rowset.c"
#include "sqlite3-33-pager.c"
#include "sqlite3-34-wal.c"
#include "sqlite3-35-btmutex.c"
#include "sqlite3-36-btree.c"
#include "sqlite3-37-backup.c"
#include "sqlite3-38-vdbemem.c"
#include "sqlite3-39-vdbeaux.c"
#include "sqlite3-40-vdbeapi.c"
#include "sqlite3-41-vdbetrace.c"
#include "sqlite3-42-vdbe.c"
#include "sqlite3-43-vdbeblob.c"
#include "sqlite3-44-vdbesort.c"
#include "sqlite3-45-memjournal.c"
#include "sqlite3-46-walker.c"
#include "sqlite3-47-resolve.c"
#include "sqlite3-48-expr.c"
#include "sqlite3-49-alter.c"
#include "sqlite3-50-analyze.c"
#include "sqlite3-51-attach.c"
#include "sqlite3-52-auth.c"
#include "sqlite3-53-build.c"
#include "sqlite3-54-callback.c"
#include "sqlite3-55-delete.c"
#include "sqlite3-56-func.c"
#include "sqlite3-57-fkey.c"
#include "sqlite3-58-insert.c"
#include "sqlite3-59-legacy.c"
#include "sqlite3-60-loadext.c"
#include "sqlite3-61-pragma.c"
#include "sqlite3-62-prepare.c"
#include "sqlite3-63-select.c"
#include "sqlite3-64-table.c"
#include "sqlite3-65-trigger.c"
#include "sqlite3-66-update.c"
#include "sqlite3-67-vacuum.c"
#include "sqlite3-68-vtab.c"
#include "sqlite3-69-wherecode.c"
#include "sqlite3-70-whereexpr.c"
#include "sqlite3-71-where.c"
#include "sqlite3-72-parse.c"
#include "sqlite3-73-tokenize.c"
#include "sqlite3-74-complete.c"
#include "sqlite3-75-main.c"
#include "sqlite3-76-notify.c"
#include "sqlite3-77-fts3.c"
#include "sqlite3-78-fts3_aux.c"
#include "sqlite3-79-fts3_expr.c"
#include "sqlite3-80-fts3_hash.c"
#include "sqlite3-81-fts3_porter.c"
#include "sqlite3-82-fts3_tokenizer.c"
#include "sqlite3-83-fts3_tokenizer1.c"
#include "sqlite3-84-fts3_tokenize_vtab.c"
#include "sqlite3-85-fts3_write.c"
#include "sqlite3-86-fts3_snippet.c"
#include "sqlite3-87-fts3_unicode.c"
#include "sqlite3-88-fts3_unicode2.c"
#include "sqlite3-89-rtree.c"
#include "sqlite3-90-icu.c"
#include "sqlite3-91-fts3_icu.c"
#include "sqlite3-92-sqlite3rbu.c"
#include "sqlite3-93-dbstat.c"
#include "sqlite3-94-sqlite3session.c"
#include "sqlite3-95-json1.c"
#include "sqlite3-96-the_last.c"
