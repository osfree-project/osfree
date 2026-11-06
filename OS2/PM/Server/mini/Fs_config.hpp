/*!
 * @file Fs_config.hpp
 * @brief Server-side configuration for the mini server.
 *
 * Full-server Fs_config.hpp is desktop-oriented; this mini-server
 * variant keeps only what is actually used by Fs_Server and
 * Fs_ClientList. May grow later (ini reader, etc.).
 */
#ifndef FS_CONFIG_HPP
#define FS_CONFIG_HPP

/* Limits (compile-time) */
#define F_MAX_CLIENTS           64   /*!< Maximum simultaneous connections. */
#define F_MAX_HABS              128  /*!< Maximum simultaneous HABs (per-thread anchors). */
#define F_MONITOR_TICK_MS       1000 /*!< Monitor thread wake-up, ms. */
#define F_DEFAULT_INACTIVE      60   /*!< Default inactivity timeout, sec. */
#define F_DEFAULT_GRACE         30   /*!< Default grace timeout (DETACHED->FREE), sec. */

/* Runtime-tunable defaults (may be overridden via ini later) */
struct Fs_ServerConfig {
    ULONG ulMaxClients;   /*!< Maximum simultaneous connections. */
    ULONG ulMaxHabs;      /*!< Maximum simultaneous HABs (<= F_MAX_HABS). */
    ULONG ulInactive;     /*!< Inactivity timeout, seconds. */
    ULONG ulGrace;        /*!< Grace timeout, seconds. */
    ULONG ulMonitorTick;  /*!< Monitor tick, ms. */
    BOOL  fAcceptLogs;    /*!< Accept F_CMD_DB_PRINT from clients. */

    Fs_ServerConfig()
        : ulMaxClients(F_MAX_CLIENTS),
          ulMaxHabs(F_MAX_HABS),
          ulInactive(F_DEFAULT_INACTIVE),
          ulGrace(F_DEFAULT_GRACE),
          ulMonitorTick(F_MONITOR_TICK_MS),
          fAcceptLogs(TRUE)
    {}
};

#endif /* FS_CONFIG_HPP */
