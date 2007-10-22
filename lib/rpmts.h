#ifndef H_RPMTS
#define H_RPMTS

/** \ingroup rpmts
 * \file lib/rpmts.h
 * Structures and prototypes used for an "rpmts" transaction set.
 */

#include "rpmlib.h"	/* for rpmts */
#include "rpmps.h"
#include "rpmsw.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int _rpmts_debug;
extern int _rpmts_stats;
extern int _fps_debug;

/**
 * Bit(s) to control digest and signature verification.
 */
typedef enum rpmVSFlags_e {
    RPMVSF_DEFAULT	= 0,
    RPMVSF_NOHDRCHK	= (1 <<  0),
    RPMVSF_NEEDPAYLOAD	= (1 <<  1),
    /* bit(s) 2-7 unused */
    RPMVSF_NOSHA1HEADER	= (1 <<  8),
    RPMVSF_NOMD5HEADER	= (1 <<  9),	/* unimplemented */
    RPMVSF_NODSAHEADER	= (1 << 10),
    RPMVSF_NORSAHEADER	= (1 << 11),	/* unimplemented */
    /* bit(s) 12-15 unused */
    RPMVSF_NOSHA1	= (1 << 16),	/* unimplemented */
    RPMVSF_NOMD5	= (1 << 17),
    RPMVSF_NODSA	= (1 << 18),
    RPMVSF_NORSA	= (1 << 19)
    /* bit(s) 16-31 unused */
} rpmVSFlags;

/**
 * Transaction Types
 */
typedef enum rpmtsType_e {
	RPMTRANS_TYPE_NORMAL       = 0,
	RPMTRANS_TYPE_ROLLBACK     = (1 << 0),
	RPMTRANS_TYPE_AUTOROLLBACK = (1 << 1)
} rpmtsType;

#define	_RPMVSF_NODIGESTS	\
  ( RPMVSF_NOSHA1HEADER |	\
    RPMVSF_NOMD5HEADER |	\
    RPMVSF_NOSHA1 |		\
    RPMVSF_NOMD5 )

#define	_RPMVSF_NOSIGNATURES	\
  ( RPMVSF_NODSAHEADER |	\
    RPMVSF_NORSAHEADER |	\
    RPMVSF_NODSA |		\
    RPMVSF_NORSA )

#define	_RPMVSF_NOHEADER	\
  ( RPMVSF_NOSHA1HEADER |	\
    RPMVSF_NOMD5HEADER |	\
    RPMVSF_NODSAHEADER |	\
    RPMVSF_NORSAHEADER )

#define	_RPMVSF_NOPAYLOAD	\
  ( RPMVSF_NOSHA1 |		\
    RPMVSF_NOMD5 |		\
    RPMVSF_NODSA |		\
    RPMVSF_NORSA )

/**
 * Indices for timestamps.
 */
typedef	enum rpmtsOpX_e {
    RPMTS_OP_TOTAL		=  0,
    RPMTS_OP_CHECK		=  1,
    RPMTS_OP_ORDER		=  2,
    RPMTS_OP_FINGERPRINT	=  3,
    RPMTS_OP_REPACKAGE		=  4,
    RPMTS_OP_INSTALL		=  5,
    RPMTS_OP_ERASE		=  6,
    RPMTS_OP_SCRIPTLETS		=  7,
    RPMTS_OP_COMPRESS		=  8,
    RPMTS_OP_UNCOMPRESS		=  9,
    RPMTS_OP_DIGEST		= 10,
    RPMTS_OP_SIGNATURE		= 11,
    RPMTS_OP_DBADD		= 12,
    RPMTS_OP_DBREMOVE		= 13,
    RPMTS_OP_DBGET		= 14,
    RPMTS_OP_DBPUT		= 15,
    RPMTS_OP_DBDEL		= 16,
    RPMTS_OP_MAX		= 17
} rpmtsOpX;

#if defined(_RPMTS_INTERNAL)

#include "rpmhash.h"	/* XXX hashTable */
#include "rpmal.h"	/* XXX availablePackage/relocateFileList ,*/

