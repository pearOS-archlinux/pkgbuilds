
#ifndef KRDB_EXPORT_H
#define KRDB_EXPORT_H

#ifdef KRDB_STATIC_DEFINE
#  define KRDB_EXPORT
#  define KRDB_NO_EXPORT
#else
#  ifndef KRDB_EXPORT
#    ifdef krdb_EXPORTS
        /* We are building this library */
#      define KRDB_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define KRDB_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef KRDB_NO_EXPORT
#    define KRDB_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef KRDB_DEPRECATED
#  define KRDB_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef KRDB_DEPRECATED_EXPORT
#  define KRDB_DEPRECATED_EXPORT KRDB_EXPORT KRDB_DEPRECATED
#endif

#ifndef KRDB_DEPRECATED_NO_EXPORT
#  define KRDB_DEPRECATED_NO_EXPORT KRDB_NO_EXPORT KRDB_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef KRDB_NO_DEPRECATED
#    define KRDB_NO_DEPRECATED
#  endif
#endif

#endif /* KRDB_EXPORT_H */
