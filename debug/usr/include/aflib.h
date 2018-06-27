#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <event2/event.h>
#include <af_log.h>

/*
 * aflib API for:
 *   - receiving attribute "set" requests from the service
 *   - sending attribute updates to the service
 */

/* API will not accept attribute values larger than this */
#define MAX_ATTRIBUTE_SIZE                  255

typedef enum {
    AF_SUCCESS = 0,
    // errors that don't apply to edge:
    // AF_ERROR_NO_SUCH_ATTRIBUTE = -1,        // unknown attribute id
    // AF_ERROR_BUSY
    // AF_ERROR_INVALID_COMMAND
    // AF_ERROR_QUEUE_OVERFLOW
    // AF_ERROR_QUEUE_UNDERFLOW
    AF_ERROR_INVALID_PARAM = -6,            // bad input parameter
    AF_ERROR_UNAVAILABLE = -7,              // hubby is not available right now

    // errors returned from reading the binary profile:
    AF_ERROR_FILE_NOT_FOUND = -21,
    AF_ERROR_PROFILE_CORRUPTED = -22,
    AF_ERROR_PROFILE_TOO_BIG = -23,
    AF_ERROR_PROFILE_TOO_NEW = -24,
} af_status_t;

/*
 * a remote client is requesting that an attribute be changed.
 * return `true` to accept the change, or `false` to reject it.
 * (to process changes asynchronously, set `aflib_...`.)
 */
typedef bool (*aflib_set_handler_t)(const uint16_t attr_id, const uint16_t value_len, const uint8_t *value);

/*
 * notification of an attribute's current value, either because it has
 * changed internally, or because you asked for the current value with
 * `aflib_get_attribute`.
 */
typedef void (*aflib_notify_handler_t)(const uint16_t attr_id, const uint16_t value_len, const uint8_t *value);

/*
 * service connection status has changed.
 */
typedef void (*aflib_connect_handler_t)(bool connected);

/*
 * IPC connection to hubby has been broken, typically because hubby has exited
 */
typedef void (*aflib_ipc_disconnected_handler_t)(void);

/*
 * start aflib and register callbacks.
 */
af_status_t aflib_init(struct event_base *ev, aflib_set_handler_t set_handler, aflib_notify_handler_t notify_handler);

/*
 * request the current value of an attribute. the result is sent via the
 * `aflib_notify_handler_t` callback.
 */
af_status_t aflib_get_attribute(const uint16_t attr_id);

/*
 * request an attribute to be set.
 */
af_status_t aflib_set_attribute_bytes(const uint16_t attr_id, const uint16_t value_len, const uint8_t *value);

/*
 * variants of setting an attribute, for convenience.
 */
af_status_t aflib_set_attribute_bool(const uint16_t attr_id, const bool value);
af_status_t aflib_set_attribute_i8(const uint16_t attr_id, const int8_t value);
af_status_t aflib_set_attribute_i16(const uint16_t attr_id, const int16_t value);
af_status_t aflib_set_attribute_i32(const uint16_t attr_id, const int32_t value);
af_status_t aflib_set_attribute_i64(const uint16_t attr_id, const int64_t value);
af_status_t aflib_set_attribute_str(const uint16_t attr_id, const uint16_t value_len, const char *value);

/*
 * if you want to get notified when the hub's connection to the service goes
 * up/down, register this handler.
 */
void aflib_set_connect_handler(aflib_connect_handler_t handler);

/*
 * if you want to get notified when you lose the connection to hubby (typically
 * because hubby exited), register this handler.
 */
void aflib_set_ipc_disconnected_handler(aflib_ipc_disconnected_handler_t handler);

/*
 * if set to true, ignore the return code from an `aflib_set_handler_t`.
 * instead, you must call `aflib_confirm_attr` to confim or reject a client
 * "set" request.
 */
void aflib_handle_set_async(bool async);
void aflib_confirm_attr(uint16_t attr_id, bool accepted);

/*
 * for debugging: set to one of LOG_DEBUG1, ... LOG_DEBUG4, or LOG_DEBUG_OFF (the default)
 */
void aflib_set_debug_level(int level);

/*
 * attribute description from a profile.
 * you can use the `user_data` field to store associated data, if you want.
 */
typedef struct {
    uint16_t attr_id;
    uint16_t type;          // from af_attribute_type
    uint16_t flags;         // from af_attribute_flag
    uint16_t max_length;
    void *user_data;        // for your use (NULL by default)
} af_attribute_t;

/*
 * profile description: the list of attributes and their id, type, and flags.
 */
typedef struct {
    uint16_t attribute_count;
    af_attribute_t *attributes;
} af_profile_t;

enum af_attribute_type {
    ATTR_TYPE_BOOLEAN = 1,
    ATTR_TYPE_SINT8 = 2,
    ATTR_TYPE_SINT16 = 3,
    ATTR_TYPE_SINT32 = 4,
    ATTR_TYPE_SINT64 = 5,
    ATTR_TYPE_FIXED_16_16 = 6,
    ATTR_TYPE_FIXED_32_32 = 7,
    ATTR_TYPE_UTF8S = 20,
    ATTR_TYPE_BYTES = 21,
};

enum af_attribute_flag {
    ATTR_FLAG_READ = 0x0001,
    ATTR_FLAG_READ_NOTIFY = 0x0002,
    ATTR_FLAG_WRITE = 0x0004,
    ATTR_FLAG_WRITE_NOTIFY = 0x0008,
    ATTR_FLAG_HAS_DEFAULT = 0x0010,
    ATTR_FLAG_LATCH = 0x0020,
    ATTR_FLAG_MCU_HIDE = 0x0040,
    ATTR_FLAG_PASS_THROUGH = 0x0080,
    ATTR_FLAG_STORE_IN_FLASH = 0x0100,
};

/*
 * load the hub's profile description.
 * if `filename` is `NULL`, it uses the standard profile file location.
 */
af_status_t aflib_profile_load(const char *filename, af_profile_t *profile);

/*
 * find the attribute description from a profile, given the attribute id.
 * returns `NULL` if no attribute has that id.
 */
af_attribute_t *aflib_profile_find_attribute(af_profile_t *profile, uint16_t attr_id);