/**********************
 * Transaction Scores *
 **********************
 *
 * In order to allow instance counts to be adjusted properly when an
 * autorollback transaction is ran, we keep a list that is indexed
 * by rpm name of whether the rpm has been installed or erased.  This listed
 * is only updated:
 *
 *	iif autorollbacks are enabled.
 *	iif this is not a rollback or autorollback transaction.
 *
 * When creating an autorollback transaction, its rpmts points to the same
 * rpmtsScore object as the running transaction.  So when the autorollback
 * transaction runs it can see where each package was in the running transaction
 * at the point the running transaction failed, and thus on a per package
 * basis make adjustments to the instance counts.
 *
 * XXX: Jeff, I am not convinced that this does not need to be in its own file
 *      (i.e. rpmtsScore.{h,c}), but I first wanted to get it working.
 */
struct rpmtsScoreEntry_s {
    char *         N;			/*!<Name of package                */
    rpmElementType te_types;		/*!<te types this entry represents */
    int            installed;		/*!<Was the new header installed   */
    int            erased;		/*!<Was the old header removed     */
};

typedef struct rpmtsScoreEntry_s * rpmtsScoreEntry;

struct rpmtsScore_s {
	int entries;			/*!< Number of scores       */
	rpmtsScoreEntry * scores;	/*!< Array of score entries */
    	int nrefs;			/*!< Reference count.       */
};

typedef struct rpmtsScore_s * rpmtsScore;


/** \ingroup rpmts
 * initialize rpmtsScore for running transaction and autorollback
 * transaction.
 * @param runningTS	Running Transaction.
 * @param rollbackTS	Rollback Transaction.
 * @return		RPMRC_OK
 */
rpmRC rpmtsScoreInit(rpmts runningTS, rpmts rollbackTS);

/** \ingroup rpmts
 * Free rpmtsScore provided no more references exist against it.
 * @param score		rpmtsScore to free
 * @return		NULL always
 */
rpmtsScore rpmtsScoreFree(rpmtsScore score);

/** \ingroup rpmts
 * Get rpmtsScore from transaction.
 * @param ts	RPM Transaction.
 * @return	rpmtsScore or NULL.
 */
rpmtsScore rpmtsGetScore(rpmts ts);

/** \ingroup rpmts
 * Get rpmtsScoreEntry from rpmtsScore.
 * @param score   RPM Transaction Score.
 * @return	  rpmtsScoreEntry or NULL.
 */
rpmtsScoreEntry rpmtsScoreGetEntry(rpmtsScore score, const char *N);

/** \ingroup rpmts
 * \file lib/rpmts.h
 * Structures and prototypes used for an "rpmts" transaction set.
 */

/**************************
 * END Transaction Scores *
 **************************/

extern int _cacheDependsRC;

/** \ingroup rpmts
 */
typedef	struct diskspaceInfo_s * rpmDiskSpaceInfo;

/** \ingroup rpmts
 */
struct diskspaceInfo_s {
    dev_t dev;			/*!< File system device number. */
    signed long bneeded;	/*!< No. of blocks needed. */
    signed long ineeded;	/*!< No. of inodes needed. */
    int bsize;			/*!< File system block size. */
    signed long long bavail;	/*!< No. of blocks available. */
    signed long long iavail;	/*!< No. of inodes available. */
};

/** \ingroup rpmts
 * Adjust for root only reserved space. On linux e2fs, this is 5%.
 */
#define	adj_fs_blocks(_nb)	(((_nb) * 21) / 20)

/* argon thought a shift optimization here was a waste of time...  he's
   probably right :-( */
#define BLOCK_ROUND(size, block) (((size) + (block) - 1) / (block))

/** \ingroup rpmts
 */
typedef enum tsStage_e {
    TSM_UNKNOWN		=  0,
    TSM_INSTALL		=  7,
    TSM_ERASE		=  8,
} tsmStage;

/** \ingroup rpmts
 * The set of packages to be installed/removed atomically.
 */
struct rpmts_s {
    rpmtransFlags transFlags;	/*!< Bit(s) to control operation. */
    tsmStage goal;		/*!< Transaction goal (i.e. mode) */
    rpmtsType type;             /*!< default, rollback, autorollback */

    rpmdb sdb;			/*!< Solve database handle. */
    int sdbmode;		/*!< Solve database open mode. */
    int (*solve) (rpmts ts, rpmds key, const void * data);
                                /*!< Search for NEVRA key. */
    const void * solveData;	/*!< Solve callback data */
    int nsuggests;		/*!< No. of depCheck suggestions. */
    const void ** suggests;	/*!< Possible depCheck suggestions. */

    rpmCallbackFunction notify;	/*!< Callback function. */
    rpmCallbackData notifyData;	/*!< Callback private data. */

    rpmps probs;		/*!< Current problems in transaction. */
    rpmprobFilterFlags ignoreSet;
				/*!< Bits to filter current problems. */

    int filesystemCount;	/*!< No. of mounted filesystems. */
    const char ** filesystems;	/*!< Mounted filesystem names. */
    rpmDiskSpaceInfo dsi;	/*!< Per filesystem disk/inode usage. */

    rpmdb rdb;			/*!< Install database handle. */
    int dbmode;			/*!< Install database open mode. */
    hashTable ht;		/*!< Fingerprint hash table. */

    int * removedPackages;	/*!< Set of packages being removed. */
    int numRemovedPackages;	/*!< No. removed package instances. */
    int allocedRemovedPackages;	/*!< Size of removed packages array. */

    rpmal addedPackages;	/*!< Set of packages being installed. */
    int numAddedPackages;	/*!< No. added package instances. */

#ifndef	DYING
    rpmal availablePackages;	/*!< Universe of available packages. */
    int numAvailablePackages;	/*!< No. available package instances. */
#endif

    rpmte relocateElement;	/*!< Element to use when relocating packages. */

    rpmte * order;		/*!< Packages sorted by dependencies. */
    int orderCount;		/*!< No. of transaction elements. */
    int orderAlloced;		/*!< No. of allocated transaction elements. */
    int unorderedSuccessors;	/*!< Index of 1st element of successors. */
    int ntrees;			/*!< No. of dependency trees. */
    int maxDepth;		/*!< Maximum depth of dependency tree(s). */

    int selinuxEnabled;		/*!< Is SE linux enabled? */
    int chrootDone;		/*!< Has chroot(2) been been done? */
    const char * rootDir;	/*!< Path to top of install tree. */
    const char * currDir;	/*!< Current working directory. */
    FD_t scriptFd;		/*!< Scriptlet stdout/stderr. */
    int delta;			/*!< Delta for reallocation. */
    int_32 tid;			/*!< Transaction id. */

    uint_32 color;		/*!< Transaction color bits. */
    uint_32 prefcolor;		/*!< Preferred file color. */

    rpmVSFlags vsflags;		/*!< Signature/digest verification flags. */

    const char * fn;		/*!< Current package fn. */
    int_32  sigtag;		/*!< Current package signature tag. */
    int_32  sigtype;		/*!< Current package signature data type. */
    const void * sig;		/*!< Current package signature. */
    int_32 siglen;		/*!< Current package signature length. */

    const unsigned char * pkpkt;/*!< Current pubkey packet. */
    size_t pkpktlen;		/*!< Current pubkey packet length. */
    unsigned char pksignid[8];	/*!< Current pubkey fingerprint. */

    struct rpmop_s ops[RPMTS_OP_MAX];

    pgpDig dig;			/*!< Current signature/pubkey parameters. */

    rpmSpec spec;		/*!< Spec file control structure. */

    rpmtsScore score;		/*!< Transaction Score (autorollback). */

    int nrefs;			/*!< Reference count. */
};
#endif	/* _RPMTS_INTERNAL */

/** \ingroup rpmts
 * Perform dependency resolution on the transaction set.
 *
 * Any problems found by rpmtsCheck() can be examined by retrieving the 
 * problem set with rpmtsProblems(), success here only means that
 * the resolution was successfully attempted for all packages in the set.
 *
 * @param ts		transaction set
 * @return		0 on success
 */
