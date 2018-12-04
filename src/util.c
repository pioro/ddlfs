#define _XOPEN_SOURCE
#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>

#ifdef _MSC_VER
	#include <windows.h>
	#include <io.h>
#endif

#include "logging.h"

#ifdef _MSC_VER
	#define strdup _strdup	
	#pragma warning(disable:4996) // Disable warnings like: "This function or variable may be unsafe. Consider using sscanf_s instead."
#endif


time_t utl_str2time(char *time) {
    struct tm *temptime = malloc(sizeof(struct tm));
    if (temptime == NULL) {
            logmsg(LOG_ERROR, "utl_str2time(): unable to allocate memory for temptime");
            return -1;
    }
    memset(temptime, 0, sizeof(struct tm));
    
	// strptime is not available in Windows
	// char* xx = strptime(time, "%Y-%m-%d %H:%M:%S", temptime);
	// if (xx == NULL || *xx != '\0') {
	//     logmsg(LOG_ERROR, "utl_str2time(): unable to parse date [%s]", time);
	//     free(temptime);
	//     return -1;
	// }
	temptime->tm_isdst = -1;
	memset(temptime, 0, sizeof(struct tm));
	sscanf(time, "%d-%d-%d %d:%d:%d", &temptime->tm_year, &temptime->tm_mon, &temptime->tm_mday, &temptime->tm_hour, &temptime->tm_min, &temptime->tm_sec);
	temptime->tm_mon -= 1;		// months are 0 - 11 and 
	temptime->tm_year -= 1900;	// years start in 1900
	
	time_t retval = mktime(temptime); //timegm(temptime);
    free(temptime);
    return retval;
}

int utl_fs2oratype(char **fstype) {
    char *type = *fstype;

    if (strcmp(type, "PACKAGE_SPEC") == 0) {
        free(type);
        type = strdup("PACKAGE");
    } else if (strcmp(type, "PACKAGE_BODY") == 0) {
        free(type);
        type = strdup("PACKAGE BODY");
    } else if (strcmp(type, "TYPE_BODY") == 0) {
        free(type);
        type = strdup("TYPE BODY");
    } else if (strcmp(type, "JAVA_SOURCE") == 0) {
        free(type);
        type = strdup("JAVA SOURCE");
    } else if (strcmp(type, "JAVA_CLASS") == 0) {
        free(type);
        type = strdup("JAVA CLASS");
    } else if (strcmp(type, "MATERIALIZED_VIEW") == 0) {
        free(type);
        type = strdup("MATERIALIZED VIEW");
    }

    if (type == NULL) {
        logmsg(LOG_ERROR, "str_fs2oratype() - unable to malloc for normalized type.");
        return EXIT_FAILURE;
    }

    *fstype = type;
    return EXIT_SUCCESS;
}

int utl_ora2fstype(char **oratype) {
    char *type = *oratype;
    if (strcmp(type, "PACKAGE") == 0) {
        free(type);
        type = strdup("PACKAGE_SPEC");
    } else if (strcmp(type, "PACKAGE BODY") == 0) {
        free(type);
        type = strdup("PACKAGE_BODY");
    } else if (strcmp(type, "TYPE BODY") == 0) {
        free(type);
        type = strdup("TYPE_BODY");
    } else if (strcmp(type, "JAVA SOURCE") == 0) {
        free(type);
        type = strdup("JAVA_SOURCE");
    } else if (strcmp(type, "JAVA CLASS") == 0) {
        free(type);
        type = strdup("JAVA_CLASS");
    } else if (strcmp(type, "MATERIALIZED VIEW") == 0) {
        free(type);
        type = strdup("MATERIALIZED_VIEW");
    }

    if (type == NULL) {
        logmsg(LOG_ERROR, "str_ora2fstype() - unable to malloc for normalized type.");
        return EXIT_FAILURE;
    }

    *oratype = type;
    return EXIT_SUCCESS;
}

#ifdef _MSC_VER
/**
 * Implementation of strcasecmp() as found in strings.h. It's here becasue
 * this function is not part of the standard and so MSC doesn't have it.
 */
int strcasecmp(const char *s1, const char *s2) {

	size_t l1 = strlen(s1);
	size_t l2 = strlen(s2);

	if (l1 != l2)
		return 1;
	
	for (int i = 0; i < l1; i++) {
		int retval = toupper(s1[i]) - toupper(s2[i]);
		if (retval != 0)
			return retval;
	}
	return 0;
}
/*
BOOL ReadFile(
  HANDLE       hFile,
  LPVOID       lpBuffer,
  DWORD        nNumberOfBytesToRead,
  LPDWORD      lpNumberOfBytesRead,
  LPOVERLAPPED lpOverlapped
);

A 32-bit unsigned integer. The range is 0 through 4294967295 decimal.
This type is declared in IntSafe.h as follows:
typedef unsigned long DWORD;


	LPDWORD: A pointer to a DWORD.
	DWORD:	 A 32-bit unsigned integer.

	typedef struct _OVERLAPPED {
		ULONG_PTR Internal;
		ULONG_PTR InternalHigh;
		union {
			struct {
				DWORD Offset;
				DWORD OffsetHigh;
			} DUMMYSTRUCTNAME;
			PVOID Pointer;
		} DUMMYUNIONNAME;
		HANDLE    hEvent;
	} OVERLAPPED, *LPOVERLAPPED;

*/

// https://docs.microsoft.com/sl-si/windows/desktop/Sync/synchronization-and-overlapped-input-and-output
int pread(int fd, void *buf, size_t count, off_t offset) {
	HANDLE hFile = (HANDLE)_get_osfhandle(fd);
	DWORD bytesRead;
	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(OVERLAPPED));
	overlapped.Offset = (DWORD)(offset);
	// overlapped.OffsetHigh = (DWORD)(offset >> 32); 

	if (!ReadFile(hFile, buf, (DWORD)count, &bytesRead, &overlapped)) {
		// @todo: should probably also set errno
		return -1;
	}
	
	return bytesRead;
}

int pwrite(int fd, const void *buf, size_t count, off_t offset) {
	HANDLE hFile = (HANDLE)_get_osfhandle(fd);
	DWORD bytesWritten;
	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(OVERLAPPED));
	overlapped.Offset = (DWORD) offset;
	/*
	BOOL WriteFile(
  HANDLE       hFile,
  LPCVOID      lpBuffer,
  DWORD        nNumberOfBytesToWrite,
  LPDWORD      lpNumberOfBytesWritten,
  LPOVERLAPPED lpOverlapped
);
	*/
	
	if (!WriteFile(hFile, buf, (DWORD)count, &bytesWritten, &overlapped)) {
		// @todo: should probably also set errno
		return -1;
	}

	return bytesWritten;
}

#endif