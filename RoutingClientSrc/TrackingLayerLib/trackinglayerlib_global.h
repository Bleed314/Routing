#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(TRACKINGLAYERLIB_LIB)
#  define TRACKINGLAYERLIB_EXPORT Q_DECL_EXPORT
# else
#  define TRACKINGLAYERLIB_EXPORT Q_DECL_IMPORT
# endif
#else
# define TRACKINGLAYERLIB_EXPORT
#endif