int rpmtsCheck(rpmts ts);

/** \ingroup rpmts
 * Determine package order in a transaction set according to dependencies.
 *
 * Order packages, returning error if circular dependencies cannot be
 * eliminated by removing Requires's from the loop(s). Only dependencies from
 * added or removed packages are used to determine ordering using a
 * topological sort (Knuth vol. 1, p. 262). Use rpmtsCheck() to verify
 * that all dependencies can be resolved.
 *
 * The final order ends up as installed packages followed by removed packages,
 * with packages removed for upgrades immediately following the new package
 * to be installed.
 *
 * @param ts		transaction set
 * @return		no. of (added) packages that could not be ordered
 */
int rpmtsOrder(rpmts ts);

/** \ingroup rpmts
 * Process all package elements in a transaction set.  Before calling
 * rpmtsRun be sure to have:
 *
 *    - setup the rpm root dir via rpmtsSetRoot().
 *    - setup the rpm notify callback via rpmtsSetNotifyCallback().
 *    - setup the rpm transaction flags via rpmtsSetFlags().
 * 
 * Additionally, though not required you may want to:
 *
 *    - setup the rpm verify signature flags via rpmtsSetVSFlags().
 *       
 * @param ts		transaction set
 * @param okProbs	previously known problems (or NULL)
 * @param ignoreSet	bits to filter problem types
 * @return		0 on success, -1 on error, >0 with newProbs set
 */
int rpmtsRun(rpmts ts, rpmps okProbs, rpmprobFilterFlags ignoreSet);

/** \ingroup rpmts
 * Unreference a transaction instance.
 * @param ts		transaction set
 * @param msg
 * @return		NULL always
 */
rpmts rpmtsUnlink (rpmts ts,
		const char * msg);

/** @todo Remove debugging entry from the ABI. */
rpmts XrpmtsUnlink (rpmts ts,
		const char * msg, const char * fn, unsigned ln);
#define	rpmtsUnlink(_ts, _msg)	XrpmtsUnlink(_ts, _msg, __FILE__, __LINE__)

/** \ingroup rpmts
 * Reference a transaction set instance.
 * @param ts		transaction set
 * @param msg
 * @return		new transaction set reference
 */
rpmts rpmtsLink (rpmts ts, const char * msg);

/** @todo Remove debugging entry from the ABI. */
rpmts XrpmtsLink (rpmts ts,
		const char * msg, const char * fn, unsigned ln);
#define	rpmtsLink(_ts, _msg)	XrpmtsLink(_ts, _msg, __FILE__, __LINE__)

/** \ingroup rpmts
 * Close the database used by the transaction.
 * @param ts		transaction set
 * @return		0 on success
 */
int rpmtsCloseDB(rpmts ts);

/** \ingroup rpmts
 * Open the database used by the transaction.
 * @param ts		transaction set
 * @param dbmode	O_RDONLY or O_RDWR
 * @return		0 on success
 */
int rpmtsOpenDB(rpmts ts, int dbmode);

/** \ingroup rpmts
 * Initialize the database used by the transaction.
 * @deprecated An explicit rpmdbInit() is almost never needed.
 * @param ts		transaction set
 * @param dbmode	O_RDONLY or O_RDWR
 * @return		0 on success
 */
int rpmtsInitDB(rpmts ts, int dbmode);

/** \ingroup rpmts
 * Return the transaction database mode
 * @param ts		transaction set
 * @return		O_RDONLY, O_RDWR or -1 (lazy opens disabled)
 */
int rpmtsGetDBMode(rpmts ts);

/** \ingroup rpmts
 * Set the transaction database mode. Only permitted when when backing
 * database hasn't been opened yet (ie rpmtsGetRdb(ts) == NULL)
 * @param ts		transaction set
 * @param dbmode	O_RDONLY, O_RDWR or -1 (disable lazy opens)
 * @return		0 on success, 1 on error 
 */
