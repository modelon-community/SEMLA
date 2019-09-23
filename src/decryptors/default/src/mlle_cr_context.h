#ifndef MLLE_CR_CONTEXT_H_
#define MLLE_CR_CONTEXT_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <uthash.h>

#include "random_key_file.h"

#define PACKAGE_MO_STRLEN 10
#define PACKAGE_MOC_STRLEN 11

typedef struct mlle_key_mask_map {
    char* relpath;             /* relative path to directory acts as hash key */    
    UT_hash_handle hh;         /* makes this structure hashable */
    char key_mask[MLLE_CR_KEY_LEN];
    char buffer[2];
} mlle_key_mask_map;

typedef struct mlle_cr_context {
    char no_mask[MLLE_CR_KEY_LEN]; /* empty mask used for top-level package.moc */
    struct mlle_key_mask_map* keymask_map; /* makes this structure hashable */
    char basedir[1];             /*  basedir where all encrypted files are stored.  */
    /*  Relpath used in keymap are relative to this directory. */
} mlle_cr_context;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif