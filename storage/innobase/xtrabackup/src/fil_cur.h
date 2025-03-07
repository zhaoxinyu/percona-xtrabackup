/******************************************************
XtraBackup: hot backup tool for InnoDB
(c) 2009-2021 Percona LLC and/or its affiliates.
Originally Created 3/3/2009 Yasufumi Kinoshita
Written by Alexey Kopytov, Aleksandr Kuzminsky, Stewart Smith, Vadim Tkachenko,
Yasufumi Kinoshita, Ignacio Nin and Baron Schwartz.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

*******************************************************/

/* Source file cursor interface */

#ifndef FIL_CUR_H
#define FIL_CUR_H

#include <my_dir.h>
#include "file_utils.h"
#include "read_filt.h"

struct xb_fil_cur_t {
  pfs_os_file_t file; /*!< source file handle */
  fil_node_t *node;   /*!< source tablespace node */
  char rel_path[FN_REFLEN];
  /*!< normalized file path */
  char abs_path[FN_REFLEN];
  /*!< absolute file path */
  MY_STAT statinfo;            /*!< information about the file */
  ulint zip_size;              /*!< compressed page size in bytes or 0
                               for uncompressed pages */
  ulint page_size;             /*!< = zip_size for compressed pages or
                               UNIV_PAGE_SIZE for uncompressed ones */
  ulint page_size_shift;       /*!< bit shift corresponding to
                               page_size */
  bool is_system;              /*!< true for system tablespace, false
                               otherwise */
  bool is_ibd;                 /*!< true for IBD tablespace tablespace,
                               false otherwise */
  bool is_encrypted; /*!< true for encrypted tables. Used to determine with we
                     should compress a table when using --compress during
                     backup. Encrypted tables are not compressed and copied
                     as it is. */
  xb_read_filt_t *read_filter; /*!< read filter */
  xb_read_filt_ctxt_t read_filter_ctxt;
  /*!< read filter context */
  byte *orig_buf;         /*!< read buffer */
  byte *buf;              /*!< aligned pointer for orig_buf */
  byte *scratch;          /*!< page to use for temporary
                          decompress */
  byte *decrypt;          /*!< page to use for temporary
                          decrypt */
  ulint buf_size;         /*!< buffer size in bytes */
  ulint buf_read;         /*!< number of read bytes in buffer
                          after the last cursor read */
  ulint buf_npages;       /*!< number of pages in buffer after the
                          last cursor read */
  uint64_t buf_offset;    /*!< file offset of the first page in
                             buffer */
  ulint buf_page_no;      /*!< number of the first page in
                          buffer */
  uint thread_n;          /*!< thread number for diagnostics */
  ulint space_id;         /*!< ID of tablespace */
  ulint space_flags;      /*!< tablespace flags */
  ulint space_size;       /*!< space size in pages */
  size_t block_size;      /*!< FS block size */

  unsigned char encryption_key[32];
  /*!< encryption key */
  ulint encryption_klen;
  /*!< encryption key length */
  unsigned char encryption_iv[32];
  /*!< encryption iv */
};


/************************************************************************
Open a source file cursor and initialize the associated read filter.

@return XB_FIL_CUR_SUCCESS on success, XB_FIL_CUR_SKIP if the source file must
be skipped and XB_FIL_CUR_ERROR on error. */
xb_fil_cur_result_t xb_fil_cur_open(
    /*============*/
    xb_fil_cur_t *cursor,        /*!< out: source file cursor */
    xb_read_filt_t *read_filter, /*!< in/out: the read filter */
    fil_node_t *node,            /*!< in: source tablespace node */
    uint thread_n);              /*!< thread number for diagnostics */

/************************************************************************
Reads and verifies the next block of pages from the source
file. Positions the cursor after the last read non-corrupted page.

@return XB_FIL_CUR_SUCCESS if some have been read successfully, XB_FIL_CUR_EOF
if there are no more pages to read and XB_FIL_CUR_ERROR on error. */
xb_fil_cur_result_t xb_fil_cur_read(
    /*============*/
    xb_fil_cur_t *cursor); /*!< in/out: source file cursor */

/************************************************************************
Close the source file cursor opened with xb_fil_cur_open() and its
associated read filter. */
void xb_fil_cur_close(
    /*=============*/
    xb_fil_cur_t *cursor); /*!< in/out: source file cursor */

/***********************************************************************
Extracts the relative path ("database/table.ibd") of a tablespace from a
specified possibly absolute path.

For user tablespaces both "./database/table.ibd" and
"/remote/dir/database/table.ibd" result in "database/table.ibd".

For system tablepsaces (i.e. When is_system is true) both "/remote/dir/ibdata1"
and "./ibdata1" yield "ibdata1" in the output. */
const char *xb_get_relative_path(
    /*=================*/
    fil_space_t *space, /*!< in: tablespace */
    const char *path);  /*!< in: tablespace path (either
                        relative or absolute) */

/** Reads and verifies the next block of pages from the source
file. Positions the cursor after the last read non-corrupted page.
@param[in/out]	cursor	 	source file cursor
@param[in]	offset	 	offset of file to read from
@param[in]	to_read		bytest to read
@return XB_FIL_CUR_SUCCESS if some have been read successfully, XB_FIL_CUR_EOF
if there are no more pages to read and XB_FIL_CUR_ERROR on error */
xb_fil_cur_result_t xb_fil_cur_read_from_offset(xb_fil_cur_t *cursor,
                                                uint64_t offset,
                                                uint64_t to_read);
#endif