int rpmtsSetDBMode(rpmts ts, int dbmode);

/** \ingroup rpmts
 * Rebuild the database used by the transaction.
 * @param ts		transaction set
 * @return		0 on success
 */
int rpmtsRebuildDB(rpmts ts);

/** \ingroup rpmts
 * Verify the database used by the transaction.
 * @param ts		transaction set
 * @return		0 on success
 */
int rpmtsVerifyDB(rpmts ts);

/** \ingroup rpmts
 * Return transaction database iterator.
 * @param ts		transaction set
 * @param rpmtag	rpm tag
 * @param keyp		key data (NULL for sequential access)
 * @param keylen	key data length (0 will use strlen(keyp))
 * @return		NULL on failure
 */
rpmdbMatchIterator rpmtsInitIterator(const rpmts ts, rpmTag rpmtag,
			const void * keyp, size_t keylen);

/**
 * Retrieve pubkey from rpm database.
 * @param ts		rpm transaction
 * @return		RPMRC_OK on success, RPMRC_NOKEY if not found
 */
rpmRC rpmtsFindPubkey(rpmts ts);

/** \ingroup rpmts
 * Close the database used by the transaction to solve dependencies.
 * @param ts		transaction set
 * @return		0 on success
 */
int rpmtsCloseSDB(rpmts ts);

/** \ingroup rpmts
 * Open the database used by the transaction to solve dependencies.
 * @param ts		transaction set
 * @param dbmode	O_RDONLY or O_RDWR
 * @return		0 on success
 */
int rpmtsOpenSDB(rpmts ts, int dbmode);

/**
 * Attempt to solve a needed dependency using the solve database.
 * @param ts		transaction set
 * @param ds		dependency set
 * @param data		opaque data associated with callback
 * @return		-1 retry, 0 ignore, 1 not found
 */
int rpmtsSolve(rpmts ts, rpmds ds, const void * data);

/**
 * Attempt to solve a needed dependency using memory resident tables.
 * @deprecated This function will move from rpmlib to the python bindings.
 * @param ts		transaction set
 * @param ds		dependency set
 * @return		0 if resolved (and added to ts), 1 not found
 */
int rpmtsAvailable(rpmts ts, const rpmds ds);

/**
 * Set dependency solver callback.
 * @param ts		transaction set
 * @param (*solve)	dependency solver callback
 * @param solveData	dependency solver callback data (opaque)
 * @return		0 on success
 */
int rpmtsSetSolveCallback(rpmts ts,
		int (*solve) (rpmts ts, rpmds ds, const void * data),
		const void * solveData);

/**
 * Return the type of a transaction.
 * @param ts		transaction set
 * @return		0 it is not, 1 it is.
 */
rpmtsType rpmtsGetType(rpmts ts);

/**
 * Set transaction type.   Allowed types are:
 *
 * 	RPMTRANS_TYPE_NORMAL
 *	RPMTRANS_TYPE_ROLLBACK
 * 	RPMTRANS_TYPE_AUTOROLLBACK
 *
 * @param ts		transaction set
 * @param type		transaction type
 * @return		void
 */
void rpmtsSetType(rpmts ts, rpmtsType type);

/**
 * Return current transaction set problems.
 * @param ts		transaction set
 * @return		current problem set (or NULL)
 */
rpmps rpmtsProblems(rpmts ts);

/** \ingroup rpmts
 * Free signature verification data.
 * @param ts		transaction set
 */
void rpmtsCleanDig(rpmts ts);

/** \ingroup rpmts
 * Free memory needed only for dependency checks and ordering.
 * @param ts		transaction set
 */
void rpmtsClean(rpmts ts);

/** \ingroup rpmts
 * Re-create an empty transaction set.
 * @param ts		transaction set
 */
void rpmtsEmpty(rpmts ts);

/** \ingroup rpmts
 * Destroy transaction set, closing the database as well.
 * @param ts		transaction set
 * @return		NULL always
 */
rpmts rpmtsFree(rpmts ts);

