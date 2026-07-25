
#ifndef TASKMANAGER_EXPORT_H
#define TASKMANAGER_EXPORT_H

#ifdef TASKMANAGER_STATIC_DEFINE
#  define TASKMANAGER_EXPORT
#  define TASKMANAGER_NO_EXPORT
#else
#  ifndef TASKMANAGER_EXPORT
#    ifdef taskmanager_EXPORTS
        /* We are building this library */
#      define TASKMANAGER_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define TASKMANAGER_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef TASKMANAGER_NO_EXPORT
#    define TASKMANAGER_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef TASKMANAGER_DEPRECATED
#  define TASKMANAGER_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef TASKMANAGER_DEPRECATED_EXPORT
#  define TASKMANAGER_DEPRECATED_EXPORT TASKMANAGER_EXPORT TASKMANAGER_DEPRECATED
#endif

#ifndef TASKMANAGER_DEPRECATED_NO_EXPORT
#  define TASKMANAGER_DEPRECATED_NO_EXPORT TASKMANAGER_NO_EXPORT TASKMANAGER_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef TASKMANAGER_NO_DEPRECATED
#    define TASKMANAGER_NO_DEPRECATED
#  endif
#endif

#endif /* TASKMANAGER_EXPORT_H */
