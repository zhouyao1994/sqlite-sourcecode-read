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
#include "sqlite3-1-sqliteInt.h"
#include "sqlite3-2-global.c"
#include "sqlite3-3-ctime.c"
#include "sqlite3-4-status.c"
#include "sqlite3-5-date.c"
#include "sqlite3-6-os.c"
#include "sqlite3-7-fault.c"
#include "sqlite3-8-mem0.c"
#include "sqlite3-9-mem1.c"
#include "sqlite3-10-mem2.c"
#include "sqlite3-11-mem3.c"
#include "sqlite3-12-mem5.c"
#include "sqlite3-13-mutex.c"
#include "sqlite3-14-mutex_noop.c"
#include "sqlite3-15-mutex_unix.c"
#include "sqlite3-16-mutex_w32.c"
#include "sqlite3-17-malloc.c"
#include "sqlite3-18-printf.c"
#include "sqlite3-19-treeview.c"
#include "sqlite3-20-random.c"
#include "sqlite3-21-threads.c"
#include "sqlite3-22-utf.c"
#include "sqlite3-23-util.c"
#include "sqlite3-24-hash.c"
#include "sqlite3-25-opcodes.c"
#include "sqlite3-26-os_unix.c"
#include "sqlite3-27-os_win.c"
#include "sqlite3-28-bitvec.c"
#include "sqlite3-29-pcache.c"
#include "sqlite3-30-pcache1.c"
#include "sqlite3-31-rowset.c"
#include "sqlite3-32-pager.c"
#include "sqlite3-33-wal.c"
#include "sqlite3-34-btmutex.c"
#include "sqlite3-35-btree.c"
#include "sqlite3-36-backup.c"
#include "sqlite3-37-vdbemem.c"
#include "sqlite3-38-vdbeaux.c"
#include "sqlite3-39-vdbeapi.c"
#include "sqlite3-40-vdbetrace.c"
#include "sqlite3-41-vdbe.c"
#include "sqlite3-42-vdbeblob.c"
#include "sqlite3-43-vdbesort.c"
#include "sqlite3-44-memjournal.c"
#include "sqlite3-45-walker.c"
#include "sqlite3-46-resolve.c"
#include "sqlite3-47-expr.c"
#include "sqlite3-48-alter.c"
#include "sqlite3-49-analyze.c"
#include "sqlite3-50-attach.c"
#include "sqlite3-51-auth.c"
#include "sqlite3-52-build.c"
#include "sqlite3-53-callback.c"
#include "sqlite3-54-delete.c"
#include "sqlite3-55-func.c"
#include "sqlite3-56-fkey.c"
#include "sqlite3-57-insert.c"
#include "sqlite3-58-legacy.c"
#include "sqlite3-59-loadext.c"
#include "sqlite3-60-pragma.c"
#include "sqlite3-61-prepare.c"
#include "sqlite3-62-select.c"
#include "sqlite3-63-table.c"
#include "sqlite3-64-trigger.c"
#include "sqlite3-65-update.c"
#include "sqlite3-66-vacuum.c"
#include "sqlite3-67-vtab.c"
#include "sqlite3-68-wherecode.c"
#include "sqlite3-69-whereexpr.c"
#include "sqlite3-70-where.c"
#include "sqlite3-71-parse.c"
#include "sqlite3-72-tokenize.c"
#include "sqlite3-73-complete.c"
#include "sqlite3-74-main.c"
#include "sqlite3-75-notify.c"
#include "sqlite3-76-fts3.c"
#include "sqlite3-77-fts3_aux.c"
#include "sqlite3-78-fts3_expr.c"
#include "sqlite3-79-fts3_hash.c"
#include "sqlite3-80-fts3_porter.c"
#include "sqlite3-81-fts3_tokenizer.c"
#include "sqlite3-82-fts3_tokenizer1.c"
#include "sqlite3-83-fts3_tokenize_vtab.c"
#include "sqlite3-84-fts3_write.c"
#include "sqlite3-85-fts3_snippet.c"
#include "sqlite3-86-fts3_unicode.c"
#include "sqlite3-87-fts3_unicode2.c"
#include "sqlite3-88-rtree.c"
#include "sqlite3-89-icu.c"
#include "sqlite3-90-fts3_icu.c"
#include "sqlite3-91-sqlite3rbu.c"
#include "sqlite3-92-dbstat.c"
#include "sqlite3-93-sqlite3session.c"
#include "sqlite3-94-json1.c"
#include "sqlite3-95-fts5.c"
#include "sqlite3-96-the_last.c"