/** \ingroup rpmts
 * Get verify signatures flag(s).
 * @param ts		transaction set
 * @return		verify signatures flags
 */
rpmVSFlags rpmtsVSFlags(rpmts ts);

/** \ingroup rpmts
 * Set verify signatures flag(s).
 * @param ts		transaction set
 * @param vsflags	new verify signatures flags
 * @return		previous value
 */
rpmVSFlags rpmtsSetVSFlags(rpmts ts, rpmVSFlags vsflags);

/** \ingroup rpmts
 * Set index of 1st element of successors.
 * @param ts		transaction set
 * @param first		new index of 1st element of successors
 * @return		previous value
 */
int rpmtsUnorderedSuccessors(rpmts ts, int first);

/** \ingroup rpmts
 * Get transaction rootDir, i.e. path to chroot(2).
 * @param ts		transaction set
 * @return		transaction rootDir
 */
extern const char * rpmtsRootDir(rpmts ts);

/** \ingroup rpmts
 * Set transaction rootDir, i.e. path to chroot(2).
 * @param ts		transaction set
 * @param rootDir	new transaction rootDir (or NULL)
 */
void rpmtsSetRootDir(rpmts ts, const char * rootDir);

/** \ingroup rpmts
 * Get transaction currDir, i.e. current directory before chroot(2).
 * @param ts		transaction set
 * @return		transaction currDir
 */
extern const char * rpmtsCurrDir(rpmts ts);

/** \ingroup rpmts
 * Set transaction currDir, i.e. current directory before chroot(2).
 * @param ts		transaction set
 * @param currDir	new transaction currDir (or NULL)
 */
void rpmtsSetCurrDir(rpmts ts, const char * currDir);

/** \ingroup rpmts
 * Get transaction script file handle, i.e. stdout/stderr on scriptlet execution
 * @param ts		transaction set
 * @return		transaction script file handle
 */
FD_t rpmtsScriptFd(rpmts ts);

/** \ingroup rpmts
 * Set transaction script file handle, i.e. stdout/stderr on scriptlet execution
 * @param ts		transaction set
 * @param scriptFd	new script file handle (or NULL)
 */
void rpmtsSetScriptFd(rpmts ts, FD_t scriptFd);

/** \ingroup rpmts
 * Get selinuxEnabled flag, i.e. is SE linux enabled?
 * @param ts		transaction set
 * @return		selinuxEnabled flag
 */
int rpmtsSELinuxEnabled(rpmts ts);

/** \ingroup rpmts
 * Get chrootDone flag, i.e. has chroot(2) been performed?
 * @param ts		transaction set
 * @return		chrootDone flag
 */
int rpmtsChrootDone(rpmts ts);

/** \ingroup rpmts
 * Set chrootDone flag, i.e. has chroot(2) been performed?
 * @param ts		transaction set
 * @param chrootDone	new chrootDone flag
 * @return		previous chrootDone flag
 */
int rpmtsSetChrootDone(rpmts ts, int chrootDone);

/** \ingroup rpmts
 * Get transaction id, i.e. transaction time stamp.
 * @param ts		transaction set
 * @return		transaction id
 */
int_32 rpmtsGetTid(rpmts ts);

/** \ingroup rpmts
 * Set transaction id, i.e. transaction time stamp.
 * @param ts		transaction set
 * @param tid		new transaction id
 * @return		previous transaction id
 */
int_32 rpmtsSetTid(rpmts ts, int_32 tid);

/** \ingroup rpmts
 * Get signature tag.
 * @param ts		transaction set
 * @return		signature tag
 */
int_32 rpmtsSigtag(const rpmts ts);

/** \ingroup rpmts
 * Get signature tag type.
 * @param ts		transaction set
 * @return		signature tag type
 */
int_32 rpmtsSigtype(const rpmts ts);

/** \ingroup rpmts
 * Get signature tag data, i.e. from header.
 * @param ts		transaction set
 * @return		signature tag data
 */
extern const void * rpmtsSig(const rpmts ts);

/** \ingroup rpmts
 * Get signature tag data length, i.e. no. of bytes of data.
 * @param ts		transaction set
 * @return		signature tag data length
 */
