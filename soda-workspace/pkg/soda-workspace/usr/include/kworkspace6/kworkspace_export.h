
#ifndef KWORKSPACE_EXPORT_H
#define KWORKSPACE_EXPORT_H

#ifdef KWORKSPACE_STATIC_DEFINE
#  define KWORKSPACE_EXPORT
#  define KWORKSPACE_NO_EXPORT
#else
#  ifndef KWORKSPACE_EXPORT
#    ifdef kworkspace_EXPORTS
        /* We are building this library */
#      define KWORKSPACE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define KWORKSPACE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef KWORKSPACE_NO_EXPORT
#    define KWORKSPACE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef KWORKSPACE_DEPRECATED
#  define KWORKSPACE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef KWORKSPACE_DEPRECATED_EXPORT
#  define KWORKSPACE_DEPRECATED_EXPORT KWORKSPACE_EXPORT KWORKSPACE_DEPRECATED
#endif

#ifndef KWORKSPACE_DEPRECATED_NO_EXPORT
#  define KWORKSPACE_DEPRECATED_NO_EXPORT KWORKSPACE_NO_EXPORT KWORKSPACE_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef KWORKSPACE_NO_DEPRECATED
#    define KWORKSPACE_NO_DEPRECATED
#  endif
#endif

#endif /* KWORKSPACE_EXPORT_H */
