
#ifndef KLOOKANDFEEL_EXPORT_H
#define KLOOKANDFEEL_EXPORT_H

#ifdef KLOOKANDFEEL_STATIC_DEFINE
#  define KLOOKANDFEEL_EXPORT
#  define KLOOKANDFEEL_NO_EXPORT
#else
#  ifndef KLOOKANDFEEL_EXPORT
#    ifdef klookandfeel_EXPORTS
        /* We are building this library */
#      define KLOOKANDFEEL_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define KLOOKANDFEEL_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef KLOOKANDFEEL_NO_EXPORT
#    define KLOOKANDFEEL_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef KLOOKANDFEEL_DEPRECATED
#  define KLOOKANDFEEL_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef KLOOKANDFEEL_DEPRECATED_EXPORT
#  define KLOOKANDFEEL_DEPRECATED_EXPORT KLOOKANDFEEL_EXPORT KLOOKANDFEEL_DEPRECATED
#endif

#ifndef KLOOKANDFEEL_DEPRECATED_NO_EXPORT
#  define KLOOKANDFEEL_DEPRECATED_NO_EXPORT KLOOKANDFEEL_NO_EXPORT KLOOKANDFEEL_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef KLOOKANDFEEL_NO_DEPRECATED
#    define KLOOKANDFEEL_NO_DEPRECATED
#  endif
#endif

#endif /* KLOOKANDFEEL_EXPORT_H */