int_32 rpmtsSiglen(const rpmts ts);

/** \ingroup rpmts
 * Set signature tag info, i.e. from header.
 * @param ts		transaction set
 * @param sigtag	signature tag
 * @param sigtype	signature tag type
 * @param sig		signature tag data
 * @param siglen	signature tag data length
 * @return		0 always
 */
int rpmtsSetSig(rpmts ts,
		int_32 sigtag, int_32 sigtype,
		const void * sig, int_32 siglen);

/** \ingroup rpmts
 * Get OpenPGP packet parameters, i.e. signature/pubkey constants.
 * @param ts		transaction set
 * @return		signature/pubkey constants.
 */
pgpDig rpmtsDig(rpmts ts);

/** \ingroup rpmts
 * Get OpenPGP signature constants.
 * @param ts		transaction set
 * @return		signature constants.
 */
pgpDigParams rpmtsSignature(const rpmts ts);

/** \ingroup rpmts
 * Get OpenPGP pubkey constants.
 * @param ts		transaction set
 * @return		pubkey constants.
 */
pgpDigParams rpmtsPubkey(const rpmts ts);

/** \ingroup rpmts
 * Get transaction set database handle.
 * @param ts		transaction set
 * @return		transaction database handle
 */
rpmdb rpmtsGetRdb(rpmts ts);

/** \ingroup rpmts
 * Initialize disk space info for each and every mounted file systems.
 * @param ts		transaction set
 * @return		0 on success
 */
int rpmtsInitDSI(const rpmts ts);

/** \ingroup rpmts
 * Update disk space info for a file.
 * @param ts		transaction set
 * @param dev		mount point device
 * @param fileSize	file size
 * @param prevSize	previous file size (if upgrading)
 * @param fixupSize	size difference (if
 * @param action	file disposition
 */
void rpmtsUpdateDSI(const rpmts ts, dev_t dev,
		uint_32 fileSize, uint_32 prevSize, uint_32 fixupSize,
		rpmFileAction action);

/** \ingroup rpmts
 * Check a transaction element for disk space problems.
 * @param ts		transaction set
 * @param te		current transaction element
 */
void rpmtsCheckDSIProblems(const rpmts ts, const rpmte te);

/**
 * Perform transaction progress notify callback.
 * @param ts		transaction set
 * @param te		current transaction element
 * @param what		type of call back
 * @param amount	current value
 * @param total		final value
 * @return		callback dependent pointer
 */
void * rpmtsNotify(rpmts ts, rpmte te,
                rpmCallbackType what, unsigned long amount, unsigned long total);

/**
 * Return number of (ordered) transaction set elements.
 * @param ts		transaction set
 * @return		no. of transaction set elements
 */
int rpmtsNElements(rpmts ts);

/**
 * Return (ordered) transaction set element.
 * @param ts		transaction set
 * @param ix		transaction element index
 * @return		transaction element (or NULL)
 */
rpmte rpmtsElement(rpmts ts, int ix);

/** \ingroup rpmts
 * Get problem ignore bit mask, i.e. bits to filter encountered problems.
 * @param ts		transaction set
 * @return		ignore bit mask
 */
rpmprobFilterFlags rpmtsFilterFlags(rpmts ts);

/** \ingroup rpmts
 * Get transaction flags, i.e. bits that control rpmtsRun().
 * @param ts		transaction set
 * @return		transaction flags
 */
rpmtransFlags rpmtsFlags(rpmts ts);

/** \ingroup rpmts
 * Set transaction flags, i.e. bits that control rpmtsRun().
 * @param ts		transaction set
 * @param transFlags	new transaction flags
 * @return		previous transaction flags
 */
rpmtransFlags rpmtsSetFlags(rpmts ts, rpmtransFlags transFlags);

/** \ingroup rpmts
 * Get spec control structure from transaction set.
 * @param ts		transaction set
 * @return		spec control structure
 */
rpmSpec rpmtsSpec(rpmts ts);

/** \ingroup rpmts
 * Set a spec control structure in transaction set.
 * @param ts		transaction set
 * @param spec		new spec control structure
 * @return		previous spec control structure
 */
rpmSpec rpmtsSetSpec(rpmts ts, rpmSpec spec);

/** \ingroup rpmts
 * Get current relocate transaction element.
 * @param ts		transaction set
 * @return		current relocate transaction element
 */
rpmte rpmtsRelocateElement(rpmts ts);

/** \ingroup rpmts
 * Set current relocate transaction element.
 * @param ts		transaction set
 * @param relocateElement new relocate transaction element
 * @return		previous relocate transaction element
 */
rpmte rpmtsSetRelocateElement(rpmts ts, rpmte relocateElement);

/**
 * Retrieve color bits of transaction set.
 * @param ts		transaction set
 * @return		color bits
 */
uint_32 rpmtsColor(rpmts ts);

/**
 * Retrieve prefered file color
 * @param ts		transaction set
 * @return		color bits
 */
uint_32 rpmtsPrefColor(rpmts ts);

/**
 * Set color bits of transaction set.
 * @param ts		transaction set
 * @param color		new color bits
 * @return		previous color bits
 */
uint_32 rpmtsSetColor(rpmts ts, uint_32 color);

/**
 * Retrieve operation timestamp from a transaction set.
 * @param ts		transaction set
 * @param opx		operation timestamp index
 * @return		pointer to operation timestamp.
 */
rpmop rpmtsOp(rpmts ts, rpmtsOpX opx);

/** \ingroup rpmts
 * Set transaction notify callback function and argument.
 *
 * @warning This call must be made before rpmtsRun() for
 *	install/upgrade/freshen to function correctly.
 *
 * @param ts		transaction set
 * @param notify	progress callback
 * @param notifyData	progress callback private data
 * @return		0 on success
 */
int rpmtsSetNotifyCallback(rpmts ts,
		rpmCallbackFunction notify,
		rpmCallbackData notifyData);

/** \ingroup rpmts
 * Create an empty transaction set.
 * @return		new transaction set
 */
rpmts rpmtsCreate(void);

/** \ingroup rpmts
 * Add package to be installed to transaction set.
 *
 * The transaction set is checked for duplicate package names.
 * If found, the package with the "newest" EVR will be replaced.
 *
 * @param ts		transaction set
 * @param h		header
 * @param key		package retrieval key (e.g. file name)
 * @param upgrade	is package being upgraded?
 * @param relocs	package file relocations
 * @return		0 on success, 1 on I/O error, 2 needs capabilities
 */
int rpmtsAddInstallElement(rpmts ts, Header h,
		const fnpyKey key, int upgrade,
		rpmRelocation * relocs);

/** \ingroup rpmts
 * Add package to be erased to transaction set.
 * @param ts		transaction set
 * @param h		header
 * @param dboffset	rpm database instance
 * @return		0 on success
 */
int rpmtsAddEraseElement(rpmts ts, Header h, int dboffset);

/** \ingroup rpmts
 * Retrieve keys from ordered transaction set.
 * @todo Removed packages have no keys, returned as interleaved NULL pointers.
 * @param ts		transaction set
 * @retval ep		address of returned element array pointer (or NULL)
 * @retval nep		address of no. of returned elements (or NULL)
 * @return		0 always
 */
int rpmtsGetKeys(rpmts ts,
		fnpyKey ** ep,
		int * nep);

/**
 * Return (malloc'd) header name-version-release string.
 * @param h		header
 * @retval np		name tag value
 * @return		name-version-release string
 */
char * hGetNEVR(Header h, const char ** np );

/**
 * Return (malloc'd) header name-version-release.arch string.
 * @param h		header
 * @retval np		name tag value
 * @return		name-version-release string
 */
char * hGetNEVRA(Header h, const char ** np );

/**
 * Return header color.
 * @param h		header
 * @return		header color
 */
uint_32 hGetColor(Header h);

#ifdef __cplusplus
}
#endif


#endif	/* H_RPMTS */
